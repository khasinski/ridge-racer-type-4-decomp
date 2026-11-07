/* GCC_VERSION: 2.7.2 */
extern int D_80110314, D_801102FC, StRingStartIndex, StRingWriteIndex, D_801102EC, D_801102DC, StCurFrameNo;
extern short StNextSectorOffset;
extern void init_ring_status();
void StClearRing(void) {
    int a = D_80110314;
    D_801102FC = 0;
    StRingStartIndex = 0;
    StRingWriteIndex = 0;
    D_801102EC = 0;
    init_ring_status(0, a);
    D_801102DC = 0;
    StNextSectorOffset = 0;
    StCurFrameNo = 0;
}
