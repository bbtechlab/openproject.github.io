/*
 * =====================================================================================
 *
 *       Filename:  libcommon.h
 *
 *    Description:  RFTU project 
 *
 *        Version:  the simplest version
 *        Created:  10/18/2012 04:40:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bamboo Do, dovanquyen.vn@gmail.com
 *        Company: 
 *
 * =====================================================================================
 */
#ifndef __LIBCOMMON_H__
#define __LIBCOMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <sys/time.h>
#include <netdb.h>
#include <fcntl.h>

/*************** MODES OPERATION ******************************************************/
#define DEBUG_MODE   "-d"  /* debug mode */
#define DAEMON_MODE  "-dm" /* deamon mode, used for server only */
/*************** MODES OPERATION ******************************************************/

/****************PRINTING FUNCTION*****************************************************/
/* funtions for printing only in the debug mode */
#define dprintf(x,a) if(debug) {printf(x,a); fflush(stdout);}
/* functions for printing error only (no need debug mode) */
#define eprintf(x)   {printf(x); fflush(stdout);}
/* functions for printing in normal case (print without any condition) */
#define nprintf(x){printf(x); fflush(stdout);}

/* printing functions, used when server runs as a daemon */
#define dm_dprintf(x, a) 	if (is_daemon) syslog(LOG_INFO, x, a); 	else dprintf(x, a);
#define dm_eprintf(x) 		if (is_daemon) syslog(LOG_ERR, x); 	else eprintf(x);
#define dm_nprintf(x) 		if (is_daemon) syslog(LOG_INFO, x);	else nprintf(x);
/****************PRINTING FUNCTION*****************************************************/

/****************RETURN VALUES*********************************************************/
#define CREATE_SOCK_FAILURE   -1
#define CLOSE_SOCK_FAILURE -2
#define RECVFROM_FAILURE   -3
#define TRANS_PACKET_FAILURE  -4
#define REQUEST_TIMEOUT  -5
#define STAT_FILE_FAILURE  -6
#define SERVER_TIMEOUT  -7
/****************RETURN VALUE**********************************************************/

/****************TIMERS DECLARATION****************************************************/
#define CLIENT_TIME_WAIT   5
#define SERVER_TIME_WAIT   5
#define RETRY  5  /* the number of retransmission */
/****************TIMERS DECLARATION****************************************************/

/****************OTHER PARAMETERS******************************************************/
#define MAX_DATA_SIZE   1024
#define MAXBUFFER 65536 /* maximum size of buffer */
#define MINBUFFER 1024
#define INIT_SEQ_NO  0 /* init sequence number the client */

#define SA struct sockaddr /* following shortens all the typecasts of pointer argument */

/* the directory that server process runs in the daemon mode */
#define RUNNING_DIR "/home/quyendv/working/rftu_project/"

/* the prefix for each log message when calling syslog() function */
#define LOG_PREFIX   "rftu_server"
/****************OTHER PARAMETERS******************************************************/

/* packet format */
typedef struct rftu
{
   uint8_t    type;     /* the type of packet */ 
   uint16_t   length;   /* the length of data, in byte */
   uint32_t   seq_no;   /* sequence number */
   uint8_t    padding;  /* Reserved in the future */
} rftu_packet;

/* the type of packet */
enum rftu_type
{
   RFTU_GET = 1,   /* request to get data*/
   RFTU_DAT = 2,   /* data packet */
   RFTU_ACK = 3,   /* acknowledgment of packet */
   RFTU_FIN = 4   /* finish packet */
};

/* ***************FUNCTION************************************************************/
/* display time */
void display_time();
/*
 * handle when SIGALRM occur - Alarm clock
 * */
void handle_alarm();
/* 
 * set the debug mode according to the command line argument. Then shift down
 * the other arguments overwriting the debug argument*/
 void set_debug(int *argc, char *argv[]);

/* handle interrupts(e.g., when CTRL-C is pressed) - close file, socket and
 * exit */
 void sigint();

 /* create udp socket, return the socket handler - sockfd */
 int create_udp_socket();

 /* close udp socket */
 void close_udp_socket(int sockfd);

 /* create a new packet based on packet type, return a packet */
 rftu_packet create_packet(uint8_t rftu_type);

 /* transmit a packet */
 int transmit_packet(rftu_packet pkg, const char *data, uint16_t data_len, int sockfd, const SA *sockaddr);

 /* function can used to set the loss percentage of data  */
 unsigned int set_dropper(unsigned int Loss);

#endif

