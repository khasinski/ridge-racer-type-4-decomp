/* CC1_FLAGS: -mno-split-addresses */

extern int _padIntPad(), _padIsVsync();

extern int PadIntQueueHandler[];

void _padSetVsyncParam() {
    int *p = PadIntQueueHandler;
    p[0] = (int)_padIntPad;
    p[1] = (int)_padIsVsync;
    p[-1] = 0;
    p[2] = 0;
}
