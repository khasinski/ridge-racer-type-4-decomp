#ifndef RR_AUDIO_STATE_TYPES_H
#define RR_AUDIO_STATE_TYPES_H
#include "rr/layout.h"
/* Full resident audio owner: func8004EC28 clears0xA8D0 bytes from this base.
 * Four620-byte slots start38. CD status/control occupies265C..26F8.
 * The TOC address is28C8+bank*2048+entry*8; sixteen256-entry banks endA8C8.
 * CdAudioStopRequest is the request26B4 alias, not a separate allocation. */
typedef struct { s32 position,other04; } RrAudioTocEntry8;
typedef struct { s32 keys[9], values[9]; } RrAudioCurve48;
typedef struct { s16 coarse, fine; } RrAudioPitch4;
typedef union { s32 word; struct {s16 level,gain;} halves; } RrAudioLevels4;
typedef struct {
 u32 flags00; s16 bank04; u16 reserved06;
 s32 voice08,count0C,level10,program14,previous18,reserved1C;
 RrAudioLevels4 levels20; s16 pan24,reserved26,group28,reserved2A,spatial2C,reserved2E;
 s32 filter30; u8 reserved34[8];
 RrAudioCurve48 curves3C[2][10];
 u16 programs5DC[10];
 u16 duration5F0,gain5F2,spread5F4,reserved5F6;
 RrAudioPitch4 pitch5F8[10];
} RrAudioSlot620;
RR_ASSERT_SIZE(RrAudioSlot620,0x620);
RR_ASSERT_OFFSET(RrAudioSlot620,programs5DC,0x5DC);
RR_ASSERT_OFFSET(RrAudioSlot620,pitch5F8,0x5F8);
typedef struct {
    u8 pad00[4];s32 muted04,master08;u8 pad0C[12];s16 gain18;u8 pad1A[12];s16 bank26;
    u8 pad28[0x38-0x28];
    RrAudioSlot620 slots38[4];
    u8 pad18B8[0x19D0-0x18B8];
    s32 active19D0,count19D4,timer19D8,timer19DC,mode19E0;
    s16 x19E4,y19E6;u16 items19E8[8];
    u16 left19F8,right19FA;u32 pitch19FC;s32 active1A00;s16 program1A04;u8 pad1A06[2];
    u8 pad1A08[0x265C-0x1A08];
    u8 status265C[8];
    s32 sector2664,sector2668,filter266C,action2670,phase2674,step2678,flag267C,flag2680,reserved2684;
    s32 position2688,previous268C;u32 end2690;
    s32 reserved2694,index2698,state269C,flag26A0,timer26A4,timer26A8,state26AC,reserved26B0,request26B4;
    u8 reserved26B8[0x26CE - 0x26B8];u16 volume26CE;
    s16 tick26D0,half26D2,quarter26D4,reserved26D6;
    s32 tick26D8,previous26DC,reserved26E0,reserved26E4,offset26E8,reserved26EC,base26F0,rate26F4;
    u8 reserved26F8[0x28A4-0x26F8];
    s32 track28A4,count28A8,displayed28AC;
    u8 reserved28B0[24];RrAudioTocEntry8 toc28C8[16][256];
    s32 reservedA8C8,bankA8CC;
} RrAudioState;
RR_ASSERT_SIZE(RrAudioState,0xA8D0);
RR_ASSERT_OFFSET(RrAudioState,active19D0,0x19D0);
RR_ASSERT_OFFSET(RrAudioState,left19F8,0x19F8);
RR_ASSERT_OFFSET(RrAudioState,track28A4,0x28A4);
RR_ASSERT_OFFSET(RrAudioState,status265C,0x265C);
RR_ASSERT_OFFSET(RrAudioState,sector2664,0x2664);
RR_ASSERT_OFFSET(RrAudioState,phase2674,0x2674);
RR_ASSERT_OFFSET(RrAudioState,request26B4,0x26B4);
RR_ASSERT_OFFSET(RrAudioState,tick26D0,0x26D0);
RR_ASSERT_OFFSET(RrAudioState,rate26F4,0x26F4);
RR_ASSERT_OFFSET(RrAudioState,toc28C8,0x28C8);
RR_ASSERT_OFFSET(RrAudioState,bankA8CC,0xA8CC);
#endif
