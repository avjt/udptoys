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

#define	BUFFERSIZE	32768

unsigned char buffer[BUFFERSIZE];

void dump(const unsigned char *B, int L, const struct sockaddr_in *A)
{
	int i;

	fprintf( stderr, "%s:%hu %d bytes <", inet_ntoa(A->sin_addr), ntohs(A->sin_port), L );

	for( i=0; i<L; i++ ) {
		int	c = B[i];

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

void usage( const char *c )
{
	fprintf( stderr, "Usage:\n" );
	fprintf( stderr, "  %s client <to-ip> <to-port> [ <to-ip> <to-port> [...] ]\n", c );
	fprintf( stderr, "  %s server <listen-port> [ <slave-ip> <slave-port> ]\n", c );
	fprintf( stderr, "  %s slave <listen-port>\n", c );
}

int client( int C, char **V )
{
	int	sd;
	struct	sockaddr_in
		to, from;
	int	fromlen, r;
	char	*B = (char *) buffer;
	struct timeval
		T;

	if( (sd = socket(PF_INET, SOCK_DGRAM, 0))  < 0 ) {
		perror( "socket" );
		return 1;
	}

	while( C > 1 ) {
		const char *to_ip_ = V[0], *to_port_ = V[1];

		memset( &to, 0, sizeof(to) );
		to.sin_family = AF_INET;
		to.sin_port = htons(atoi(to_port_));
		to.sin_addr.s_addr = inet_addr(to_ip_);

		sprintf( B, "%s:%hu:%s", inet_ntoa(to.sin_addr), ntohs(to.sin_port), "client" );
		r = strlen(B);

		if( sendto( sd, B, r, 0, (struct sockaddr *) &to, sizeof(to) ) < 0 ) {
			perror( "sendto" );
			return 1;
		}

		V += 2;
		C -= 2;
	}

	T.tv_sec = 5;
	T.tv_usec = 0;

	if( setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &T, sizeof(T)) < 0 ) {
		perror( "setsockopt:SO_RCVTIMEO" );
		abort();
	}

	for( ; ; ) {
		fromlen = sizeof(from);
		r = recvfrom( sd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&from, (socklen_t *) &fromlen );
		if( r < 0 ) {
			if( errno == EAGAIN ) {
				fprintf( stderr, "[timeout]\n" );
				return 1;
			} else {
				perror( "recvfrom" );
				return 1;
			}
		}

		dump(buffer, r, &from);
	}

	return 0;
}

int server( int C, char **V )
{
	int	rsd, tsd;
	struct	sockaddr_in
		to, from, address;
	int	fromlen, r, relay = 0;
	const char
		*listen_port_, *to_ip_, *to_port_;

	listen_port_ = V[0];

	if( C == 3 ) {
		to_ip_ = V[1];
		to_port_ = V[2];

		relay = 1;

		memset( &to, 0, sizeof(to) );
		to.sin_family = AF_INET;
		to.sin_port = htons(atoi(to_port_));
		to.sin_addr.s_addr = inet_addr(to_ip_);

		if( (tsd = socket(PF_INET, SOCK_DGRAM, 0))  < 0 ) {
			perror( "socket:t" );
			return 1;
		}
	}

	if( (rsd = socket(PF_INET, SOCK_DGRAM, 0))  < 0 ) {
		perror( "socket:r" );
		return 1;
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(listen_port_));
	address.sin_addr.s_addr = INADDR_ANY;

	if( bind( rsd, (struct sockaddr *)&address, sizeof(struct sockaddr_in) ) < 0 ) {
		perror( "bind" );
		return 1;
	}

	for( ; ; ) {
		char	*B = (char *) buffer;

		fromlen = sizeof(struct sockaddr_in);
		if( (r = recvfrom( rsd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&from, (socklen_t *) &fromlen )) < 0 ) {
			perror( "recvfrom" );
			return 1;
		}

		dump(buffer, r, &from);

		sprintf( B, "%s:%hu:%s", inet_ntoa(from.sin_addr), ntohs(from.sin_port), "server" );
		r = strlen(B);

		if( sendto( rsd, buffer, r, 0, (struct sockaddr *) &from, sizeof(from) ) < 0 ) {
			perror( "sendto:r" );
			return 1;
		}

		if( relay ) {
			if( sendto( tsd, buffer, r, 0, (struct sockaddr *) &to, sizeof(to) ) < 0 ) {
				perror( "sendto:t" );
				return 1;
			}
		}
	}

	return 0;
}

int slave( const char *listen_port_ )
{
	int	rsd;
	struct	sockaddr_in
		to, from, address;
	int	fromlen, r;

	if( (rsd = socket(PF_INET, SOCK_DGRAM, 0))  < 0 ) {
		perror( "socket" );
		return 1;
	}

	memset( &address, 0, sizeof(address) );
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(listen_port_));
	address.sin_addr.s_addr = INADDR_ANY;

	if( bind( rsd, (struct sockaddr *)&address, sizeof(struct sockaddr_in) ) < 0 ) {
		perror( "bind" );
		return 1;
	}

	for( ; ; ) {
		char *to_ip_, *to_port_, *B = (char *) buffer;

		fromlen = sizeof(struct sockaddr_in);
		if( (r = recvfrom( rsd, buffer, BUFFERSIZE, 0, (struct sockaddr *)&from, (socklen_t *) &fromlen )) < 0 ) {
			perror( "recvfrom" );
			return 1;
		}

		dump(buffer, r, &from);

		to_ip_ = strtok(B, ":");
		to_port_ = strtok(0, ":");
		
		memset( &to, 0, sizeof(to) );
		to.sin_family = AF_INET;
		to.sin_port = htons(atoi(to_port_));
		to.sin_addr.s_addr = inet_addr(to_ip_);

		sprintf( B, "%s:%hu:%s", inet_ntoa(from.sin_addr), ntohs(from.sin_port), "slave" );
		r = strlen(B);

		if( sendto( rsd, buffer, r, 0, (struct sockaddr *) &to, sizeof(to) ) < 0 ) {
			perror( "sendto" );
			return 1;
		}
	}

	return 0;
}

int main( int C, char **V )
{
	if( C < 2 ) {
		usage( V[0] );
		return 1;
	}

	if( !strcmp(V[1], "client") ) {
		if( (C < 4) || (C % 2) ) {
			fprintf(stderr, "Incorrect parameter count for subcommand <%s>\n\n", V[1]);
			usage( V[0] );
			return 1;
		}

		return client( C - 2, V + 2 );
	} else if( !strcmp(V[1], "server") ) {
		if( (C != 3) && (C != 5) ) {
			fprintf(stderr, "Incorrect parameter count for subcommand <%s>\n\n", V[1]);
			usage( V[0] );
			return 1;
		}

		// return server( V[2], V[3], V[4] );
		return server( C - 2, V + 2 );
	} else if( !strcmp(V[1], "slave") ) {
		if( C != 3 ) {
			fprintf(stderr, "Incorrect parameter count for subcommand <%s>\n\n", V[1]);
			usage( V[0] );
			return 1;
		}

		return slave( V[2] );
	} else {
		fprintf(stderr, "Bad subcommand <%s>\n\n", V[1]);
		usage( V[0] );
		return 1;
	}

}

