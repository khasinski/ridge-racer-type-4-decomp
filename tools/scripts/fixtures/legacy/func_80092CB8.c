/* CC1_FLAGS: -mno-split-addresses */
extern unsigned char GpuDebugLevel[];
extern void (*GPU_printf)();
extern struct { char pad[0x10]; int (*f)(); } *GpuFuncTable;
extern char StrSetDispMaskFmt[];
extern int GPU_memset();
void SetDispMask(a0) int a0; {
 unsigned char *p = GpuDebugLevel;
 if (*p >= 2) GPU_printf(StrSetDispMaskFmt, a0);
 if (a0 == 0) GPU_memset(p + 0x6A, -1, 0x14);
 GpuFuncTable->f(a0 ? 0x3000000 : 0x3000001); }
