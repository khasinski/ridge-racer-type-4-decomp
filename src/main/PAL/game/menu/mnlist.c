/* RODATA 0x80010ecc 0x34 */
typedef unsigned char u8;

typedef struct R4
{
  u8 a;
  u8 b;
  u8 c;
  u8 d;
} R4;

typedef signed char s8;

typedef struct {
    u8 a, b, c, d;
} R4_1;

typedef struct {
    char pad0[0x17];
    s8 b17;
    char pad18[4];
    u8 b1C;
    u8 b1D;
    char pad1E[0x2C - 0x1E];
} S2C;

typedef struct 
{
  char p[0x14];
  short s;
} S1;

typedef struct 
{
  char p[0x10];
  int w;
} S3;

typedef struct 
{
  char p[0x2B];
  signed char c;
} S2;

extern int AddTextSprites();
extern int AddDrawModePrim();
extern int AddTilePrim();
extern int AddSpritePrim();
extern int CopyStringClearTail();
extern int FormatDecimalDigits();
extern R4 UiFontGlyphs[];
extern char D_8009FDBF[];
extern char D_8009FDC0[];
extern char StrPacMan[];
extern u8 CarInfoLabelSpacing[];
extern u8 SaveListHeaderGlyphs[];
extern u8 SaveListColumnGlyphPairs[];
extern int SelectionPlayer;
extern signed char *ImageRows;
extern char *CarRecordPtrs[];
extern char *BadgeRows;
extern R4 SelectionRows[];
extern int AddTileSemiTransPrim();
extern u8 SaveListColumnSpacing[];
extern u8 SaveListRowOffsets[];
extern u8 D_8009FDFC[];
extern u8 SaveListFirstColGlyphs[];
extern u8 D_8009FE04[];
extern u8 SaveListCol2GlyphIds[];
extern u8 SaveListTileWidths[];
extern S2C SaveSlotSelections[];
extern short SaveSlotIndex;
extern char FractionText[];
extern unsigned char D_8009FBEC[];
extern char Options[];
extern char PlayerControllers[];
extern int MenuActivePlayerPlusOne;

int DrawCarRecordPanel();
int DrawSaveSlotList();
char *FormatCountOfTotal(int a, int b, int c);
void DrawControlTypeLetterPanel();

int DrawCarRecordPanel(arg0, arg1, arg2)
int arg0;
int arg1;
int arg2;
{
  char sp28[8];
  int sp30;
  int sp34;
  int sp38;
  int sp3C;
  int sp40;
  int sp44;
  int sp48;
  char *sp4C;
  char *a1p;
  int x;
  int y;
  int tx;
  int i;
  int s1;
  int s2;
  int s6;
  int j2;
  int v;
  int t3;
  int t4;
  char *rec;
  u8 *e;
  signed char *f;
  R4 *ent;
  char *c;
  char *q;
  char *q8;
  x = *((short *) ((BadgeRows + (arg2 * 0x1C)) + 4));
  y = *((short *) ((BadgeRows + (arg2 * 0x1C)) + 6));
  sp30 = *((short *) ((BadgeRows + (arg2 * 0x1C)) + 0xC));
  sp3C = (SelectionRows[SelectionPlayer].b * 2) + 8;
  sp38 = SelectionRows[SelectionPlayer].a * 2;
  sp34 = (SelectionRows[SelectionPlayer].c ^ 0x27) == 0;
  if ((SelectionRows[SelectionPlayer].c ^ 0x27) == 0)
  {
    if (SelectionRows[SelectionPlayer].a == 2)
    {
      sp4C = StrPacMan;
    }
    else
    {
      if (SelectionRows[SelectionPlayer].a == 0)
      {
        CopyStringClearTail(D_8009FDBF, "D ");
      }
      else
      {
        CopyStringClearTail(D_8009FDBF, "G ");
      }
      q = D_8009FDC0;
      FormatDecimalDigits(q, SelectionRows[SelectionPlayer].b + 1, 0);
      sp4C = q - 8;
    }
  }
  c = CarRecordPtrs[SelectionPlayer];
  sp48 = 0x11;
  sp44 = ((u8) c[0x1F3]) + 1;
  sp40 = *((short *) (c + 0x1F0));
  if ((*((short *) (c + 0x1D2))) > 0)
  {
    sp48 = 0x12;
  }
  i = 7;
  q8 = &sp28[7];
  do
  {
    *q8 = 0;
    i--;
    q8--;
  }
  while (i >= 0);
  y += 0x1E;
  for (i = 0; i < 0xC; i++)
  {
    x += CarInfoLabelSpacing[i];
    e = SaveListHeaderGlyphs + i;
    arg1 = AddSpritePrim(arg0, arg1, x, y, &UiFontGlyphs[*e], 0x7FF8, 0);
    t3 = x + 2;
    s2 = t3 + UiFontGlyphs[*e].c;
    if (ImageRows[1] != 6)
    {
      switch (i)
      {
        case 1:

        case 7:
          if (sp34 == 0)
        {
          if (i == 1)
          {
            s6 = sp3C;
          }
          else if (i == 7)
          {
            s6 = sp38;
          }
          for (s1 = s6; s1 < (s6 + 2); s1++)
          {
            f = ((signed char *) SaveListColumnGlyphPairs) + s1;
            arg1 = AddSpritePrim(arg0, arg1, s2, y, &UiFontGlyphs[*f], 0x7FF8, 0);
            t4 = s2 + 2;
            s2 = t4 + UiFontGlyphs[*f].c;
          }
        }
        else
        {
          arg1 = AddTilePrim(arg0, arg1, s2, y + 8, 0x38, 2, 0xFF, 0xFF, 0xFF);
        }
          break;

        case 3:
          if (sp34 == 0)
        {
          arg1 = AddTextSprites(arg0, arg1, s2, y, CarRecordPtrs[SelectionPlayer] + 0x1F5, 0x7FF8, 2, 0);
        }
        else
        {
          arg1 = AddTextSprites(arg0, arg1, s2, y, sp4C, 0x7FF8, 2, 0);
        }
          break;

        case 5:

        case 8:
          s6 = 2;
          if (i == 5)
        {
          s1 = sp44;
          s6 = 1;
        }
        else
        {
          s1 = sp40;
        }
          FormatDecimalDigits(sp28, s1, s6);
          arg1 = AddTextSprites(arg0, arg1, s2, y, sp28, 0x7FF8, 0, 0);
          break;

        case 11:
          arg1 = AddSpritePrim(arg0, arg1, s2, y, &UiFontGlyphs[sp48], 0x7FF8, 0);
          break;

      }

    }
    if (i == 5)
    {
      y += 0x1C;
      x = *((short *) ((BadgeRows + (arg2 * 0x1C)) + 4));
    }
  }

  arg1 = AddDrawModePrim(arg0, arg1, 0x38);
  y = y - 8;
  x = *((short *) ((BadgeRows + (arg2 * 0x1C)) + 4));
  arg1 = AddTilePrim(arg0, arg1, x, y, sp30, 2, 0xFF, 0xFF, 0xFF);
  for (i = 0; i < 4; i++)
  {
    v = -0x18;
    if (!(i < 2))
    {
      v = 4;
    }
    sp30 = 0xE4;
    if (i & 1)
    {
      sp30 = 0x78;
    }
    arg1 = AddTilePrim(arg0, arg1, x + sp30, y + v, 1, 0x14, 0xFF, 0xFF, 0xFF);
  }

  return arg1;
}

/* legacy typing of shared globals in this part of the unit */
#define FormatDecimalDigits ((void (*)())FormatDecimalDigits)
#define UiFontGlyphs (*(R4_1 (*)[])&UiFontGlyphs)

int DrawSaveSlotList(arg0, arg1, arg2)
int arg0;
int arg1;
int arg2;
{
    int sp28[3];
    char sp38[8];
    int sp40;
    int x;
    int y;
    int i;
    int j;
    char *p;

    x = *(short *)(BadgeRows + arg2 * 0x1C + 4);
    y = *(short *)(BadgeRows + arg2 * 0x1C + 6);
    sp40 = *(short *)(BadgeRows + arg2 * 0x1C + 0xC);
    y += 0x20;

    for (i = 0; i < 4; i++) {
        x += SaveListColumnSpacing[i];
        arg1 = AddSpritePrim(arg0, arg1, x, y, &UiFontGlyphs[D_8009FDFC[i]], 0x7FF8, 0);
    }

    i = 7;
    p = &sp38[7];
    do {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    y -= 8;
    for (i = 0; i < 3; i++) {
        x = *(short *)(BadgeRows + arg2 * 0x1C + 4) + 8;
        FormatDecimalDigits(sp38, i + 1, 0);
        arg1 = AddTextSprites(arg0, arg1, x, y + SaveListRowOffsets[i + 1], sp38, 0x7FF8, 0, 0);
        if ((sp28[2] = SaveSlotSelections[i].b17) != -1) {
            sp28[0] = SaveListFirstColGlyphs[SaveSlotSelections[i].b1C & 3];
            sp28[1] = D_8009FE04[SaveSlotSelections[i].b1D & 3];
            sp28[2] = SaveListCol2GlyphIds[SaveSlotSelections[i].b17 / 2];
            for (j = 0; j < 3; j++) {
                x += SaveListColumnSpacing[j + 1];
                arg1 = AddSpritePrim(arg0, arg1, x, y + SaveListRowOffsets[i + 1], &UiFontGlyphs[sp28[j]], 0x7FF8, 0);
                if (j == 2 && SaveSlotSelections[i].b17 < 4) {
                    x += 0xE;
                    arg1 = AddSpritePrim(arg0, arg1, x, y + SaveListRowOffsets[i + 1], &UiFontGlyphs[38], 0x7FF8, 0);
                }
            }
        } else {
            for (j = 0; j < 3; j++) {
                x += SaveListColumnSpacing[j + 1];
                arg1 = AddTilePrim(arg0, arg1, x, y + SaveListRowOffsets[i + 1] + 8, SaveListTileWidths[j], 2, 0xFF, 0xFF, 0xFF);
            }
        }
        if (i == SaveSlotIndex) {
            arg1 = AddTileSemiTransPrim(arg0, arg1, *(short *)(BadgeRows + arg2 * 0x1C + 4), y + SaveListRowOffsets[i + 1] - 4, sp40, 0x18, 0x20, 0x20, 0x20);
        }
    }

    arg1 = AddDrawModePrim(arg0, arg1, 0x38);
    y += 0x1C;
    x = *(short *)(BadgeRows + arg2 * 0x1C + 4);
    arg1 = AddTilePrim(arg0, arg1, x, y, sp40, 2, 0xFF, 0xFF, 0xFF);
    y -= 0x1C;
    for (i = 0; i < 4; i++) {
        x = *(short *)(BadgeRows + arg2 * 0x1C + 4);
        for (j = 0; j < 3; j++) {
            x += SaveListColumnSpacing[j + 1];
            arg1 = AddTilePrim(arg0, arg1, x, y + SaveListRowOffsets[i], 2, 0x10, 0xFF, 0xFF, 0xFF);
        }
    }

    return arg1;
}

#undef FormatDecimalDigits
#undef UiFontGlyphs

/* legacy typing of shared globals in this part of the unit */
#define FormatDecimalDigits ((void (*)())FormatDecimalDigits)

char *FormatCountOfTotal(int a, int b, int c) {
    int i = 7;
    char *p = FractionText + i;
    do {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);
    FormatDecimalDigits(FractionText, a, c);
    FractionText[c + 1] = 0x2F;
    FormatDecimalDigits(FractionText + c + 2, b, c);
    return FractionText;
}

#undef FormatDecimalDigits

void DrawControlTypeLetterPanel(a0, a1, a2)
int a0;
int a1;
int a2;
{
  char sp[8];
  int i;
  char *p;
  int s3;
  int s4;
  /* ASM_BARRIER: pin a0 copy to $s5 to match original save-reg allocation */
  register int aa asm("$21");
  int x;
  int nv;
  int y;
  int s2;
  int idx;
  int v1;
  int r;
  char *v0;
  char *g;
  aa = a0;
  g = BadgeRows;
  v0 = (char *) ((a2 * 0x1C) + ((int) g));
  x = *((short *) (v0 + 4));
  y = *((short *) (v0 + 6));
  s3 = x + 8;
  s4 = y + 0x24;
  i = 7;
  p = sp + 7;
  do
  {
    *p = 0;
    i--;
    p--;
  }
  while (i >= 0);
  idx = (MenuActivePlayerPlusOne - 1) & 1;
  if (idx < 0)
  {
    idx = 0;
  }
  v1 = ((S3 *) (PlayerControllers + (idx * 0x5C)))->w;
  switch (v1)
  {
    case 0:

    default:
      s2 = *((short *) (Options + (idx * 4)));
      break;

    case 1:
      s2 = ((S1 *) ((idx * 16) + ((int) Options)))->s;
      break;

    case 2:
      s2 = ((S2 *) ((idx * 4) + ((int) Options)))->c;
      break;

  }

  r = AddSpritePrim(aa, a1, s3, s4, D_8009FBEC, 0x7FF8, 0);
  sp[0] = s2 + 0x41;
  nv = s3 + 2;
  AddDrawModePrim(aa, AddTextSprites(aa, r, nv + D_8009FBEC[2], s4, sp, 0x7FF8, 0, 0), 0x38);
  /* ASM_BARRIER: keeps s3 live to the end (original save-reg order) */
  __asm__ volatile("" : : "r"(s3));
}
