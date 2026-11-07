/* GCC_VERSION: 2.7.2 */
extern int CdAltReadyCallbackFunc[];
int CdAltReadyCallback(a0) int a0; { int *p = CdAltReadyCallbackFunc; int r = *p; *p = a0; return r; }
