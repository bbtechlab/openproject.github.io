# =====================================================================================
#
#       Filename:  makefile
#
#    Description:  compile project RFToUDP using gcc
#
#        Version:  none
#        Created:  09/28/2012 09:17:28 AM
#       Revision:  none
#       Compiler:  none
#
#         Author:  Bamboo Do,
#        Company:  dovanquyen.vn@gmail.com
#
# =====================================================================================

#variable

CC = gcc
CFLAGS = -Wall -ansi -pedantic -g
 


UDP_SERVER = rftu_server
UDP_CLIENT = rftu_client
LIB = libcommon
UDP_SERVER_OUTPUT = rftu_server
UDP_CLIENT_OUTPUT = rftu_client


#rule
all: clean ${UDP_SERVER} ${UDP_CLIENT} cleanobj


${UDP_SERVER}: ${LIB}.o 
	$(CC) ${CFLAGS} -o ${UDP_SERVER_OUTPUT} ${UDP_SERVER}.c ${LIB}.o

${UDP_CLIENT}: ${LIB}.o
	$(CC) ${CFLAGS} -o ${UDP_CLIENT_OUTPUT} ${UDP_CLIENT}.c ${LIB}.o

${LIB}.o: ${LIB}.h


clean:
	rm -f *.o ${UDP_SERVER} ${UDP_CLIENT}
cleanobj:
	rm -f *.o


