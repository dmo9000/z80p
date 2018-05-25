#include <stdint.h>
#include "zextest.h"

#define MAX_SOCKETS	16
#define MAC_DIGITS	6
#define STATE_LINK_DOWN	0
#define STATE_LINK_UP	1

#define TCP_RETURN	0
#define TCP_CONNECT	1
#define TCP_CLOSE	2
#define TCP_SEND	3
#define TCP_RECV	4
#define TCP_ABORT	5

struct sockmap {
                int realfd;
                int mappedfd;
               };

typedef struct sockmap TCP_Socket;


int TCP_init();
int TCP_dispatch(ZEXTEST *context, uint16_t offset, uint16_t limit);
int TCP_Connect(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4, uint16_t portnum);

