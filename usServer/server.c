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

int fd;
char* mfile;
struct sckReq Pack;
struct stat sb;	
char *buffer;

#define SERVER_PORT 3000;


int main(int argc, char *argv[]) {

	int puerto;
	char archivo[256];

	if (argc < 3) {
		printf("Uso: %s [puerto] [archivo formateado]\n", argv[0]);
		exit(-1);
	}

	puerto = atoi(argv[1]);
	strcpy(archivo,argv[2]);

	mmapear(archivo);

	iniciar_server(puerto);
	terminar();

}

void mmapear(char* archivo) {

	if ((fd = open(archivo, O_RDWR, 0)) == -1) {
                perror("open");
		exit(1);
	}
	fstat(fd, &sb);


	mfile = mmap(0, sb.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
	printf("Archivo %s mapeado\n", archivo);
}

void terminar() {
	munmap(mfile, sb.st_size);
	close(fd);
}

void iniciar_server(int puerto) {

	struct sockaddr_in server; 
	char hsk[28];

	int newConn, sd, len;
	if ((sd=socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
		printf("error en socket()\n");
		exit(-1);
	}

	server.sin_family = AF_INET; 
	server.sin_port = htons(puerto);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server.sin_zero),8);

	if (bind (sd,(struct sockaddr*)&server, sizeof(struct sockaddr))==-1) {
		printf ("error en bind() \n");
		exit (-1);
	}
	if (listen (sd, 10) == -1) {
		printf ("error en listen()\n");
		exit (-1);
	}


	if ((newConn = accept(sd, NULL, NULL))==-1) {
		printf("error en accept()\n");
		exit(-1);

	}
	recv(newConn, hsk, 27, 0);
	send(newConn,"HOLA KERNEL", 11, 0);


	while (len = recv(newConn, &Pack, sizeof(struct sckReq), 0) > 0) {
			printf("Recibi un pedido de %d bytes con: %d %ld %ld\n", len, Pack.op, Pack.offset, Pack.nbytes);
			if (Pack.op == 0) {
				//Lectura
				Pack.op = 11;
				buffer = malloc(Pack.nbytes + sizeof(struct sckReq));
				memcpy(buffer, &Pack, sizeof(struct sckReq));
				memcpy(buffer + sizeof(struct sckReq), mfile + Pack.offset, Pack.nbytes);
				send(newConn, buffer, sizeof(struct sckReq) + Pack.nbytes, 0);
			} else { 
				//Escritura
				recv(newConn, buffer, Pack.nbytes, 0 );
				memcpy(mfile + Pack.offset, buffer, Pack.nbytes);
				Pack.op = 10;
				send(newConn, (void*)&Pack, sizeof(struct sckReq), 0);
			}
	}

}

