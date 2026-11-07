/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/bios_devices.h"

void *firstfile(char *path, void *directory);
s32 _firstfile_hook(s32 *a0,s32 a1,s32 a2);

static __inline__ s32 save_first_file_handler(void) {
    RrBiosDevice *device, *end, *limit;

    device = *(RrBiosDevice **)0x150;
    end = device + (*(u32 *)0x154 / sizeof(RrBiosDevice));
    if (device < end) {
        limit = end;
        do {
            if (device->name && PsyqStrcmp(device->name, CardFirstFileDeviceName) == 0) {
                CardFirstFileHandler = device->handler;
                return 1;
            }
            device++;
        } while (device < limit);
    }
    return 0;
}

void *firstfile(char *path, void *directory) {
    char *output;
    s8 *input;
    RrBiosDevice *device, *end, *limit;

    input = (s8 *)path;
    output = CardFirstFileDeviceName;
    /* The BIOS prefix ends at ':' or any lower signed character. */
    while (*input > ':') {
        *output++ = (u8)*input++;
    }
    *output = 0;

    if (!save_first_file_handler()) {
        return 0;
    }

    /* Re-read the descriptor: comparison callbacks may replace the table. */
    device = *(RrBiosDevice **)0x150;
    end = device + (*(u32 *)0x154 / sizeof(RrBiosDevice));
    if (device < end) {
        limit = end;
        do {
            if (device->name && PsyqStrcmp(device->name, CardFirstFileDeviceName) == 0) {
                device->handler = _firstfile_hook;
                break;
            }
            device++;
        } while (device < limit);
    }
    return firstfile2(path, directory);
}

s32 _firstfile_hook(s32 *a0,s32 a1,s32 a2) {
    /* ASM_BARRIER: Keep the captured handler in its original saved register across name comparisons. */
    register RrBiosDeviceHandler handler __asm__("$19");
    unsigned long cursor,end,limit;
    if(*a0==0) *a0=1;
    cursor=(unsigned long)*(RrBiosDevice **)0x150;
    handler=CardFirstFileHandler;
    end=cursor+(*(u32 *)0x154/sizeof(RrBiosDevice))*sizeof(RrBiosDevice);
    if(cursor<end) {
        /* ASM_BARRIER: Keep the loop bound assignment after the initial table-range gate. */
        __asm__ volatile("" : : : "$17");
        limit=end;
        do {
            RrBiosDevice *device=(RrBiosDevice *)cursor;
            if(device->name) {
                /* ASM_BARRIER: Materialize the comparison name in its call argument each iteration. */
                register char *page __asm__("$5");
                /* ASM_BARRIER: Keep page materialization inside the nonnull-name branch. */
                __asm__ volatile("" : : : "$5");
                page=(char *)0x80110000U;
                /* ASM_BARRIER: Materialize the name page before its call-delay offset. */
                __asm__("" : "+r"(page));
                if(PsyqStrcmp(device->name,page+0x43C0)==0) {
                    device->handler=handler;
                    break;
                }
            }
            cursor+=sizeof(RrBiosDevice);
        } while(cursor<limit);
    }
    return CardFirstFileHandler(a0,a1,a2);
}
