extern void (*SndSeqTickEnvUserCallback[])();
void _SsTickCallback(void) {
 void (**p)() = SndSeqTickEnvUserCallback;
 /* ASM_BARRIER: keeps SndSeqTickEnvUserCallback address in one register (lui+addiu) so p[-1] uses a negative offset */
 __asm__("" : "=r"(p) : "0"(p));
 if (p[0] != 0) p[0]();
 p[-1]();
}
