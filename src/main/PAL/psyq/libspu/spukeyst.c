/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/spu_init.h"
#include "psyq/libspu/spukeyst.h"

#define BANK ((RrSpuRegisterStorage *)_spu_RXX)
#define STATUS(bank) ((bank)->halves[215])

extern int _spu_keystat;

/* legacy typing of shared globals in this part of the unit */
#define _spu_RXX ((char *)_spu_RXX)

int SpuGetKeyStatus(a0)
int a0;
{
  int idx = -1;
  int i = 0;
  int one = 1;
  int m;
  int r;
  int x;
  int b;
  int f;
  int off;
  unsigned short h;
  loop:
  m = one << i;

  if (a0 & m)
  {
    goto found;
  }
  i++;
  if (i < 0x18)
  {
    goto loop;
  }
  check:
  if (idx != (-1))
  {
    goto have;
  }

  r = -1;
  goto out;
  found:
  idx = i;

  goto check;
  have:
  a0 = idx << 4;

  a0 = a0 + ((int) _spu_RXX);
  f = _spu_keystat;
  f = f & (1 << idx);
 do { a0 = *((unsigned short *) (a0 + 0xC)); x = a0 != 0; if (f == 0) { r = x << 1; goto out; } } while (0);
  r = 1;
  if (a0 != 0)
  {
    goto out;
  }
  r = 3;
  out:
  return r;

}

#undef _spu_RXX

int _spu_init(s32 warm)
{
    register RrSpuRegisterStorage *bank __asm__("$2");
    u32 count;
    s32 i;
    u16 *entry;
    *SPU_DPCR |= 0xB0000;
    _spu_transMode=0;
    _spu_addrMode=0;
    _spu_tsa=0;
    bank=BANK;
    bank->halves[192]=0;
    bank->halves[193]=0;
    bank->halves[213]=0;
    _spu_Fw1ts();
    bank=BANK;
    bank->halves[192]=0;
    bank->halves[193]=0;
    count=0;
    if(STATUS(bank)&0x7FF) {
        /* ASM_BARRIER: Retain the initialized count before its first increment. */
        __asm__("" : "+r"(count));
        count++;
        do {
            if(count>0xF00) {
                DebugPrintStub(SpuTimeoutFmt,SpuWaitResetStr);
                break;
            }
            count++;
        } while(STATUS(BANK)&0x7FF);
    }
    i=0;
    /* ASM_BARRIER: Keep the clear loop indexed by its initialized row count. */
    __asm__("" : "+r"(i));
    entry=_spu_RQ;
    _spu_mem_mode=2;
    _spu_mem_mode_plus=3;
    _spu_mem_mode_unit=8;
    _spu_mem_mode_unitM=7;
    /* ASM_BARRIER: Publish software configuration before capturing the bank. */
    __asm__("" : : : "memory");
    bank=BANK;
    bank->halves[214]=4;
    bank->halves[194]=0;
    bank->halves[195]=0;
    bank->halves[198]=0xFFFF;
    bank->halves[199]=0xFFFF;
    bank->halves[204]=0;
    bank->halves[205]=0;
    do {
        *entry++=0;
        i++;
    } while(i<10);
    if(warm==0) {
        const u16 *source=D_800A4818;
        /* ASM_BARRIER: Prepare the real bounded input owner before MMIO writes. */
        __asm__("" : : "r"(source));
        bank=BANK;
        _spu_tsa=0x200;
        bank->halves[200]=0;
        bank->halves[201]=0;
        bank->halves[202]=0;
        bank->halves[203]=0;
        bank->halves[216]=0;
        bank->halves[217]=0;
        bank->halves[218]=0;
        bank->halves[219]=0;
        /* ASM_BARRIER: Finish MMIO stores before preparing the transfer length. */
        __asm__("" : : : "$5", "memory");
        _spu_FwriteByIO(source,16);
        i=0;
        {
        volatile u16 *voice=BANK->halves;
        do {
            voice[0]=0;voice[1]=0;voice[2]=0x3FFF;
            voice[3]=0x200;voice[4]=0;voice[5]=0;
            /* ASM_BARRIER: Publish each voice before advancing its counter. */
            __asm__("" : "+r"(i) : : "memory");
            i++;voice+=8;
        } while(i<24);
        }
        bank=BANK;
        bank->halves[196]=0xFFFF;
        bank->halves[197]=0xFF;
        _spu_Fw1ts();_spu_Fw1ts();_spu_Fw1ts();_spu_Fw1ts();
        bank=BANK;
        bank->halves[198]=0xFFFF;
        bank->halves[199]=0xFF;
        _spu_Fw1ts();_spu_Fw1ts();_spu_Fw1ts();_spu_Fw1ts();
    }
    { register s32 result __asm__("$2")=0;
    register RrSpuRegisterStorage *end=BANK;
    
    _spu_inTransfer=1;
    end->halves[213]=0xC000;
    _spu_transferCallback=0;
    _spu_IRQCallback=0;
    /* ASM_BARRIER: Finish reset stores before the return phase. */
    __asm__("" : : "r"(result) : "memory");
    return result; }
}
