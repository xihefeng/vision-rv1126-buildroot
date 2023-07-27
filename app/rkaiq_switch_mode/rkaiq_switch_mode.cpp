#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "rkpacket.hpp"
void handle_pipe(int sig)
{

}

void usage(void) {
	printf("usage:\r\n");
	printf("rkaiq_switch_mode set on\r\n");
	printf("rkaiq_switch_mode set off\r\n");
	printf("rkaiq_switch_mode get\r\n");
	printf("rkaiq_switch_mode get\r\n");
}

int main(int argc, char **argv)
{
	struct sigaction action;
	action.sa_handler = handle_pipe;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, NULL);
	rk_aiq_gray_mode_t mode = RK_AIQ_GRAY_MODE_ON;
	int id = ENUM_ID_IMGPROC_SETGRAYMODE;

	if ( argc < 2 ) {
		usage();
		return -1;
	} else if ( !strcmp(argv[1], "set") ) {
		if ( argc != 3 ) {
			usage();
			return -1;
		} else if ( !strcmp(argv[2], "on") ) {
			mode = RK_AIQ_GRAY_MODE_ON;
		} else if ( !strcmp(argv[2], "off") ) {
			mode = RK_AIQ_GRAY_MODE_OFF;
		} else {
			usage();
			return -1;
		}
		id = ENUM_ID_IMGPROC_SETGRAYMODE;
	} else if ( !strcmp(argv[1], "get") ) {
		if ( argc != 2 ) {
			usage();
			return -1;
		}
		id = ENUM_ID_IMGPROC_GETGRAYMODE;
	} else {
		usage();
		return -1;
	}

	
	class rkpacket pack(id,mode);
rwrite:
	int sockfd = socket(AF_UNIX,SOCK_STREAM,0);
	struct sockaddr_un address;
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path,"/tmp/UNIX.domain");
	int result = connect(sockfd,(struct sockaddr *)&address,sizeof(address));
	if( result == -1 ){
		printf("connect failed: ");
		close(sockfd);
		goto rwrite;
	}
	result = write(sockfd,pack.packetinfo,PKSIZE);
	if(result < 0) {
		close(sockfd);
		goto rwrite;
	}
	read(sockfd,pack.packetinfo,PKSIZE);
	result = pack.printinfo();
	if(result != 0) {
		close(sockfd);
		goto rwrite;
	}

	close(sockfd);
	return result;
}
