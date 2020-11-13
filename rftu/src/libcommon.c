/*
 * =====================================================================================
 *
 *       Filename:  libcommon.c
 *
 *    Description:  RFTU project
 *
 *        Version:  1.0
 *        Created:  10/19/2012 01:18:54 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bamboo Do, dovanquyen.vn@gmail.com   
 *        Company:  
 *
 * =====================================================================================
 */
#include "libcommon.h"

/* the flag determines the debug mode is enabled. It is used as reference for all other
 * files */
int debug = 0;


extern int sockfd;
extern FILE *fp;

unsigned int DROPPER_loss_percentage = 0;/* the loss parameter in percentage */

void display_time()
{
   time_t current_time;
   
   current_time = time(NULL);
   printf("TIME: %s", asctime(localtime(&current_time)));

}
/* handle ALRM signal */
void handle_alarm()
{
   display_time();
   printf("\t signal - ALRM.\n");
}

/* function is used to handle interrupt signals - SIGINT*/
void sigint()
{
   eprintf("Catch SIGINT signal, so terminates the process!\n");
   if(NULL != fp)
   {
      fclose(fp);
   }

   close_udp_socket(sockfd);

   exit(EXIT_SUCCESS);
}

/* Create an UDP socket and return the sockfd */
int create_udp_socket()
{
   int sockfd;
   if(CREATE_SOCK_FAILURE == (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)))
   {
      eprintf("Failed on creating socket!\n");
      exit(EXIT_FAILURE);
   }
   nprintf("Created socket successfully.\n");
   return sockfd;
}

/* Close an UDP socket, if open */
void close_udp_socket(int sockfd)
{
   if(-1 == sockfd)
      return;
   if(CLOSE_SOCK_FAILURE == close(sockfd))
   {
      eprintf("Failed on closing packet!\n");
      exit(EXIT_FAILURE);
   }
   nprintf("Closed socket already.\n");
}

/* Function used to parse the arguments inputted by users (client or server side), check whether the "-d"
 * argument is used or not. If that, the debug mode is enabled. Then it removes this argument from the 
 * argument lists */
void set_debug(int *argc, char *argv[])
{
	int i;

	/* return if no extra argument is specified. */
	if (*argc <= 1)	
	{
		return;
	}
	
	for (i = 1; i < *argc; i++)
	{
		if (0 == strncmp(argv[i], DEBUG_MODE, 2))
		{
			debug = 1;	/* debug mode is specified */
			break;
		}
	}

	/* Need to remove the debug argument if it is specified */
	if (debug)
	{
		for ( ; i < *argc; i++)
		{
			if (i < (*argc) - 1)
			{
				argv[i] = argv[i+1];
			}

			(*argc)--;
				
		}
	}
}

/* Function used to transmit a packet. 
 *	- pkg:		Packet to be transmited.
 *	- data:		data portion to be transmitted. If data is NULL, it means no data will be transmitted.
 *	- data_len:	size of data portion to be transmitted.
 *	- sockfd:	current socket descriptor.
 *	- sockaddr:	destination where the packet is sent to.
 *
 * Return:
 * 	ret:		the number of bytes successfully transmitted.
 * 	TRANSMIT_PACKET_FAILURE:	failed in transmit packet.	
 *		
 * */
int transmit_packet(rftu_packet pkg, const char *data, uint16_t data_len, int sockfd, const SA *sockaddr)
{
	int ret;
	char buffer[MAXBUFFER];
	int p_type;
   unsigned int pkg_len = 0;
	char *packet_types[] = {"RFTU_GET", "RFTU_DAT", "RFTU_ACK", "RFTU_FIN", "UNKNOWN"};

   pkg_len = sizeof(pkg);
   
   /* fill bytes from packet header into buffer */
   memcpy((void *)buffer, (void *)&pkg, pkg_len);
   if(NULL != data)
   {
      pkg_len += data_len;
      /* fill bytes from data portion into end of buffer */
      memcpy((void *)&buffer[sizeof(pkg)], (void *)data, data_len);
   }  
	
	switch (pkg.type)
   {
		case RFTU_GET:
			p_type = 0;
			break;
		case RFTU_DAT:
			p_type = 1;
			break;
		case RFTU_ACK:
			p_type = 2;
			break;
		case RFTU_FIN:
			p_type = 3;
			break;
		default:
			p_type = 4;	/* unknown packet */
			break;
   }

	dprintf("Try to send '%s' packet,", packet_types[p_type]);
	dprintf(" with seq_no = %d\n", ntohl(pkg.seq_no));

	/* Try to send the packet to the destination*/
	if(-1 == (ret = sendto(sockfd, (void *) buffer, pkg_len, 0, sockaddr, sizeof(struct sockaddr_in))))
	{
		eprintf("\t sendto() error.\n");
      ret = TRANS_PACKET_FAILURE;
	}
	else if(ret != pkg_len)
	{
		eprintf("Can't send enough packet. Sending packet failed.\n");

		ret = TRANS_PACKET_FAILURE;
	}
	else
	{
		dprintf("\t Successfully sending packet with %d bytes.\n", pkg_len);
	}

	/* ********************************** NOTE *****************************************/

	/* We need to check the return value from the function that calls to transmit_packet().
	 * If it fails to transmit packet, then the retransmission is required */
	return ret;		
}

/* Packet create_packet(unsigned char pkg_typ, unsigned char cmd) */
/* Function used to create a packet based on packet type and command. 
 *	- pkg_typ:	packet type.
 *
 * Return:
 * 	Packet created.
 * */
rftu_packet create_packet(uint8_t pkg_type)
{
	rftu_packet pkg;
	memset((void *) &pkg, 0, sizeof(rftu_packet));
	pkg.type = pkg_type;

	return pkg;
}

unsigned int set_dropper(unsigned int Loss)
{
	struct timeval current_time;

	if ((Loss < 0) || (Loss > 100))
	{
		eprintf("Set_dropper: Invalid value of loss percentage\n");
		return -1;
	}

	DROPPER_loss_percentage = Loss;

	dprintf("Set_dropper: loss percentage is set to %d\n", DROPPER_loss_percentage);

	/* retrieves the current system time then uses it to seed the random number generator */
	gettimeofday(&current_time, NULL);
	srand(current_time.tv_usec); 

	return 0;
}

























