#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#define	BUFFERSIZE	16384

int main( int C, char **V )
{
	int	rsd;
	struct	sockaddr_in
		to;
	int	r, i;
	unsigned char
		*buffer;
	struct timeval
		T;

	if( C != 4 ) {
		fprintf( stderr, "Usage: %s <host> <port> <message>\n", V[0] );
		return 1;
	}

	if( (buffer = (unsigned char *) malloc(BUFFERSIZE)) == NULL) {
		perror("malloc");
	}

	rsd = socket( PF_INET, SOCK_DGRAM, 0 );
	if( rsd < 0 ) {
		perror( "socket:r" );
		exit( 1 );
	}

	T.tv_sec = 5;
	T.tv_usec = 0;

	if( setsockopt(rsd, SOL_SOCKET, SO_RCVTIMEO, &T, sizeof(T)) < 0 ) {
		perror( "setsockopt:SO_RCVTIMEO" );
		abort();
	}

	memset( &to, 0, sizeof(to) );
	to.sin_family = AF_INET;
	to.sin_port = htons(atoi(V[2]));
	to.sin_addr.s_addr = inet_addr(V[1]);

	for( ; ; ) {
		struct sockaddr_in	
			from;
		int	fromlen;

		if( sendto( rsd, V[3], strlen(V[3]), 0, (struct sockaddr *) &to, sizeof(struct sockaddr_in) ) < 0 ) {
			perror( "sendto" );
			return 1;
		}

		fromlen = sizeof(struct sockaddr_in);
		r = recvfrom( rsd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&from, (socklen_t *) &fromlen );
		if( r < 0 ) {
			if( errno == EAGAIN ) {
				fprintf( stderr, "[timeout]\n" );
				continue;
			} else {
				perror( "recvfrom" );
				return 1;
			}
		}

		fprintf( stderr, "%s:%hu %d bytes <", inet_ntoa(from.sin_addr), ntohs(from.sin_port), r );
		for( i=0; i<r; i++ ) {
			int	c = buffer[i];

			if( !c ) {
				break;
			}

			if( isprint(c) ) {
				putc(c, stderr);
			} else {
				putc('.', stderr);
			}
		}
		fprintf( stderr, ">\n" );

		sleep(1);
	}
}
