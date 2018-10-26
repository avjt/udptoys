#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<ctype.h>

#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#define	BUFFERSIZE	16384

int main( int C, char **V )
{
	int	sd, r, i, fromlen;
	struct	sockaddr_in
		address, from;
	unsigned char
		*buffer;
	struct timeval
		then, now;
	long	delta_t;

	if( C != 4 ) {
		fprintf( stderr, "Usage: %s <host> <port> <message>\n", V[0] );
		return 1;
	}

	if( (buffer = (unsigned char *) malloc(BUFFERSIZE)) == NULL) {
		perror("malloc");
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

	if( gettimeofday(&then, 0) < 0 ) {
		perror( "gettimeofday" );
		return 1;
	}

	fromlen = sizeof(struct sockaddr_in);
	r = recvfrom( sd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&from, (socklen_t *) &fromlen );
	if( r < 0 ) {
		perror( "recvfrom" );
		return 1;
	}

	if( gettimeofday(&now, 0) < 0 ) {
		perror( "gettimeofday" );
		return 1;
	}

	now.tv_usec += 1000000 * (now.tv_sec - then.tv_sec);
	delta_t = (long)(now.tv_usec - then.tv_usec);

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
	fprintf( stderr, "> %ld.%06ld seconds\n", delta_t/1000000, delta_t%1000000 );

	return 0;
}
