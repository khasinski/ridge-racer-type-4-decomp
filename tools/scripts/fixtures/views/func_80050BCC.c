#include "rr/audio_bank_setup.h"

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x26]; s16 h; char pad2[0x195C - 0x28]; struct { s32 a, b, c; char p[16]; } e[2]; } (*))&SoundState)

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x1964]; struct { s32 a, pad, b, c; char pad2[12]; } e[1]; } (*))&SoundState)

#undef SoundState

void SlotSetupBank(RrAudioBankSetup *arg0)
{
  register u8 *bankOwner asm("$23");
  register u8 *read asm("$18");
  u8 *loopRoot;register s32 zeroArg asm("$6");register u8 *toneArg asm("$7");
  s32 new_var; u8 *slotBase;
  register u8 *write asm("$17");
  register s32 readOffset asm("$19");
  register s32 writeOffset asm("$20");
  register s32 semitone asm("$16");
register s32 channelOffset asm("$4");
  s32 channels[2];
  RrSpuTone32 toneRecord;
  register u8 *tone asm("$22");
  register s32 bankIndex asm("$18");
  register s32 offset asm("$16");
  register s32 i asm("$21");
  s32 mode;register s32 voice asm("$2");
  register s32 *channel asm("$19");
  RrAudioSlot620 *slot;
  register RrAudioState *owner asm("$20");
  u8 *programCursor;
  u8 *pitchCursor;
  s16 handle;register s32 calculation asm("$2");
  register RrAudioState *bankView asm("$3");
  register RrAudioState *bankPointer asm("$16");
  register RrAudioBankSetup *input asm("$22") = arg0;
  new_var = input->slot0C;
  bankIndex = new_var;
  channels[0] = 5;
  channels[1] = 6;slotBase=(u8*)SoundSlots;asm("" : : "r"(slotBase));
  asm("" : "+r"(input));
  calculation=((bankIndex*3)<<4)+bankIndex;asm("" : "+r"(calculation));offset=calculation<<5;
  channelOffset=bankIndex<<2;channel=(s32*)((u8*)channels+channelOffset);
  owner = (RrAudioState *) (slotBase - 0x38);
  handle = *((s16 *) ((((u8 *) owner) + 0x24) + ((*channel) * 2)));
  slot = (RrAudioSlot620 *) (slotBase + offset);
  
  if (handle > (0 - 1))
  {
    SsVabClose(handle);
    *((s16 *) ((((u8 *) owner) + 0x24) + ((*channel) * 2))) = -1;
  }
  PsyqBzero(slot, 0x620);
  mode = 0x5D000;
  if (bankIndex == 0)
  {
    mode = 0x6F000;
  }
  if (input->bank00 != ((const u8 *) (~0UL)))
  {
    handle = (s16) SsVabOpenHeadSticky(input->bank00, -1, mode);
    *((s16 *) ((((u8 *) owner) + 0x24) + ((*channel) * 2))) = handle;
    bankView = (RrAudioState *) (((u8 *) owner) + offset);
    *((RrAudioBankStorage32*)&bankView->slots38[0].bank04) = (s32) handle;
  }
  if (input->data04 != ((const u16 *) (~0UL)))
  {
    bankPointer = (RrAudioState *) (((u8 *) owner) + offset);
    handle = (s16) SsVabTransBody(input->data04, bankPointer->slots38[0].bank04);
    *((s16 *) ((((u8 *) owner) + 0x24) + ((*channel) * 2))) = handle;
    *((RrAudioBankStorage32*)&bankPointer->slots38[0].bank04) = (s32) handle;
    SsVabTransCompleted(1);
  }
  if (input->group08 != (RrAudioProgramData*)~0UL)
  {
    SlotLoadCurves(input->group08, bankIndex);
  }
  slot->flags00 = 1;
  slot->levels20.halves.level = 0x4B;
  slot->levels20.halves.gain = 0x80;
  slot->pan24 = 0x40;
  slot->reserved26 = 0;
  if (!bankIndex)
  {
    asm("");voice=20;
  }
  else
  {
    voice = 0x10;
  }
  slot->voice08 = voice;
  slot->count0C = 4;asm("" : : : "$21", "$22", "$2", "memory");
  i = 0;
  loopRoot = (u8 *) (&SoundState);asm("" : : "r"(loopRoot));
  calculation=((bankIndex*3)<<4)+bankIndex;asm("" : "+r"(calculation));calculation<<=5;
  bankOwner=loopRoot+calculation;
tone=(u8*)&toneRecord;
readOffset=calculation;writeOffset=readOffset;
  do
  {
zeroArg=0;toneArg=tone;
    read = (u8*)((unsigned long)readOffset+(unsigned long)loopRoot);
    write = (u8*)((unsigned long)writeOffset+(unsigned long)loopRoot);
asm("" : : "r"(write));
    SsUtGetVagAtr(*((s16 *) (bankOwner + 0x3C)), *((s16 *) (read + 0x614)), zeroArg, toneArg);
    writeOffset = writeOffset + 4;
    readOffset = readOffset + 2;
    semitone = 0x3C;
    *((s16 *) (write + 0x630)) = (s16) (semitone - ((u8 *) (&toneRecord))[12]);
    *((s16 *) (write + 0x632)) = (s16) ((((u8 *) (&toneRecord))[13] + ((u8 *) (&toneRecord))[12]) << 7);
    SsUtGetVagAtr(*((s16 *) (bankOwner + 0x3C)), *((s16 *) (read + 0x61E)), 0, tone);
    *((s16 *) (write + 0x644)) = (s16) (semitone - ((u8 *) (&toneRecord))[12]);
    i++;
    *((s16 *) (write + 0x646)) = (s16) ((((u8 *) (&toneRecord))[13] + ((u8 *) (&toneRecord))[12]) << 7);
  }
  while (i <= (5 - 1));
}
