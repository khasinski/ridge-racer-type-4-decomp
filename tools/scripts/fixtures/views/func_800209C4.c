/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((char *)CurrentDrawBuf)

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define AddDrawModePrim ((s32 (*)())AddDrawModePrim)

#undef AddDrawModePrim

/* legacy typing of shared globals in this part of the unit */
#define AddDrawModePrim ((int (*)())AddDrawModePrim)

#undef AddDrawModePrim

extern int AddPrim();

extern char RaceHudPacketsMode0[];

extern char HudSpriteTemplatesType1[];

extern char RaceHudPacketTemplatesB[];

extern char FrameHudSpritePacketsA[];

extern short PlayerCount;

void InitRaceHudPackets(int a0)
{
    int n, i, j, a, b;
    char *tab;
    char *base;
    char *s1;
    char *s0;
    switch (a0) {
    case 0:
    case 5:
    default:
        n = 4;
        tab = RaceHudPacketsMode0;
        break;
    case 1:
        n = 8;
        tab = HudSpriteTemplatesType1;
        break;
    case 2:
    case 3:
    case 4:
    case 6:
        if (PlayerCount == 2 && a0 == 4) goto l60;
        if (a0 != 6) { n = 12; goto l64; }
    l60:
        n = 6;
    l64:
        tab = RaceHudPacketTemplatesB;
        break;
    }
    for (i = 0; i < n; i++) {
        for (j = 0; j < 2; j++) {
            s1 = FrameHudSpritePacketsA + j * 0x22778;
            s0 = s1 + (i * 0x14 + 0x18);
            InitHudSpritePacket(s0, tab + i * 0x14);
            if (i != 0) AddPrim(s1 + (i * 0x14 + 4), s0);
        }
    }
}
