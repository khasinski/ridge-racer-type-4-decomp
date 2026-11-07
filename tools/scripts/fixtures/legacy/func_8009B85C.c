/* GCC_VERSION: 2.7.2 */
#include "rr/interrupts.h"
#include "rr/serial.h"
extern u16 SioShadowControl;
extern u8 SioIrqQueueEntry[];
extern s32 SysDeqIntRP(s32,void *);
s32 _sio_deinit(void) {
    RrInterruptRegisters *interrupts=I_STAT_Sio;
    RrSerialPortRegisters *port;
    volatile u16 *control;
    interrupts->mask &= ~0x100U;
    port=SIO1_REGS;
    port->control |= 0x50;
    control=&SioShadowControl;
    /* The final control write occupies the original call's delay slot. */
    *(u16 *)&port->control=*control;
    SysDeqIntRP(3,SioIrqQueueEntry);
    {
        /* Share the request bases across the store/reload pairs. */
        register volatile RrSerialRequest *send=&SioRequest;
        register RrSerialReceiveRequest *receive;
        /* ASM_BARRIER: Reload request words through the original value register. */
        register s32 active __asm__("$3");
        /* ASM_BARRIER: Keep zero in the shared return register after the sender base expires. */
        register s32 result __asm__("$2");
        /* ASM_BARRIER: Retain the request base across both store/reload pairs. */
        __asm__("" : "=r"(send) : "0"(send));
        send->active=0;
        active=send->active;
        /* ASM_BARRIER: Finish the active reload before materializing the receiver base. */
        __asm__ volatile("" : : "r"(active) : "$4", "memory");
        receive=&SioReceiveRequest;
        /* ASM_BARRIER: Materialize the receive prefix before publishing its reloaded fields. */
        __asm__("" : "=r"(receive) : "0"(receive));
        receive->request.active=active;
        send->second=0;
        active=send->second;
        result=0;
        receive->request.second=active;
        /* ASM_BARRIER: Keep the zero result before the shared return instruction. */
        __asm__ volatile("" : : "r"(result));
        return result;
    }
}
