/*
 * =====================================================================================
 *
 *       Filename:  rftu_server.c
 *
 *    Description:  send any file to client over simple rftu protocol
 *
 *        Version:  simple v1.0
 *        Created:  10/19/2012 03:44:20 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bamboo Do, dovanquyen.vn@gmail.com
 *        Company:  
 *
 * =====================================================================================
 */
#include "libcommon.h"

/*******************************DECLARATION VARIABLE************************************/

extern int debug;	/* Variable check whether the debug mode is set or not */
extern int errno;

int is_daemon;
int sockfd = -1;
FILE *fp = NULL;

static struct sockaddr_in cli_addr;	/* client socket struct */
static unsigned int cli_socklen = sizeof(cli_addr);
static unsigned int current_seq = INIT_SEQ_NO;		/* sequence number of a packet */
static unsigned int data_size;/* size of data part (excluding the packet header part) in a Packet */
static unsigned long file_size;   /* the size of file in bytes */
static unsigned int read_pos_file;  /* the position of the file */
static char tmp_data[MAXBUFFER];	/* temporary variable, used to store bytes read from file */	
static unsigned char flag_sent_fin = 0;/* check when server sent FIN message. If server 
                                        has received EXPECTED ACK and sent FIN message already,
                                        it would have closed connection. Sending file sucessfully. */
static unsigned char  flag_recv_get = 0; /* check when server recived GET message.
                                           If server has recieved GET message already, it would have
                                           reset RETRY.*/
static unsigned char retry = RETRY + 1;

/*******************************DECLARATION VARIABLE************************************/

/*******************************DECLARATION FUNCTION************************************/

static void server_close();
static void server_connect_client(unsigned int port_no);
static int server_receive_packet(void *buff);
static void server_transmit_packet(uint8_t rftu_type, unsigned int seq_no, char *data, int read_size);
static void server_handle_get(const rftu_packet *pkg, const char *filename);
static void server_handle_ack(const rftu_packet *pkg);
static void server_handle_protocol_packet(const char *buff, char *filename);
static void server_daemonize_process();

/*******************************DECLARATION FUNCTION************************************/


/***************************************************************************************
 * Method used to close file, if open, and reset the reading position in file 
 * *************************************************************************************/
static void 
server_close()
{
	if (NULL != fp)
	{
		fclose(fp);
		fp = NULL;
	}

	nprintf("Close the file already. and listening other client.\n");

	errno = 0;	/* Reset the errno */
   alarm(0);

	/* Reset the value of sequence numbers (also the reading position in file to the begining position of file) */
   current_seq = INIT_SEQ_NO;
   read_pos_file = 0;
   flag_sent_fin = 0;
   flag_recv_get = 0;
   retry = RETRY + 1;
}

/* **************************************************************************************
 * Function used to set-up a socket, bind the port, and set reusable option to this socket 
 *	- port_no: the port that server uses to bind to the created socket.
 * ***************************************************************************************/
static void 
server_connect_client(unsigned int port_no)
{
	int ru_addr;
	struct sockaddr_in s_addr;	/* server socket struct */

	sockfd = create_udp_socket();

	/* allow the port to be reusable (for multiple connection) right after server closes connection */
	ru_addr = 1;

	if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &ru_addr, sizeof(ru_addr)))	
	{
		eprintf("Cannot set the reusable socket option!\n");
		close_udp_socket(sockfd);
		exit(EXIT_FAILURE);
	}
   
   /* init socket address */
	memset((void *) &s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;  /* IPv4 */
	s_addr.sin_port = htons(port_no);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (-1 == bind(sockfd, (struct sockaddr *) &s_addr, sizeof (s_addr)))
	{
		eprintf("Cannot bind socket to port. Port maybe already in use.\n");
		exit(EXIT_FAILURE);
	}

	nprintf("Bind socket OK. Listening for connections...\n");
}

/******************************************************************************************************
 * receive packet from client
 * Return:
 *    RECVFROM_FAILURE if error. Close and exit.
 *    ret: the number of received bytes successfully.
 ******************************************************************************************************/
static int
server_receive_packet(void *buff)
{
   int ret;
   rftu_packet pkg;
 
	ret = recvfrom(sockfd, buff, MAXBUFFER, 0, (SA *) &cli_addr, &cli_socklen);

   if(errno != EINTR)
   {
      if(-1 == ret)
      {
         ret = RECVFROM_FAILURE;
         eprintf("\t Recvfrom error.\n");
         server_close();
         exit(EXIT_FAILURE);
      }
      else
      {
         /* data size of packet */
         data_size = ret - sizeof(pkg);
      }
   }
   return ret;
}

/* ****************************************************************************************************
 * Handle transmission data packet which read from the file
 * ****************************************************************************************************/
static void
server_transmit_packet(uint8_t rftu_type, unsigned int seq_no, char *data, int read_size)
{
   char *buffer;  /* Is used to store DATA packet to send to client */
   rftu_packet new_pkg;

   buffer = (char *)malloc(read_size);
   if(NULL != data)
   {
      memcpy((void *)buffer, (void *)data, read_size);
   }
   new_pkg = create_packet(rftu_type);
   new_pkg.seq_no = htonl(seq_no);
   new_pkg.length = htons(read_size);
   
   if(TRANS_PACKET_FAILURE == transmit_packet(new_pkg, buffer, read_size, sockfd, (SA *)&cli_addr))
   {
      return;
   }
   else
   {
      free(buffer);
   }
}
/* ****************************************************************************************************
 * Method is used to handle the GET packet 
 * 	- pkg: 		the packet to be handled.
 * 	- file_name: 	the requested file name client wants to download.
 * 	- If reading file successfully, it will send the first data to client
 * 	Otherwise, reset resource.
 **************************************************************************************************** */
static void 
server_handle_get(const rftu_packet *pkg, const char *filename)
{
	struct stat f_info;		/* object used to calculate the size of file */
	char f_size[MAXBUFFER];		/* size of requested file in bytes */
   char tmp_data[MAXBUFFER];  /* temporary variable, used to store bytes read from file */
   int read_size; /* the actual number of bytes is read from file */

	current_seq = ntohl(pkg->seq_no);	/* obtain the sequence number from ACK packet */

	dprintf("\t Received a GET packet, seq = %d.\n", current_seq);
 

	/* open filename to read data */
	if ((NULL == (fp = fopen(filename, "r"))) ||(-1 == stat(filename, &f_info))) 
	{
		dprintf("\t Openning file %s error!.\n", filename);
		server_close();	/* Reset the resource when error happens */			
	} 
	else
	{
      alarm(0);/* reset timer */
      if(current_seq != INIT_SEQ_NO)
      {
         printf("\t Unexpected sequence.!");
         return;
      }
      else
      {
         sprintf(f_size, "%lu", (long) f_info.st_size);		/* Get the size in bytes of file */
         file_size = atoi(f_size);
         dprintf("\t Openning file %s successfully with", filename);
         dprintf(" file_size = %lu\n", file_size);
         dprintf("\t ...Start sending file %s.\n", filename);

         /* Read bytes from file. In here 'read_size' is actual no of bytes
          * successfully read from file */
         read_size = fread((void *)tmp_data, 1, MAX_DATA_SIZE, fp);
         
         /* send the first data to client */
    
         server_transmit_packet(RFTU_DAT, current_seq, tmp_data, read_size);
         /*backup data for retransmission*/
         read_pos_file += read_size;
         
         /* update retry */
         if(flag_recv_get == 0)
         {
            retry = RETRY;
            flag_recv_get = 1;
         }
      }
   }
}

/*************************************************************************************************
 * Method is used to handle the ACK message from client. 
 * 	- pkg: 		the packet to be handled
 * 	- If received ACK with expected sequence, it would send next DATA.
 * 	Otherwise it would send DATA with previous sequence number
 *************************************************************************************************/
static void
server_handle_ack(const rftu_packet *pkg)
{
   int read_size;			/* the actual number of bytes is read from file */
   unsigned int seq_no;
	seq_no = ntohl(pkg->seq_no);	/* obtain the sequence number from ACK packet */
	/* 
    * check expected sequence which differ to current_seq 
    * send DATA with previous sequence = current sequence -1 if received unexpected sequence.
    * */
	if (seq_no != current_seq)
	{
		dprintf("\t Received an ACK packet with unexpected sequence no: %d.\n", seq_no);
      if(!feof(fp))
      {
         printf("RETRY = %d", retry);
		   dprintf("\t ...re-Sending DATA to client with previous_seq = %d.\n", current_seq);      
         server_transmit_packet(RFTU_DAT, current_seq, tmp_data, read_size);
      }
      else
      {
         printf("RETRY = %d", retry);
         dprintf("\t ...re-Sending FIN to client with previous_seq = %d.\n", current_seq);      
         server_transmit_packet(RFTU_FIN, current_seq, NULL, 0);

      }
	}

   else
   {
      dprintf("\t Received an ACK packet with expected sequence no: %d.\n", seq_no);

   	/* Check whether the file is finish. If that,
	    * server will send RFTU_FIN message */
      if (feof(fp)&&(0 == flag_sent_fin))
      {
         current_seq += 1;
         dprintf("\t ...Sending FIN to client with seq = %d.\n", current_seq);      
		   /* Send RFTU_FIN packet to notify the end of transmision */
		   server_transmit_packet(RFTU_FIN, current_seq, NULL, 0);
         flag_sent_fin = 1;
         return;
      }
      if (feof(fp)&&(1 == flag_sent_fin))
      {
         nprintf("Sucessfully sending file to client.\n");
         server_close();
         return; 
      }

         /* Send next DATA */
	      /* Move to the correct position in the file to read data */
      if(!feof(fp))
      {
          dprintf("\t ...Sending next DATA to client with seq = %d.\n", current_seq+1);      
 
	      if (-1 == fseek(fp, read_pos_file, SEEK_SET))
   	   {
		      dprintf("Error setting reading position %d in file\n", read_pos_file);

   		   /* Need to pause and re-read the file after a period of time??? */

		      /* Do nothing, since servers knows that after a period of time the client will send ACK again.
		      * At this time, server will retry to read file again. */
   	   }

	      /* Read bytes from file. In here 'read_size' is the actual no of bytes successfully read from file */
	      read_size = fread((void *) tmp_data, 1, MAX_DATA_SIZE, fp);

	      if (ferror(fp))
	      {
	   	   eprintf("error reading from file.\n");
   	   }		
	      else
	      {
            alarm(0);/* reset alarm */
		      dprintf("Successfully read %d bytes from file, then transmit it to client.\n", read_size);
         
            current_seq += 1;
            server_transmit_packet(RFTU_DAT, current_seq, tmp_data, read_size);

		      /* Update the read position in file */
            read_pos_file += read_size;
        }
      }
   }
}

/**************************************************************************************************
 * Handle the header of received packet. 
 *    For each packet received by server, this function determines
 *    the packet type and then jumps to the corresponding function to handle it.
 **************************************************************************************************/
static void 
server_handle_protocol_packet(const char *buff, char *filename)
{	
	rftu_packet pkg;		
	char data[MAXBUFFER];	/* data portion, followed by the packet header */

	/* copy received header from buff into pkg */
   memcpy((void *) &pkg, (void *) buff, sizeof(pkg));
   /* copy recieved data from buff into array data */
	memmove((void *) &data, (const void *) &buff[sizeof(pkg)], data_size);

	switch (pkg.type)
   {
		case RFTU_GET:
			server_handle_get(&pkg, filename); 	
			break;
		case RFTU_ACK:
			server_handle_ack(&pkg);
			break;
		default:
			eprintf("Received a invalid packet command.\n");
			break;
	}
}

/* ***********************************************************************************************
 * Function used to parse the arguments inputted by users (client or server side), check 
 * whether the "-dm" argument is used or not. If that, the daemon mode is enabled.
 * Then it removes this argument from the  argument lists
 * ***********************************************************************************************/
static void
server_set_daemon_mode(int *argc, char *argv[])
{
   int i;
   /* return if no extra argument is specified. */
   if(*argc <= 1)
   {
      return;
   }
   for(i = 1; i < *argc; i++)
   {
      if(0 == strncmp(argv[i], DAEMON_MODE, 3))
      {
         is_daemon = 1; /* daemon mode is specified */
         break;
      }
   }
   /* Need to remove the daemon argument if it is specified */
   if(is_daemon)
   {
      for(; i < *argc; i++)
      {
         if(i <(*argc) - 1)
         {
            argv[i] = argv[i + 1];
         }
         (*argc)--;
      }
   }
}

/************************************************************************************************
 * Function used to daemonize the server process.
 ***********************************************************************************************/
static void 
server_daemonize_process()
{
	/* Our process ID and session ID */	
	pid_t pid, sid;
	int max_no_of_descriptor = getdtablesize();

	int i = 0;

	/* Fork off the parent process */
	pid = fork();

	if (pid < 0)
	{
		exit(EXIT_FAILURE);	/* fork() error!!! */
	}
	else if (pid > 0)
	{
		exit(EXIT_SUCCESS);	/* fort() successfully, and now parent exits. */
	}

	/* Change the file mode mask to ensure that any files (including logs) created by the deamon can be
	 * written or read from properly (i.e., grant full access permission for these files).
	 * */
	umask(0);

	/* Open the log file here */



	/* Create a new SID for the child process */
	sid = setsid();

	if (sid < 0)
	{
		/* Log the failure here */
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir(RUNNING_DIR)) < 0)
	{
		/* Log the failure here */
		exit(EXIT_FAILURE);
	}

	/* close all open descriptors when che child process start running */
	for (i = 0; i < max_no_of_descriptor; i++)	/* getdtablesize() is one greater than the largest possible value for a file descriptor */
	{	
		close(i);
	}

	/* For safety, stdin, stdout and stderr should be opened and redirect them to a harmless I/O device (in here is /dev/null) */
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	/* openlog("rftu_server", LOG_PID, LOG_DAEMON); */
	openlog(LOG_PREFIX, LOG_PID, LOG_USER);
}

/* *******************************************************************************************************
 * Main function 
 * ********************************************************************************************************/
int 
main(int argc, char *argv[])
{
   char buff[MAXBUFFER];
   int ret;

  /* Check whether the debug mode is eabled or not */ 
   set_debug(&argc, argv);

	/* Check the deamon is enable or not */
   server_set_daemon_mode(&argc, argv);

   if(is_daemon)
   {
      server_daemonize_process();
   }


   if (argc < 3)
	{
		eprintf("Usage: ./rftu_server <port_number> <filename>\n");
		exit(EXIT_FAILURE);
	}
   dprintf("server port: %s\n", argv[1]);
   dprintf("Sending filename = %s\n", argv[2]);

	/* Set function to handler the interrupt signal (i.e., when user terminates process manually) */
	if (SIG_ERR == signal(SIGINT, sigint))
	{
		eprintf("Couldn't set handler for SIGINT signal.\n");	
		exit(EXIT_FAILURE);
	}

	/* Set up the socket, bind socket to port, and listen to the client's connection */
   server_connect_client(atoi(argv[1]));
   
	while (1)
	{
      while(retry > 1)
      {
         if(SIG_ERR == signal(SIGALRM, handle_alarm))
         {
            eprintf("\t Couldn't set SIGALRM trap!\n.");
            continue;
         }

         if(retry < RETRY + 1)
         {
            alarm(SERVER_TIME_WAIT);/* set timer for receive packet*/
            /*  display_time();*/
         }

         ret = server_receive_packet(buff);

		   if (errno == EINTR)
		   {
	         /* Timer is expired before packet can arrive (i.e., signal interrupt). */
		      nprintf("\t Timeout!, but does not receive any packet from client.\n");
            ret = SERVER_TIMEOUT;
            errno = 0; /* reset errno */
            retry --;
         }
         else
         {
            alarm(0);/* turn off ALRM */
         }

         server_handle_protocol_packet(buff, argv[2]);

      }

      if((1 == retry) && (ret == SERVER_TIMEOUT))
      {
         printf("Receiving packet is expired, it over many times.\n");
         server_close();
         alarm(0);/* turn off alarm */
         continue;
      }
   }
   return 0;
}

