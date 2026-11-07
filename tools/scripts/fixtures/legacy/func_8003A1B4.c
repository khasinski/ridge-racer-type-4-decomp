
extern int DebugPrintStub();
extern int AllocBadgeRows();
extern int AllocRect16Rows();
extern int AllocGlyphRows();
extern int AllocUiRows();
extern int AllocImageRows();
extern int CdDirectoryBuffer;
extern char StrCdReadPtrFmt[];
extern int Rect16RowCount;
extern int LoadCursor;
extern int LoadCursorMark;
extern int ImageRowCount;
extern int BadgeRowCount;
extern int UiRowCount;
extern int GlyphRowCount;
extern int MenuTextRowCount;
extern int MenuArenas[];
void InitMenuArenas()
{
  /* ASM_BARRIER: pin the 0x2D000 constant to $a2 so the addu and the call share one lui/ori like the original */
  register int c asm("$6") = 0x2D000;
  BadgeRowCount = 10;
  Rect16RowCount = 4;
  GlyphRowCount = 4;
  UiRowCount = 0x28;
  ImageRowCount = 1;
  MenuTextRowCount = 0x37;
  MenuArenas[1] = CdDirectoryBuffer + c;
  LoadCursor = MenuArenas[1];
  DebugPrintStub(StrCdReadPtrFmt, LoadCursor, 0x2D000);
  LoadCursor = LoadCursor + AllocBadgeRows(LoadCursor);
  LoadCursor = LoadCursor + AllocRect16Rows(LoadCursor);
  LoadCursor = LoadCursor + AllocGlyphRows(LoadCursor);
  LoadCursor = LoadCursor + AllocImageRows(LoadCursor);
  LoadCursor = LoadCursor + AllocUiRows(LoadCursor);
  LoadCursorMark = LoadCursor;
  MenuArenas[1] = LoadCursor + 0x19000;
  LoadCursor = LoadCursor + 0x19000;
}
