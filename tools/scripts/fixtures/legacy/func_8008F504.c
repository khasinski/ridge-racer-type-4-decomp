/* GCC_VERSION: 2.7.2 */
extern int StRingSize, StRingReadIndex, StRingStartIndex, StRingWriteIndex, StDataReadyPending, StCdIntrFlag, StCurFrameNo;
extern short StNextSectorOffset;
extern void init_ring_status();
void StClearRing(void) {
    int a = StRingSize;
    StRingReadIndex = 0;
    StRingStartIndex = 0;
    StRingWriteIndex = 0;
    StDataReadyPending = 0;
    init_ring_status(0, a);
    StCdIntrFlag = 0;
    StNextSectorOffset = 0;
    StCurFrameNo = 0;
}
