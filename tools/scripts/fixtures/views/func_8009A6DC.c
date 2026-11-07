/* CC1_FLAGS: -mno-split-addresses */

/* MASPSX_FLAGS: --assembler-profile=gnu */

extern int (*PadResetChannelFn)();

void _dirFailAuto(a0) unsigned char *a0; {
  int v = a0[0x46];
  unsigned int t;
  *(int *)(a0 + 0x4C) = *(int *)(a0 + 0x4C) + 1;
  if (v != 0) {
    if (v == 1) {
      t = a0[0x4A];
      if (t >= 0xB) {
        a0[0x49] = 2;
        a0[0x46] = 0xFF;
        return;
      }
      a0[0x4A] = t + 1;
      return;
    }
    t = a0[0x4A];
    if (t < 0xB) {
      a0[0x4A] = t + 1;
      return;
    }
    if (a0[0x49] != 0) PadResetChannelFn(a0);
  }
  if (**(unsigned char **)(a0 + 0x3C) != 0xF3) {
    (*(unsigned char **)(a0 + 0x30))[0] = 0xFF;
    (*(unsigned char **)(a0 + 0x30))[1] = 0;
    a0[0xE8] = 0;
    a0[0x35] = 0;
  }
}
