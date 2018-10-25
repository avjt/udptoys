#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<ctype.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#define	BUFFERSIZE	16384

int main( int C, char **V )
{
	int	rsd;
	struct	sockaddr_in
		address;
	int	N, r, i;
	unsigned char
		*buffer;

	if( C < 2 ) {
		fprintf( stderr, "Usage: %s port [address]\n", V[0] );
		exit(1);
	}

	if( (buffer = (unsigned char *) malloc(BUFFERSIZE)) == NULL) {
		perror("malloc");
	}

	rsd = socket( PF_INET, SOCK_DGRAM, 0 );
	if( rsd < 0 ) {
		perror( "socket:r" );
		exit( 1 );
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(V[1]));
	address.sin_addr.s_addr = (C>2) ? inet_addr(V[2]) : INADDR_ANY;

	if( bind( rsd, (struct sockaddr *)&address, sizeof(struct sockaddr_in) ) < 0 ) {
		perror( "bind" );
		exit( 1 );
	}

	for( ; ; ) {
		struct sockaddr_in	
			from;
		int	fromlen;

		fromlen = sizeof(struct sockaddr_in);
		r = recvfrom( rsd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&from, &fromlen );
		if( r < 0 ) {
			perror( "recvfrom" );
			return 1;
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
	}
}
