#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "zextest.h"
#include "network.h"

TCP_Socket TCP_Sockets[MAX_SOCKETS];

int TCP_init()
{
    int i = 0;

    printf("TCP_Init()\n");

    for (i = 0; i < MAX_SOCKETS; i++) {
        TCP_Sockets[i].realfd = -1;
        TCP_Sockets[i].mappedfd = -1;
    }

    return 1;

}

int TCP_dispatch(ZEXTEST *context, uint16_t offset, uint16_t limit)
{
    unsigned char *DMA_BUF = context->memory + offset;
    int8_t sockfd = -1;
    uint8_t instruction = TCP_RETURN;
    uint8_t o1 = 0, o2 =0, o3 = 0, o4 = 0;
    uint16_t portnum = 0;
    uint16_t ptr = 0;
    uint8_t len = 0;
    int dataSize = 0;
    int byteCount = 0;
    instruction = (uint8_t)DMA_BUF[0];

    //printf("TCP_Dispatch(0x%04x, %u)\n", offset, limit);

    switch (instruction) {
    case TCP_CONNECT:
        printf("TCP_CONNECT\n");
        o1 = DMA_BUF[1];
        o2 = DMA_BUF[2];
        o3 = DMA_BUF[3];
        o4 = DMA_BUF[4];
        portnum = DMA_BUF[5];
        portnum |= (DMA_BUF[6] << 8);
        sockfd = TCP_Connect(o1, o2, o3, o4, portnum);
        context->memory[offset + 1] = (int8_t)sockfd;
        return sockfd;
        break;
    case TCP_CLOSE:
        printf("TCP_CLOSE\n");
        sockfd = DMA_BUF[1];
        if (sockfd >= 0 && sockfd < MAX_SOCKETS) {
            if (TCP_Sockets[sockfd].realfd != -1) {
				context->memory[offset + 1] = 0;
                /* FIXME: do shutdown() first? */
				close(TCP_Sockets[sockfd].realfd);
				TCP_Sockets[sockfd].realfd = -1;
				TCP_Sockets[sockfd].mappedfd = -1;
				return 0;
            }
        }
        context->memory[offset + 1] = -1;
        return -1;
        break;
    case TCP_SEND:
        //printf("TCP_SEND\n");
        sockfd = DMA_BUF[1];
        ptr = DMA_BUF[2];
        ptr |= (DMA_BUF[3] << 8);
        len = DMA_BUF[4];
        //printf("+++ tcp_send, sockfd = %d, len = %u\n", sockfd, len);
        if (TCP_Sockets[sockfd].realfd == -1) {
            context->memory[offset + 1] = -1;
            context->memory[offset + 2] = EBADF;
            return -1;
        }
        byteCount = write(TCP_Sockets[sockfd].realfd, context->memory + ptr, len);
        if (byteCount == -1) {
            perror("write");
            if (errno == EPIPE) {
                printf("+++ write:transport endpoint is not connected\n");
                context->memory[offset + 1] = -1;
                context->memory[offset + 2] = EPIPE;
                return -1;
                }
            assert(NULL);
            }
        //printf("+++ sent %d bytes\n", byteCount);
        //memory_dump(context->memory + ptr, 0, byteCount);
        context->memory[offset + 1] = byteCount;
        context->memory[offset + 2] = 0;
        return byteCount;
        break;
    case TCP_RECV:
        //printf("TCP_RECV\n");
        sockfd = DMA_BUF[1];
        ptr = DMA_BUF[2];
        ptr |= (DMA_BUF[3] << 8);
        len = DMA_BUF[4];
        //printf("+++ tcp_recv, sockfd = %d, len = %u\n", sockfd, len);
        if (TCP_Sockets[sockfd].realfd == -1) {
            context->memory[offset + 1] = -1;
            context->memory[offset + 2] = EBADF;
            return -1;
        }
        /*
        ioctl(TCP_Sockets[sockfd].realfd, FIONREAD, &dataSize);
        //printf("+++ %d bytes are available on socket\n", dataSize);

        if (dataSize == 0) {
            context->memory[offset + 1] = 0;
            context->memory[offset + 2] = 0;
            return 0;
        }

        if (dataSize < len) {
            len = dataSize;
        }
        */

        byteCount = read(TCP_Sockets[sockfd].realfd, context->memory + ptr, len);
    //    printf("+++ read %d bytes\n", byteCount);
        //memory_dump(context->memory + ptr, 0, byteCount);
       
        if (byteCount == -1) {
   //         perror("read");
   //         printf("errno = %d, %s\n", errno, strerror(errno));
            context->memory[offset + 1] = -1;
            context->memory[offset + 2] = errno;

            if (errno == EPIPE) {
                printf("+++ read:transport endpoint is not connected\n");
                context->memory[offset + 1] = -1;
                context->memory[offset + 2] = EPIPE;
                return -1;
                }

            return -1;
            }
        
        context->memory[offset + 1] = byteCount;
        context->memory[offset + 2] = 0;
        return byteCount;
        break;
    }

    assert(NULL);


}

int TCP_Connect(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint16_t portnum)
{
    int newfd = -1;
    struct sockaddr_in serv_addr;
    unsigned long inaddr;
    int status = -1;
    int synRetries = 1; // Send a total of 3 SYN packets => Timeout ~7s
    int i = 0;
    int flags = 0;
    struct sigaction new_actn, old_actn;

    printf("TCP_Connect(%u.%u.%u.%u, %u)\n", o1, o2, o3, o4, portnum);
    newfd = socket(AF_INET, SOCK_STREAM, 0);
    if (newfd < 0) {
        printf("couldn't open socket\n");
        assert(NULL);
    }
    printf("newfd = %d\n", newfd);
    inaddr = o4 << 24 | o3 << 16 | o2 << 8 | o1;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inaddr;
    serv_addr.sin_port = htons(portnum);

    /* hacky, only works on Linux */

    setsockopt(newfd, IPPROTO_TCP, TCP_SYNCNT, &synRetries, sizeof(synRetries));
    status = connect(newfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr));
    printf("connect status = %d\n", status);

    if (status < 0) {
        printf("CONNECT FAILED\n");
        return -1;
    } else {
        printf("CONNECT OK\n");
    }

    flags = fcntl(newfd, F_GETFL, 0);
    fcntl(newfd, F_SETFL, flags | O_NONBLOCK);

    for (i = 3; i < MAX_SOCKETS; i++) {
        if (TCP_Sockets[i].realfd == -1) {
            TCP_Sockets[i].realfd = newfd;
            TCP_Sockets[i].mappedfd = i;
            printf("return socket %d\n", TCP_Sockets[i].mappedfd);

            /* ignore SIGPIPE, so that read/write can catch transport endpoint errors directly */
            
            new_actn.sa_handler = SIG_IGN;
            sigemptyset (&new_actn.sa_mask);
            new_actn.sa_flags = 0;
            sigaction (SIGPIPE, &new_actn, &old_actn);



            return TCP_Sockets[i].mappedfd;
        }

    }

    /* otherwise, some other error */

    return -1;
}
