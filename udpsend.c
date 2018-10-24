#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

int main( int C, char **V )
{
	int	sd;
	struct	sockaddr_in
		address;

	if( C != 4 ) {
		fprintf( stderr, "Usage: %s <host> <port> <message>\n", V[0] );
		return 1;
	}

	sd = socket( PF_INET, SOCK_DGRAM, 0 );
	if( sd < 0 ) {
		perror( "socket" );
		return 1;
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(V[2]));
	address.sin_addr.s_addr = inet_addr(V[1]);

	if( sendto( sd, V[3], strlen(V[3]), 0, (struct sockaddr *) &address, sizeof(struct sockaddr_in) ) < 0 ) {
		perror( "sendto" );
		return 1;
	}

	return 0;
}
