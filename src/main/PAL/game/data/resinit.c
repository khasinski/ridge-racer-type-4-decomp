struct S { int p[50]; };
extern struct S CarSlotBuffers[];
int ResetCarSlotBuffers(void) {
    int i;
    for (i = 0; i < 16; i++) {
        CarSlotBuffers[i].p[3] = 0;
        CarSlotBuffers[i].p[0xD] = 0;
        CarSlotBuffers[i].p[0x17] = 0;
        CarSlotBuffers[i].p[0x1A] = 0;
    }
}
