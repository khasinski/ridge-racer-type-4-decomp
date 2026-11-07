#include "rr/channels.h"
void PadSetMainMode(s32 selector, s32 first, s32 second) {
    _padSetMainMode(PadPortToChannelPtr(selector), first & 0xFF, second & 0xFF);
}
