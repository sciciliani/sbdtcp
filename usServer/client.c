#include "server.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct sckReq pack;

#define SERVER_PORT 3000


int main(int argc, char *argv[]) {

	struct sockaddr_in to; 

	int newConn, sd;

	char buffer[512];

	if ((sd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
		printf("error en socket()\n");
		exit(-1);
	}

	memset(&to,0, sizeof(to));
        to.sin_family = AF_INET;
        to.sin_addr.s_addr = inet_addr("127.0.0.1");
        to.sin_port = htons(SERVER_PORT);

	connect(sd, (struct sockaddr*)&to, sizeof(struct sockaddr));

	pack.op = 0;
	pack.nbytes = 4096;
	pack.offset = 0;
char* payload = malloc (sizeof(struct sckReq) + pack.nbytes);

int len = send(sd, (void*)&pack, sizeof(struct sckReq), 0);
printf ("Envio una op de lectura de %d bytes (%d %ld %ld)\n",  len, pack.op, pack.offset, pack.nbytes);
recv(sd, payload, sizeof(struct sckReq) + pack.nbytes, pack.nbytes);
if (pack.op == 11) 
	printf("Lectura procesada correctamente!\n");
}


