/* GCC_VERSION: 2.7.2 */
#include "psyq/libcd/dsstrm4.h"
extern char *StRingHeaders;
void init_ring_status(int a, unsigned n) {
    unsigned i = 0;
    if (n) { do { int t = i + a; i++; *(int *)(StRingHeaders + (t << 5)) = 0; } while (i < n); }
}
