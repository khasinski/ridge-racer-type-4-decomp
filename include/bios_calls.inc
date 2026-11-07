# Named PlayStation BIOS entries. Each macro fixes one concrete service.
# ABI: t2 is the dispatch vector; t1 is the service selector.
# Names: https://psx-spx.consoledev.net/kernelbios/#bios-function-summary
# Generated from configs/PAL/bios_services.json; padding stays in the caller unit.

.macro BIOS_SETJMP symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x13
    endlabel \symbol
.endm

.macro BIOS_STRCMP symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x17
    endlabel \symbol
.endm

.macro BIOS_STRNCMP symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x18
    endlabel \symbol
.endm

.macro BIOS_STRCPY symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x19
    endlabel \symbol
.endm

.macro BIOS_STRLEN symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x1B
    endlabel \symbol
.endm

.macro BIOS_BZERO symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x28
    endlabel \symbol
.endm

.macro BIOS_MEMCPY symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x2A
    endlabel \symbol
.endm

.macro BIOS_MEMSET symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x2B
    endlabel \symbol
.endm

.macro BIOS_MEMCHR symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x2E
    endlabel \symbol
.endm

.macro BIOS_RAND symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x2F
    endlabel \symbol
.endm

.macro BIOS_SRAND symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x30
    endlabel \symbol
.endm

.macro BIOS_INIT_HEAP symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x39
    endlabel \symbol
.endm

.macro BIOS_FLUSH_CACHE symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x44
    endlabel \symbol
.endm

.macro BIOS_GPU_CW symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x49
    endlabel \symbol
.endm

.macro BIOS_BU_INIT symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x70
    endlabel \symbol
.endm

.macro BIOS_REMOVE_CDROM symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x72
    endlabel \symbol
.endm

.macro BIOS_SET_MEM symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0x9F
    endlabel \symbol
.endm

.macro BIOS_CARD_INFO symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0xAB
    endlabel \symbol
.endm

.macro BIOS_CARD_LOAD symbol
    glabel \symbol
        addiu $t2, $zero, 0xA0
        jr $t2
         addiu $t1, $zero, 0xAC
    endlabel \symbol
.endm

.macro BIOS_DELIVER_EVENT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x7
    endlabel \symbol
.endm

.macro BIOS_OPEN_EVENT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x8
    endlabel \symbol
.endm

.macro BIOS_CLOSE_EVENT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x9
    endlabel \symbol
.endm

.macro BIOS_WAIT_EVENT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0xA
    endlabel \symbol
.endm

.macro BIOS_TEST_EVENT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0xB
    endlabel \symbol
.endm

.macro BIOS_ENABLE_EVENT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0xC
    endlabel \symbol
.endm

.macro BIOS_DISABLE_EVENT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0xD
    endlabel \symbol
.endm

.macro BIOS_INIT_PAD symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x12
    endlabel \symbol
.endm

.macro BIOS_START_PAD symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x13
    endlabel \symbol
.endm

.macro BIOS_STOP_PAD symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x14
    endlabel \symbol
.endm

.macro BIOS_PAD_INIT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x15
    endlabel \symbol
.endm

.macro BIOS_RETURN_FROM_EXCEPTION symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x17
    endlabel \symbol
.endm

.macro BIOS_RESET_ENTRY_INT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x18
    endlabel \symbol
.endm

.macro BIOS_HOOK_ENTRY_INT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x19
    endlabel \symbol
.endm

.macro BIOS_OPEN symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x32
    endlabel \symbol
.endm

.macro BIOS_LSEEK symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x33
    endlabel \symbol
.endm

.macro BIOS_READ symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x34
    endlabel \symbol
.endm

.macro BIOS_WRITE symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x35
    endlabel \symbol
.endm

.macro BIOS_CLOSE symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x36
    endlabel \symbol
.endm

.macro BIOS_EXIT symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x38
    endlabel \symbol
.endm

.macro BIOS_PUTS symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x3F
    endlabel \symbol
.endm

.macro BIOS_FIRST_FILE symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x42
    endlabel \symbol
.endm

.macro BIOS_NEXT_FILE symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x43
    endlabel \symbol
.endm

.macro BIOS_ADD_DRIVER symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x47
    endlabel \symbol
.endm

.macro BIOS_DELETE_DRIVER symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x48
    endlabel \symbol
.endm

.macro BIOS_INIT_CARD symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x4A
    endlabel \symbol
.endm

.macro BIOS_START_CARD symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x4B
    endlabel \symbol
.endm

.macro BIOS_CARD_WRITE symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x4E
    endlabel \symbol
.endm

.macro BIOS_CARD_READ symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x4F
    endlabel \symbol
.endm

.macro BIOS_NEW_CARD symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x50
    endlabel \symbol
.endm

.macro BIOS_CHANGE_CLEAR_PAD symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x5B
    endlabel \symbol
.endm

.macro BIOS_CARD_STATUS symbol
    glabel \symbol
        addiu $t2, $zero, 0xB0
        jr $t2
         addiu $t1, $zero, 0x5C
    endlabel \symbol
.endm

.macro BIOS_ENQUEUE_INTERRUPT_HANDLER symbol
    glabel \symbol
        addiu $t2, $zero, 0xC0
        jr $t2
         addiu $t1, $zero, 0x2
    endlabel \symbol
.endm

.macro BIOS_DEQUEUE_INTERRUPT_HANDLER symbol
    glabel \symbol
        addiu $t2, $zero, 0xC0
        jr $t2
         addiu $t1, $zero, 0x3
    endlabel \symbol
.endm

.macro BIOS_CHANGE_CLEAR_RCOUNT symbol
    glabel \symbol
        addiu $t2, $zero, 0xC0
        jr $t2
         addiu $t1, $zero, 0xA
    endlabel \symbol
.endm
