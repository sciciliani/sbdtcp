#define KERNEL_SECTOR_SIZE 512

struct sckReq {
        unsigned int op;
        unsigned long offset;
        unsigned long nbytes;
} __attribute__((packed));;

void mmapear(char* archivo) ;
void terminar(void);
void iniciar_server(int);

