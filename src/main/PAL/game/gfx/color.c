/* RODATA 0x8001104c 0x18 */
#include "rr/color_conversion.h"
#include "game/gfx/color.h"
#include "rr/channels.h"
#include "game/gfx/color.h"

typedef struct { char pad[0x1C]; short f1c; char pad2[0x3E]; } T;

extern T PlayerControllers[];
extern int SetCdMixMode();
extern int CdAudioSetVolumeRaw();
extern int SeqSetVolume();
extern char SoundState[];

void EnablePadAnalogMode(int i);
void HsvToRgb(const RrHsvColor input,RrRgbColor output);

void EnablePadAnalogMode(int i) {
    if (PlayerControllers[i].f1c != 0) {
        PadSetMainMode(i * 16, 1, 0);
    }
}

void ApplySoundVolumes(void) {
    char *p = SoundState;
    CdAudioSetVolumeRaw(*(unsigned short *)(p + 0x26C4));
    SeqSetVolume(*(short *)(p + 0x2888));
    SetCdMixMode(D_800AD55A);
}

void HsvToRgb(const RrHsvColor input,RrRgbColor output) {
    s32 s=input[1];
    s32 hue=input[0];
    s32 v=input[2];
    s32 h=hue*4096;
    s32 sector,top,low,falling,rising;
    if(s<0)s=0;
    if(s>4096)s=4096;
    if(v<0)v=0;
    if(v>4096)v=4096;
    if(h<0)h+=1474560;
    h%=1474560;
    sector=h/245760;
    h%=245760;
    h/=60;
    top=(v*255)/4096;
    low=(((v*(4096-s))/4096)*255)/4096;
    falling=(((v*(4096-(s*h)/4096))/4096)*255)/4096;
    rising=(((v*(4096-(s*(4096-h))/4096))/4096)*255)/4096;
    switch(sector) {
    case 0:output[0]=top;output[1]=rising;output[2]=low;return;
    case 1:output[0]=falling;output[1]=top;output[2]=low;return;
    case 2:output[0]=low;output[1]=top;output[2]=rising;return;
    case 3:output[0]=low;output[1]=falling;output[2]=top;return;
    case 4:output[0]=rising;output[1]=low;output[2]=top;return;
    case 5:output[0]=top;output[1]=low;output[2]=falling;
    }
}
