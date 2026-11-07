/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/bios_devices.h"

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
