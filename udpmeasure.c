#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>

#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<sys/select.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#define	BUFFERSIZE	65535

unsigned long long
	bytes = 0,
	packets = 0,
	turn = 0;

const char *pattern = "-\\|/";

int main( int C, char **V )
{
	int	rsd;
	struct	sockaddr_in
		address;
	int	r;
	unsigned char
		*buffer;
	struct timeval
		target;

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

	gettimeofday( &target, 0 );
	target.tv_sec++;

	for( ; ; ) {
		struct sockaddr_in	
			from;
		struct timeval
			now, T;
		fd_set	R;
		int	fromlen;

		FD_ZERO( &R );
		FD_SET( rsd, &R );

		gettimeofday( &now, 0 );

		if( (now.tv_sec > target.tv_sec) || ((now.tv_sec == target.tv_sec) && (now.tv_usec > target.tv_usec)) ) {
			fprintf(stderr, "%c %9llu P/s, %12llu B/s, %13llu b/s\r", pattern[turn], packets, bytes, 8*bytes);

			turn = (turn + 1) % 4;
			bytes = 0;
			packets = 0;
			target = now;
			target.tv_sec++;
		}

		timersub(&target, &now, &T);
		if ( (r = select( rsd + 1, &R, 0, 0, &T )) < 0 ) {
			perror("select");
			return 1;
		}

		if (r) {
			fromlen = sizeof(struct sockaddr_in);
			r = recvfrom( rsd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&from, (socklen_t *) &fromlen );
			if( r < 0 ) {
				perror( "recvfrom" );
				return 1;
			}

			bytes += r;
			packets++;
		}

	}
}
