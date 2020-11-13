/*
 * =====================================================================================
 *
 *       Filename:  rftu_client.c
 *
 *    Description:  download any file from server over simple rftu protocol
 *
 *        Version:  sinple v1.0
 *        Created:  10/19/2012 03:43:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bamboo Do, dovanquyen.vn@gmail.com
 *        Company:  
 *
 * =====================================================================================
 */
#include "libcommon.h"

/**************************FLAG DECLARATION*****************************************/
#define FLAG_CTRL_REMOVE   1
/**************************FLAG DECLARATION*****************************************/

/*******************************DECLARATION VARIABLE************************************/
static unsigned int data_size;     /* size of data part (excluding the size of packet header) */

static unsigned int current_seq = INIT_SEQ_NO;/* init sequence number is 0 */
static unsigned int retry = RETRY;
static unsigned char flag_ctrl_remove = 0;/* when turn on this flag, alowing remove file */
static unsigned char count_unexpected_data = 0;/* client count the number of received DATs 
                                                  which wereunexpected packet */
static unsigned char count_unexpected_fin = 0;/* client count the number of received FINs 
                                                 which were unexpected packet */
static struct sockaddr_in sock_addr;/* socket address */
static unsigned int sock_len = sizeof(sock_addr);
FILE *fp = NULL;
int sockfd = -1;
extern int errno;
extern int debug;
/*  extern int is_daemon;*/
extern unsigned int DROPPER_loss_percentage;
/*******************************DECLARATION VARIABLE************************************/

/*******************************DECLARATION FUNCTION************************************/
static void client_close();
static int client_receive_packet(char *buffer);
static int client_transmit_packet(rftu_packet pkg, const char *data, int data_len, int sockfd, const struct sockaddr * sockaddr);
static int client_connect_server(char *hostname, unsigned int port);
static void client_handle_send_get(char *buffer);
static void client_handle_ctrl_fin(rftu_packet pkg);
static void client_handle_data_packet(rftu_packet pkg, char data[MAXBUFFER]);
static void client_handle_protocol_packet(char *buff);
/*******************************DECLARATION FUNCTION************************************/

/*******************************************************************************/
/* close file */
/*******************************************************************************/
void
client_close()
{
   if(NULL != fp)
   {
	   fclose(fp);
      printf("Closed the file already \n");
      if (flag_ctrl_remove == FLAG_CTRL_REMOVE)
      {
         remove("receive_hello.txt");
         printf("File was removed.\n");
      }
   }  
   errno = 0;  /* reset the errno */
   alarm(0);
   current_seq = 0;  /* reset sequence number */
	close_udp_socket(sockfd);
}

/****************************************************************************************
 * transmit packet to server
 * Return
 *    ret:  the number of bytes sucessfully transmitted.
 * **************************************************************************************/
static int
client_transmit_packet(rftu_packet pkg, const char *data, int data_len, int sockfd, const struct sockaddr * sockaddr)
{
	int ret;
	int randomvalue;
	
		/* send packet to server */
		randomvalue = rand() % 100; /* draw a random number */

		if (randomvalue < DROPPER_loss_percentage)
		{
			/* packet is loss - do nothing, but make it look like success */
			nprintf("\t Packet is loss\n");
			ret = sizeof(pkg) + data_len;
		}
		else
		{
			/* otherwise packet is send normally */
			ret = transmit_packet(pkg, data, data_len, sockfd, sockaddr);
		}

	return ret;
}

/****************************************************************************************
* receive packet from server
* Return:
*     RECVFROM_FAILURE if error. Close and exit.
*     ret:  the nunber of received bytes successfully.
*****************************************************************************************/
static int
client_receive_packet(char *buffer)
{
	int ret;
   rftu_packet pkg;
   
   nprintf("...Waiting for data...\n");
	ret = recvfrom(sockfd, (void *)buffer, MAXBUFFER, 0, (SA *)&sock_addr, &sock_len);
  
   if(errno != EINTR)
   {
	   if(-1 == ret)
      {
         ret = RECVFROM_FAILURE;
         eprintf("\t Recvfrom error.\n");
         flag_ctrl_remove = FLAG_CTRL_REMOVE;
         client_close();
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

/**************************************************************************************
* given the hostname of the server in dotted decimal notation and a port #
* attempts to create a socket and connect to the server 
* Return:
*     sockfd: socket file descriptor.
*     Exit if error.
***************************************************************************************/ 
static int 
client_connect_server(char *hostname, unsigned int port)
{
	int sockfd;
	struct hostent *he;
	struct in_addr a;

	sockfd = create_udp_socket();

	/* setup server socket address */
	memset((void *)&sock_addr, 0, sock_len);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	/* get 32-bit IP address from host name - already in network byte order */
	if(NULL!=(he = gethostbyname(hostname)))
	{
		while(*(*he).h_addr_list)
		{
			memcpy((char *) &a, *(he->h_addr_list)++, sizeof(a));
			dprintf("set address: %s\n", inet_ntoa(a));

		}
	}
	else
	{
		eprintf("unable to resolve hostname!\n");
		client_close();
		exit(EXIT_FAILURE);		
	}
	sock_addr.sin_addr.s_addr = a.s_addr;
	return sockfd;
}

 /***********************************************************************************
  * process RFTU_GET 
  * If dont receive any infomation about packet. retransmit RFTU_GET in RETRY 
  * times
  * *********************************************************************************/
static void
client_handle_send_get(char *buffer)
{
	int ret;
   rftu_packet new_pkg;
   unsigned char counter = 0;
  
   /* fill init packet*/
	new_pkg = create_packet(RFTU_GET);
	new_pkg.seq_no = htonl(INIT_SEQ_NO);

   do 
   {
		if(SIG_ERR == signal(SIGALRM, handle_alarm))
		{
			eprintf("\t Couldn't set SIGALRM trap!\n");
			break;
		}
		alarm(SERVER_TIME_WAIT);/* set timer for send GET message*/	
      display_time();
		ret = (client_transmit_packet(new_pkg, NULL, 0, sockfd, (struct sockaddr *)&sock_addr)); 
		
		if(ret == TRANS_PACKET_FAILURE)
		{
			eprintf("\t Client can't transmit GET message to server.\n");
			client_close();
			exit(EXIT_FAILURE);
		}

      /* transmit GET message successfully.
       * receive the first data. Store recived packet into buffer */
label:ret = client_receive_packet(buffer);
		
      /* when signal ALRM occur process return errno == EINTR,
       * mean the time out of GET message is expired. After then RETRY will decrease 1,
       * and reset errno for retransmit GET message again*/
		if(errno == EINTR)
		{
			dprintf("\t Request timed out, remain %d times.\n", retry - 1);
			ret = REQUEST_TIMEOUT;
			errno = 0;/* reset errno */
		}
      else
		{
			alarm(0);/* turn off ALRM */
			dprintf("\t Client received the first packet successful with %d bytes\n", ret);
      	memcpy((void *) &new_pkg, (void *) buffer, sizeof(rftu_packet));
         if(ntohl(new_pkg.seq_no != current_seq))
         {
            nprintf("\t Unexpected sequence.\n");
            counter += 1;
            if(counter == 5)
            {
               counter = 0;
               display_time();
               printf("\t Timeout is expired.\n");
               client_close();
               exit(EXIT_FAILURE);
            }
            goto label;
         }
         break;
		}
		
	}while(--retry);
   
   /* trasmission GET message is expired. Close and exit */
	if(!retry && ret == REQUEST_TIMEOUT)
	{
		printf("Trasmission GET message is expired. It's over %d times.\n", RETRY);
		client_close();
		exit(EXIT_FAILURE);
	}
}

/****************************************************************************************
 * handle control finnish.
 *    - If client received FIN message with expected packet, It would send ACK and
 *    wait some seconds to close connection.Downloading file sucessfully!.
 *    - If client recieved 5 FINs message consecutively with unexpected packet, It
 *    would close connection. Dowloading file failed!.
 ****************************************************************************************/
static void 
client_handle_ctrl_fin(rftu_packet pkg)
{
	uint32_t seq_no = ntohl(pkg.seq_no);
   uint16_t length = ntohs(pkg.length);
   rftu_packet new_pkg;
   int ret;

   if(errno == EINTR)
   {
      errno = 0;
      printf("Dowloading file Sucessfully.\n");
      client_close();
      exit(EXIT_SUCCESS);
   }
   else
   {
      alarm(0);
      printf("\t CLIENT RECEIVED FINISH PACKET with\tdata_size = %d\tseq_no = %d\n",length, seq_no);

      if(seq_no != current_seq)
      {
         printf("\t Unexpected packet.\n");
         count_unexpected_fin += 1;
         printf("\t count_unexpected_fin = %d\n", count_unexpected_fin);
         if(5 == count_unexpected_fin)
         {
            count_unexpected_fin = 0;
            printf("\t Dowloading file failed.\n");
            display_time();
            flag_ctrl_remove = FLAG_CTRL_REMOVE;
            client_close();
            exit(EXIT_FAILURE);
         }
      }

      /* send ACK for FIN message */
      else
      {
         alarm(0);/* turn off alarm */
         printf("\t ...Sending ACK for FIN message with next_seq=%d\n", current_seq);
         new_pkg = create_packet(RFTU_ACK);
         new_pkg.seq_no = htonl(current_seq);

         ret = client_transmit_packet(new_pkg, NULL, 0, sockfd, (struct sockaddr *)&sock_addr);
         if(ret == TRANS_PACKET_FAILURE)
         {
            eprintf("\t Sending ACK for FIN message failed.\n");
            flag_ctrl_remove = FLAG_CTRL_REMOVE;/* Set flag for remove file */
            client_close();
            exit(EXIT_FAILURE);
         }
         else
         {
            nprintf("\t Sending ACK for FIN message successfully\n");
            printf("\t Waiting %d seconds for closing connection.\n", SERVER_TIME_WAIT*RETRY);
            /* wait for server's timeout + RTT to close connection. */              
            if(SIG_ERR == signal(SIGALRM, handle_alarm))
            {
                eprintf("Couldnt set handler for SIGALRM signal.\n");
            }
            else
            {
               alarm(SERVER_TIME_WAIT*RETRY);
               display_time();
            }
         }
      }

   }
}

/*********************************************************************************************
* handle data packet which received from server.
*     - Check sequence number and length. Writing data into file if check success, and
*     continue send ACK with Seq to receive next packet.Otherwise, discard packet
*     and waiting.
*     - If client received 5 ACKs consecutively with unexpected packet, It would close
*     connection. Remove file and downloading file failed!.
**********************************************************************************************/
static void  
client_handle_data_packet(rftu_packet pkg, char data[MAXBUFFER])
{
	uint32_t seq_no = ntohl(pkg.seq_no);
   uint16_t length = ntohs(pkg.length);
	rftu_packet new_pkg;
   int ret;

   if(errno == EINTR)
   {
      errno = 0;
      flag_ctrl_remove = FLAG_CTRL_REMOVE;/* Set flag for remove file */
      client_close();
      exit(EXIT_FAILURE);

   }
   else
   { 
      alarm(0);
      /* Client received DATA from server
       * The first it will check UNEXPECTED PACKET
       * */
      printf("\t Client received DATA packet with\tdata_size = %d\tseq_no =%d\n", length, seq_no);
      if ((data_size != length)||(seq_no != current_seq))
      {			
         eprintf("\t Unexpected packet - waiting for data...\n");
         /* what I have to do ... */
         count_unexpected_data += 1;
         printf("\t count_unexpected_data = %d\n", count_unexpected_data);
         if(5 == count_unexpected_data)
         {
            count_unexpected_data = 0;
            display_time();
            printf("\t Timeout is expired. \n"); 
            flag_ctrl_remove = FLAG_CTRL_REMOVE;
            client_close();
            exit(EXIT_FAILURE);
         }

      }		
      else
      {
         /* Received expected packet */
         printf("\t Expected data packet!\n");
        
         /* write data with data_size into file fp */
         ret = fwrite(data, data_size, 1, fp);

         /* if writing file error. Client will wait for some times and
          * tries to rewrite again before server's timeout 
          * not finish. the current if writing file error it will close and exit */
         if((ret != 1)&& (ret != 0))
         {
            eprintf("\t Writing file error!\n");
            flag_ctrl_remove = FLAG_CTRL_REMOVE;/* set flag for remove file */
            client_close();
            exit(EXIT_FAILURE);
         }
         else
         {
            nprintf("\t Writing file successful!\n");
         }
         /* send ACK for expected data packet with next_seq */
         printf("\t ...Sending ACK to server with seq_no=%d\n", current_seq);
         new_pkg = create_packet(RFTU_ACK);
         new_pkg.seq_no = htonl(current_seq);

         ret = client_transmit_packet(new_pkg, NULL, 0, sockfd, (struct sockaddr *)&sock_addr);
         if(ret == TRANS_PACKET_FAILURE)
         {
            eprintf("\t Sending ACK for expected packet failed.\n");
            flag_ctrl_remove = FLAG_CTRL_REMOVE;/* Set flag for remove file */
            client_close();
            exit(EXIT_FAILURE);
         }
         else
         {
            nprintf("\t Sending ACK for expected packet successfully\n");
             /* next_seq = current_seq + 1 */
            current_seq += 1;
            if(SIG_ERR == signal(SIGALRM, handle_alarm))
            {
                eprintf("Couldnt set handler for SIGALRM signal.\n");
            }
            else
            {
                alarm(SERVER_TIME_WAIT*RETRY);
                display_time();
            }
         }
      }/* end of writing data section*/
   }
}

/*******************************************************************************
* Handle the protocol packet.
*  - Syntax received the header of packet to jum to coressponding function.
*  - Copy data from received packet. 
*******************************************************************************/ 
static void 
client_handle_protocol_packet(char *buff)
{
	rftu_packet pkg;
	char data[MAXBUFFER];   /* data portion, followed by the packet header */

	memcpy((void *) &pkg, (void *) buff, sizeof(pkg));
	memmove((void *) &data, (void *) &buff[sizeof(pkg)], data_size);
	
	switch (pkg.type)
	{
      case RFTU_DAT:
         client_handle_data_packet(pkg, data); 
         break;
		case RFTU_FIN:
			client_handle_ctrl_fin(pkg);
			break;
		default:
			eprintf("Invalid packet command.\n");
			break;
	}
}

/*************************************************************************
*main function
**************************************************************************/
int
main(int argc, char* argv[])
{
   char buffer[MAXBUFFER];
   int ret;
	
	set_debug(&argc, argv);
	if(argc < 4)
	{
		eprintf("Usage: ./client <server_name> <server_port_number> ");
		eprintf("<loss percent>\n");
		exit(0);
	}
	dprintf("server: %s",argv[1]);dprintf(":%s\n",argv[2]);
	
	set_dropper(atoi(argv[3]));
	
	/* connect to server to send */
	sockfd = client_connect_server(argv[1], atoi(argv[2]));

   /* Create file to write */
   if(NULL == (fp = fopen("receive_hello.txt", "w")))
   {
      eprintf("\t Can't create file.\n");
      client_close();
      exit(EXIT_FAILURE);
   }

   /* client handle sending RFTU_GET message */
   client_handle_send_get(buffer);
   /*******************************************************************
    * After Client request successfully. And received the first packet.
    *******************************************************************/
 	while(1)
	{			
		client_handle_protocol_packet(buffer);			
		ret = client_receive_packet(buffer);
	}
	return 0;
}


