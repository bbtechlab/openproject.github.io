	Simple Reliable File Tranfer Over UDP Protocol
         writen by: dovanquyen.vn@gmail.com

1. Motivation

	The following is the UDP (User Datagram Protocol) file transfer protocol implemented for the client and server applications.  
	The goal of this project is to efficiently and reliably transfer files over a UDP connection between one client and one server.

2. Header Format

 0                   1                   2                   3  
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Packet Type  |       Length                  | 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
              Sequence Number                   | Padding       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Data                                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Packet Type : 8 bits

	There are four kind of packets

GET: this kind is used by client to request for downloading data from server.

DATA: is used by server to transmit data to client.

ACK: Acknowledgment field is used by client. Client has to sen ACK to the server when received packet data successfully.

FIN: Finnish feild is used by server for finnishing transmission and closing file. When client received FIN it will release resource,close file and connection.

Sequence Number: 32 bits

	The number of the first data byte of the packet for the current session.

Length: 16 bits 

	The length of data.

Padding: Variable 8 bits 

	rftu_header padding is used to ensure that rftu_header ends and data begins on 32 bit boundary. The padding is composed of zeros.

3. Protocol Operation

Operation is following STOP-and_WAIT mechanism - Data Link Flow Control Protocols
DATA transmission

	To init file transfer, client creates UDP socket and establishes connection to the server. It sends GET message to request downloading any file from server.

        	Client create file to write.
        	Client init sequence number equal zero.
        	Length is 0.
        	Client start set timer for this message. If after timeout it didn't receive any DATA message from server, it will retransmit GET message again.
        	After received DATA message from server it will reset timer, calculate sequence number and send ACK with next sequence number to server.
        	This process continues until client received FIN message.

	After  received GET message from client:

        	Server will open any file to read 
        	Sends data with length is 1024 bytes, SEQ is 0
        	Set timer for this message.
        	After timeout, server didn't receive ACK, it will retransmit this message again with RETRY times.
        	After received ACK it will reset timer and send next data with next sequence number.
        	Processing continues when end of file. It will send FIN message to client.

FINISH transmission

		When client received FIN message from the server, it will send ACK to server, wait for times (server's timeout is added by RTT) to close connection to server (reslease resource, reset timer, close file and close socket).

		When server send FIN message to client, it will wait ACK from client to close connection with current client (relesae resource, reset timer and close file). If it didn't receive ACK after timeout, it will retransmit FIN immediately.
Error Handling

		When client received UNEXPECTED PACKET (packet has sequence number differs to current sequence or data of this packet differs to Length or duplicate packet) it will discard that packets and waiting.

		When server recived UNEXPECTED ACK (ACK has sequence number differs to next sequence of server side). It will retransmit data again with previous sequence number immediately.

.   
