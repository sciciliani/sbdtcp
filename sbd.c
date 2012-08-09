/*
 * A sample, extra-simple block driver. Updated for kernel 2.6.31.
 *
 * (C) 2003 Eklektix, Inc.
 * (C) 2010 Pat Patterson <pat at superpat dot com>
 * Redistributable under the terms of the GNU GPL.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/in.h>

#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/inet.h>
#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <asm/uaccess.h>
#include <linux/file.h>
#include <linux/socket.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");
#define KERNEL_SECTOR_SIZE 512
#define SERVER_PORT 3000
#define ESCRITURA 1
#define SNDBUF 100

static int major_num = 0;
module_param(major_num, int, 0);
static int logical_block_size = KERNEL_SECTOR_SIZE;
module_param(logical_block_size, int, 0);
static int nsectors = 1024; /* How big the drive is */
module_param(nsectors, int, 0);

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */

/*
 * Our request queue.
 */
static struct request_queue *Queue;

/*
 * The internal representation of our device.
 */
static struct sbd_device {
        unsigned long size;
        spinlock_t lock;
        u8 *data;
        struct gendisk *gd;
} Device;
struct sckReq {
	unsigned int op;
	unsigned long offset;
	unsigned long nbytes;
};

char* payload;

static struct socket *clientsocket=NULL;

int send_sync_buf (struct socket *sock, const void *buf, const size_t length, unsigned long flags)
{
    struct msghdr msg;
    struct iovec iov;
    int len, written = 0, left = length;
    mm_segment_t oldmm;

    msg.msg_name     = 0;
    msg.msg_namelen  = 0;
    msg.msg_iov      = &iov;
    msg.msg_iovlen   = 1;
    msg.msg_control  = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags    = flags;

    oldmm = get_fs(); set_fs(KERNEL_DS);

repeat_send:
    msg.msg_iov->iov_len = left;
    msg.msg_iov->iov_base = (char *) buf + written;

    len = sock_sendmsg(sock, &msg, left);
    if ((len == -ERESTARTSYS) || (!(flags & MSG_DONTWAIT) &&
         (len == -EAGAIN)))
        goto repeat_send;
    if (len > 0) {
        written += len;
        left -= len;
        if (left)
            goto repeat_send;
    }
    set_fs(oldmm);
    return written ? written : len;
}

int recv_sync_buf(struct socket *sock, void *buf, const size_t length, unsigned long flags)
{       mm_segment_t oldmm;
        struct msghdr msg;
        struct iovec iov;
        int len;
        int max_size = length;

        msg.msg_name     = 0;
        msg.msg_namelen  = 0;
        msg.msg_iov      = &iov;
        msg.msg_iovlen   = 1;
        msg.msg_control  = NULL;
        msg.msg_controllen = 0;
        msg.msg_flags    = 0;

        msg.msg_iov->iov_base = buf;
        msg.msg_iov->iov_len  = max_size;

        oldmm = get_fs(); set_fs(KERNEL_DS);

read_again:
        len = sock_recvmsg(sock, &msg, max_size, 0); /* MSG_DONTWAIT); */
        if (len == -EAGAIN || len == -ERESTARTSYS) {
            goto read_again;
        }
        set_fs(oldmm);
        return len;
}

void send_string(struct socket *sock, char *str)
{
	send_sync_buf(sock, (void*)str, strlen(str), 0);
}

void recv_string(struct socket *sock, char *str, const size_t length)
{
	recv_sync_buf(sock, (void*)str, length, 0) ;
	str[length] = '\0';
}


/*
 * Handle an I/O request.
 */
static void operar_sector(struct sbd_device *dev, unsigned long offset, unsigned long nbytes, char *buffer, int op) {


	int len;
	struct sckReq pack;
        if ((offset + nbytes) > dev->size) {
                printk (KERN_NOTICE "sbd: Overflow de escritura (%ld %ld)\n", offset, nbytes);
                return;
        }
	pack.op = op;
	pack.offset = offset;
	pack.nbytes = nbytes;
	payload = kmalloc(nbytes + sizeof(struct sckReq), GFP_KERNEL);
	if (payload != NULL) {
		printk(KERN_EMERG "Me preparo para recibir %ld bytes!\n", nbytes + sizeof(struct sckReq));

	
        	if (op == ESCRITURA) {
			memcpy(payload, &pack, sizeof(struct sckReq));
			memcpy(payload + sizeof(struct sckReq), buffer, nbytes);
			len = send_sync_buf(clientsocket, payload, nbytes + sizeof(struct sckReq), 0);
			printk (KERN_NOTICE "Envio una op de escritura de %d bytes (%d %ld %ld)\n",  len ,pack.op, pack.offset, pack.nbytes);
			recv_sync_buf(clientsocket, (void*)&pack, sizeof(struct sckReq), 0);
			if (pack.op == 10) 
				printk(KERN_NOTICE "Escritura procesada correctamente!\n");

		} else {
			len = send_sync_buf(clientsocket, (void*)&pack, sizeof(struct sckReq), 0);
			printk (KERN_EMERG "Envio una op de lectura de %d bytes (%d %ld %ld)\n",  len, pack.op, pack.offset, pack.nbytes);

			len = recv_sync_buf(clientsocket, payload, nbytes + sizeof(struct sckReq) ,0);
//			len = recv_sync_buf(clientsocket, payload + 1000 + sizeof(struct sckReq), 1000 ,0);
//			len = recv_sync_buf(clientsocket, payload + 2000 + sizeof(struct sckReq), 1000 ,0);
//			len = recv_sync_buf(clientsocket, payload + 3000 + sizeof(struct sckReq), 1000 ,0);
			printk(KERN_EMERG "Recibi un tremendo paquete de: %d !\n", len);
			memcpy(&pack, payload, sizeof(struct sckReq));
			printk(KERN_EMERG "Des Serialice el Pack !\n");
			memcpy(buffer, payload + sizeof(struct sckReq), nbytes);
			printk(KERN_EMERG "Devolvi el buffer !\n");
			if (pack.op == 11) 
				printk(KERN_EMERG "Lectura procesada correctamente!\n");

		}
		kfree(payload);
	} else {
		printk(KERN_EMERG "No anduvo el kmalloc!\n");
	}

}


static void sbd_request(struct request_queue *q) {
        struct request *req;
        unsigned long offset;
        unsigned long nbytes;

        req = blk_fetch_request(q);
        while (req != NULL) {
                if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
                        printk (KERN_NOTICE "Skip non-CMD request\n");
                        __blk_end_request_all(req, -EIO);
                        continue;
                }
		offset = blk_rq_pos(req) * logical_block_size;
		nbytes = blk_rq_cur_sectors(req) * logical_block_size;

		operar_sector(&Device, offset, nbytes, req->buffer, rq_data_dir(req));

                if ( ! __blk_end_request_cur(req, 0) ) {
                        req = blk_fetch_request(q);
                }
        }
}

/*
 * The HDIO_GETGEO ioctl is handled in blkdev_ioctl(), which
 * calls this. We need to implement getgeo, since we can't
 * use tools such as fdisk to partition the drive otherwise.
 */
int sbd_getgeo(struct block_device * block_device, struct hd_geometry * geo) {
        long size;

	printk (KERN_NOTICE "Solicitud de geometria\n");

        /* We have no real geometry, of course, so make something up. */
        size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
        geo->cylinders = (size & ~0x3f) >> 6;
        geo->heads = 4;
        geo->sectors = 16;
        geo->start = 0;

	printk (KERN_NOTICE "Respondo: (%d, %d, %d)\n", geo->cylinders, geo->heads, geo->sectors);


        return 0;
}

/*
 * The device operations structure.
 */
static struct block_device_operations sbd_ops = {
                .owner  = THIS_MODULE,
                .getgeo = sbd_getgeo
};


static int __init sbd_init(void) {


	struct sockaddr_in to;
	int retVal = 0;
	char hello[12]; // "HOLA KERNEL"

	printk("Iniciando Socket...\n");

	if (sock_create(PF_INET, SOCK_STREAM, IPPROTO_IP, &clientsocket) < 0) {
		printk( KERN_ERR "server: Error creating clientsocket\n" );
		return -EIO;
	}
	
	memset(&to,0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = in_aton("127.0.0.1");  
	to.sin_port = htons(SERVER_PORT);
	retVal = clientsocket->ops->connect(clientsocket, (struct sockaddr*)&to, sizeof(struct sockaddr_in), 0);

	if (retVal >= 0) {
		printk(KERN_EMERG "Conectado!\n");
		send_string(clientsocket, "HOLA MUNDO DESDE EL KERNEL\n");
		recv_string(clientsocket, hello, 11);
		printk(KERN_EMERG "Me respondieron %s\n", hello);
	} else {
		printk(KERN_EMERG "Error %d\n", -retVal);
	}

	printk(KERN_NOTICE "Iniciando disco\n");

        /*
         * Set up our internal device.
         */
        Device.size = nsectors * logical_block_size;
        spin_lock_init(&Device.lock);
        Device.data = vmalloc(Device.size);
        if (Device.data == NULL)
                return -ENOMEM;
        /*
         * Get a request queue.
         */
        Queue = blk_init_queue(sbd_request, &Device.lock);
        if (Queue == NULL)
                goto out;
        blk_queue_logical_block_size(Queue, logical_block_size);
        /*
         * Get registered.
         */
        major_num = register_blkdev(major_num, "sbd"); //sbd
        if (major_num < 0) {
                printk(KERN_WARNING "sbd: unable to get major number\n"); //sbd
                goto out;
        }
        /*
         * And the gendisk structure.
         */
        Device.gd = alloc_disk(16);
        if (!Device.gd)
                goto out_unregister;
        Device.gd->major = major_num;
        Device.gd->first_minor = 0;
        Device.gd->fops = &sbd_ops;
        Device.gd->private_data = &Device;
        strcpy(Device.gd->disk_name, "sbd0"); //sbd0
        set_capacity(Device.gd, nsectors);
        Device.gd->queue = Queue;
        add_disk(Device.gd);

	printk("Elvis esta vivo!\n");

        return 0;

out_unregister:
        unregister_blkdev(major_num, "sbd"); //sbd
out:
        vfree(Device.data);
        return -ENOMEM;
}

static void __exit sbd_exit(void)
{

	if (clientsocket)
		sock_release(clientsocket);

        del_gendisk(Device.gd);
        put_disk(Device.gd);
        unregister_blkdev(major_num, "sbd");
        blk_cleanup_queue(Queue);
        vfree(Device.data);

	printk("EXIT MODULE\n");

}

module_init(sbd_init);
module_exit(sbd_exit);

