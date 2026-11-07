/* CC1_FLAGS: -mno-split-addresses */
#include "types.h"
#include "m2c_macros.h"
#include "rr/serial.h"
#include "psyq/libsio/siostart.h"

extern int EnterCriticalSection();
extern int ExitCriticalSection();
extern int AddDrv();
extern M2C_UNK SioDeviceBlock;
M2C_UNK FlushCache();
/* extern */
M2C_UNK DelDrv(M2C_UNK *);
/* extern */
extern M2C_UNK StrSioDeviceName;

void DelSIO(void);
void SioEmptyStub(void);
s32 _sio_write_async(s32 first, s32 second);

void AddSIO(void) {
    s32 temp_s0;

    temp_s0 = EnterCriticalSection();
    AddDrv(&SioDeviceBlock);
    if (temp_s0 == 1) {
        ExitCriticalSection();
    }
}

/* legacy typing of shared globals in this part of the unit */
#define ExitCriticalSection ((M2C_UNK (*)())ExitCriticalSection)

void DelSIO(void) {
    s32 temp_s0;

    temp_s0 = EnterCriticalSection();
    DelDrv(&StrSioDeviceName);
    FlushCache();
    if (temp_s0 == 1) {
        ExitCriticalSection();
    }
}

#undef ExitCriticalSection

void SioEmptyStub(void) {
}

s32 _sio_write_async(s32 first, s32 second) {
    volatile RrSerialRequest *request = &SioRequest;
    RrSerialPortRegisters *registers;

    if (request->active == 0) {
        request->second = second;
        request->first = first;
        registers = SIO1_REGS;
        request->active = 1;
        /* Keep the separate read/write operations on the control register. */
        registers->control |= 0x800;
        registers->control |= 0x20;
        return 0;
    }
    return -1;
}
