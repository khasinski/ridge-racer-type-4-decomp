/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "psyq/libsio/siorecv.h"

typedef struct {
    volatile unsigned char data;
    unsigned char pad01[3];
    volatile unsigned short status;
    unsigned char pad06[2];
    volatile unsigned short mode;
    volatile unsigned short control;
    unsigned char pad0C[2];
    volatile unsigned short baud;
} RrSerialPortRegisters;

typedef struct {
    int active;
    int first, second;
} RrSerialRequest;

typedef struct {
    RrSerialRequest request;
    unsigned int receive_status;
} RrSerialReceiveRequest;

extern volatile RrSerialReceiveRequest SioReceiveRequest;
extern RrSerialPortRegisters *SIO1_REGS;
extern RrSerialPortRegisters SioShadowRegs;
extern int (*SioPollHook)(int, int);
extern int DeliverEvent(unsigned int, int);

int _sio_read(int first, int second) {
    int s0 = 0;
    int s1;
    int s5;
    volatile RrSerialReceiveRequest *p1 = &SioReceiveRequest;

    if (p1->request.active != 0) {
        return -1;
    }

    s5 = SioRxFifoSizeTable[(SioShadowRegs.control & 0x300) >> 8];
    p1->request.second = second;
    p1->request.first = first;
    s1 = 0;

    if (p1->request.second != 0) {
        volatile RrSerialReceiveRequest *b = p1;
        do {
            if ((SIO1_REGS->status & 5) != 5) {
                int five;
                five = 5;
                do {
                    if (SioPollHook != 0) {
                        if (SioPollHook(2, s0++) == 0) {
                            DeliverEvent(0xF000000B, 0x100);
                            goto done;
                        }
                    }
                } while ((SIO1_REGS->status & 5) != five);
            }
            if (s1 == 0) {
                b->receive_status = SIO1_REGS->status & 0x80;
            }
            SIO1_REGS->data = *(unsigned char *)SioReceiveRequest.request.first;
            b->request.first += 1;
            s1 += 1;
            b->request.second -= 1;
            if (s1 == s5) {
                if ((SIO1_REGS->status & 0x80) == b->receive_status) {
                    volatile unsigned int *rs;
                    rs = &SioReceiveRequest.receive_status;
                    do {
                        if (SioPollHook != 0) {
                            if (SioPollHook(2, s0++) == 0) {
                                DeliverEvent(0xF000000B, 0x100);
                                return second - SioReceiveRequest.request.second - 1;
                            }
                        }
                    } while ((SIO1_REGS->status & 0x80) == *rs);
                }
                s1 = 0;
            }
        } while (SioReceiveRequest.request.second != 0);
    }
done:
    return second - SioReceiveRequest.request.second;
}
