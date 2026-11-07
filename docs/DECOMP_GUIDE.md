> **Marker update:** `/* NOSPLIT */` is replaced by the generic `/* CC1_FLAGS: <stock cc1 options> */` (first lines of the C file, e.g. `/* CC1_FLAGS: -mno-split-addresses */`); cc.sh appends the flags to cc1. `/* NOSPLIT */` remains only as a deprecated alias for running agents.

> **BARRIER RULE:** every empty-asm barrier / `register asm` pin must have a `/* ASM_BARRIER: <reason> */` comment (same line or <=3 lines above). See docs/BARRIERS.md; audit with tools/scripts/list_barriers.py.

> **RULE (supersedes everything below): only STOCK tools — stock maspsx (tools/maspsx, unmodified upstream) and stock gcc 2.7.2 / 2.8.1 cc1 or the gcc-970318 snapshot cc1 built by tools/gcc-970318/build.sh (flags only, e.g. -mno-split-addresses via /* NOSPLIT */). NO assembler emulation (MASPSX_FILL / aspsx_fill.py), NO post-processing of cc1 output, NO gas27 pipeline. The `/* FILL */` marker and all "delay-slot fill emulation" notes below are obsolete and removed from the toolchain. Matches must come from C source alone.**

# Decompiling a function (PAL, Ridge Racer Type 4)

Goal: replace the asm of `func_XXXXXXXX` with C that compiles to **byte-identical** code.
Compiler: gcc 2.8.1 (PSX) `-O2 -G0 -mips1 -mcpu=3000 -funsigned-char` via `tools/scripts/cc.sh`
(cpp -> cc1 -> maspsx -> GNU as). Always run Python from the venv: `.venv/bin/python`.

## Per-function loop
1. Original asm: `build/baseline/asm/nonmatchings/<func>/<func>.s`
2. First draft: `tools/scripts/m2c.sh <func>` (needs cleanup: `M2C_FIELD(...)` -> real struct/array syntax, `M2C_UNK` -> int, etc.)
3. Write `src/main/PAL/<func>.c` while working (one function per file; once matched it is folded into its translation unit, see `docs/SOURCE_LAYOUT.md`; includes `"types.h"` if you use s32/u8...).
   Declare callees K&R style (`extern int SetDrawMode();`) and data as `extern int BadgeRowCount;` / arrays / structs.
   Keep the symbol names exactly as in the asm (`func_XXXXXXXX`, `D_XXXXXXXX`).
4. Check: `.venv/bin/python tools/scripts/fd.py src/main/PAL/<func>.c`
   - `MATCH` = done. Otherwise it prints original asm, our disassembly and the first differing word.
   - Adjacent functions that must be compiled together may share a file (functions must be a contiguous run in address order).
5. If the object contains `.rodata` (string literals, jump tables from `switch`, float constants) fd.py prints a line
   `/* RODATA 0x800105C8 0x30 */` - paste it at the top of the C file (fd.py finds the location in the original rodata block).
   `.data`/`.sdata`/`.bss` (initialised or static data) is not supported yet: skip such functions.
   Prefer extern symbols (`extern char StrSizeEqLdFmt[];`) over string literals when the asm uses a symbol that lives in rodata:
   a literal creates *new* rodata, an extern does not. Use literals only if the string really belongs to this function's unit.

## Rules
- Never edit anything except your own `src/main/PAL/<func>.c` files. Do **not** run `make`/`make split` (other agents work in parallel); `fd.py` is safe to run concurrently.
- Only leave a file in `src/main/PAL/` if fd.py reports MATCH. Delete failed attempts. No `INCLUDE_ASM`, no inline-asm tricks, no `#pragma`.
- Give up on a function after ~6 genuinely different attempts; leave it for later (note it in your final report with what blocks it).
- Functions using the GTE (cop2/mtc2/...) or hand-written asm are out of scope.

## gcc 2.8.1 matching tips (learned so far)
- Loop and call shapes are sensitive to *how the same value is written*: `f(a0, a1++); return a1;` matched where `f(a0,a1); return a1+12;` did not.
- Unsigned `short`/`char` parameters/arguments produce `andi` masks; `int` ones do not.
- Struct arrays: `p[i].x` vs pointer walking compile differently; try both. `i < n` vs `n > i`, `for` vs `while` vs `do/while` change block order.
- `(x + 1) % n` emits the `div`/`break 7`/`break 6` sequence (maspsx expands it); `x / 1024` emits the shift-with-bias sequence for signed int.
- Temporaries: introducing/removing a local, or moving a load before/after a call, changes register allocation and scheduling.
- Function pointer/address constants like `0x80034E70` in the asm are usually `(int)LinkTimeoutCallback` (lui/addiu pair with relocation); a plain literal gives `ori`.
- Switch statements: the jump-table order (rodata) must match too; case bodies are emitted in source order, table slots follow case values.
- PsyQ library functions (>= 0x8007D000) are lower priority; `configs/PAL/psyq_hints.parasite.txt` lists likely names from another game.

## More tricks (from chunk 4)
- Struct passed by value from stack locals (`lhu/lhu/sll/or` into `$a0`): declare `struct { V a, b; } w;` and pass `w.a, w.b` with `typedef struct {short x,y;} V`; two separate `V` locals give a different stack layout.
- Round up to 4: `int v = a; int r = v & 3; if (r) v += 4 - r;` gives copy-then-branch; `a + 4 - r` does not.
- Address order: if the original does `lui` of the array base before `sll` of the index, write `T *base = arr; T *t = base + i;` instead of `&arr[i]`.
- Case bodies are emitted in source order (default/grouped cases must be placed first if they come first in the asm); table slots follow case values.
- If the original re-reads a value it just stored, re-read it from the expression (`z - t[2]`) so gcc keeps the copy in a register.
- If the original overwrites an argument register, reuse the parameter variable in the source (`i -= half;`).
- Check which argument registers the asm reads: a function that uses `$a1`,`$a2` but never `$a0` needs a dummy first parameter.
- `p[7] = 0x40; p[7] = c ? 0x42 : 0x40;` reproduces store/branch/store; two plain `if` stores do not.
- Signed `x / 2` emits `srl/addu/sra`; two divisions of the same sum reproduce some originals.
- Known layouts: `PlayerControllers` array of 0x5C-byte structs; `PadPulseState` 0x18-byte elements of shorts.

## More tricks (from chunk 6)
- In-place signed `/256`: `if (v < 0) v += 0xFF; x = v >> 8;` (plain `/ 256` adds a register copy).
- Scratchpad/hardware addresses: `int *g = (int *)0x1F800000;` then `g[7]` etc. keeps one `lui`-only base; pick the base so offsets equal the asm `%lo` offsets.
- `char *p = D_X;` at the top stops gcc re-materialising the address after each call.
- Consecutive `*p++ = const;` gives `sw` + `addiu 4`; `do { D[i+1] = ...; i++; } while (i < D[0]);` with a pre-incremented base pointer matches index loops.
- A small int `switch` (3 cases) compiles to a compare chain with no `.rodata`.
- `-funsigned-char`: plain `char` loads with `lbu`; use `signed char` when the asm uses `lb`.
- Callee arg registers that are just leftovers are not arguments: declare callees K&R with fewer args; a dead `mfhi` is not a parameter.
- Many 0x40-0xA8 byte functions around 0x80061000-0x80062000 are hand-written polygon-packet macros (no prologue, bare `$t`/`$s` registers, clobber `$at`): not C-able, skip.

## More tricks (from chunk 7)
- `int *sp = (int *)0x1F800000; sp[7] -= 0x800;` gives `lui` + offset like the original; `*(int *)0x1F80001C` gives `lui`/`ori`.
- m2c invents arguments from stale `$a1`/`$a2`: if the asm never sets them before the call, drop them.
- `if (r==5) X; else if (r==6) X;` matches shared-label chains; `r==5 || r==6` and `switch` did not.
- Reuse a parameter as the loop counter (`a1 = 0; ... a1++`) to avoid a copy; `t->arr[i]` avoids the extra induction register of pointer walking.
- gcc hoists loop-invariants (e.g. `e << 13`) into callee-saved registers; that explains `addu sN` copies.
- When independent statements are scheduled differently, brute-force statement permutations with a script.
- 4-byte-stride tables: `unsigned char T[][4]` or `short T[][2]`; read the index into a local `int i` first.
- A local declared `int` (not `short`) gives `lh` instead of `lhu`.

## More tricks (from chunk 1)
- `int c2 = c;` forces a fresh register copy of an argument (original `addu t0,a2,zero`, then a2 reused).
- Load fields into locals in the order the asm loads them, then call.
- Put `a[c] = b % 10 + 0x30;` before `t = b / 10;` when scheduling differs.
- `a / 1000 / 60` folds into one multiply; store `s = a / 1000;` in a local first, then `m = s / 60`, `r = s % 60`.
- m2c shows `s16` loop counters; the original usually has no sign extension - use `int`.
- A `*(short*)` load feeding `& 0xFFF` can narrow to `lhu`; assign to an `int` local first to keep `lh`.
- `*d++ = *s++` on a 4-word struct gives load-4-then-store-4; the order of `i++; dst++; src++` changes register assignment.
- `#define W(p,o) (*(int*)((char*)(p)+(o)))` accessors are fine for big unknown structs.
- zsh: a glob matching nothing aborts the whole command line; use `sed -i ''` on macOS.

## More tricks (from chunk 3)
- To let gcc hoist a load above an unrelated store, type pointers as pointers to structs with distinct member types (with `char*` + casts it will not reorder).
- `*(V *)dst = *(V *)src;` with a 4-word struct gives 4x `lw` then 4x `sw`; per-field copies interleave.
- Early returns: `if (cond) { ...; return 0; } return -1;` beats `r = -1; if (...) r = 0;`.
- Switch state machines: add the `/* RODATA ... */` line fd.py prints; use plain `case N:` labels and let cases fall through, even into the middle of an `if` block. Table words are in `build/baseline/asm/data/main.rodata.s`... (look at `asm/PAL/main/data/*.rodata.s`).
- `jal f; nop` with no a-register setup = zero-argument call; check whether a0/a1 flow through untouched from the caller (then they are real parameters).
- `i * 20 + (int)base` gives `addu v0,a1,v0` (offset first); `&base[i]` gives the reverse operand order.
- `int i = 7; char *p = arr + i;` makes a register `addu` pointer instead of folding into the address.
- A `u8 s` accumulator that is only returned changes register choice vs `int s`.
- Declare stack locals in stack-offset order; the order of stores in source also matters.
- `p = arr; p += 0x578;` as two statements gives lui, addiu lo, addiu 0x578; one expression folds to a single addiu.

## More tricks (from chunk 5)
- Big global struct: declare `extern struct { char pad[0x2664]; s32 a; ... } SoundState;` and write `D.field` (folds symbol+offset like the original; a char-array+cast macro did not).
- Keep base register separate from offset in a loop: `(char*)D + i*0x1C` plus constant offset instead of a walking pointer.
- A load whose result is unused is deleted by gcc; declare the field `volatile` to keep it.
- `idx*4 + (int)ptr` vs `ptr + idx` swap the `addu` operand order.
- `struct S *p = &D_X;` at the top used for all accesses changes scheduling enough to match sometimes.
- Swapping then/else bodies of an `if` flips emitted branch layout (binary-search loops).
- `u16 a0` parameter masks only at use; where asm shows inline `sll/sra`, plain `int` params with `(s16)a3` casts at the use matched.
- Magic `0x82061029` = signed /16129; `0x66666667` + `sra 2` = /10.
- `dst = *src` on a struct of `char b[N]` makes the `lwl/lwr` copy loop (size N is the full struct size, e.g. 0x5BC).
- `int a[4] = {1,4,5,6};` locals create a `.rodata` copy that fd.py matches with a RODATA line.
- Stack buffer starting above the first free slot: wrap locals in one struct with a leading `pad[8]`.
- Nested `if ((v&0x10)==0) { if ((v&1)==0) ...}` gives separate `andi/bnez`; `a && b` does not.

## More tricks (from chunk 2)
- Struct block copy `D[1] = *a0;` with a big struct (fields at real offsets) emits the copy loop.
- Unexpected `andi 0xff` -> field/local needs `signed char`; plain `char` gives `lbu`.
- `*(short *)&p->b = p->b;` reproduces byte load + `sh`.
- A dead `int t[2]` stack array can reserve stack slots; bubble-sort swap `x=D[j-1]; y=D[j]; D[j]=x; D[j-1]=y;`.
- Chained early returns `if (c) { D |= n; return; }` match repeated `return` patterns.
- Layouts: `SelectionState` byte fields 0x12..0x1E; `PlayerCars` array of 0x320-byte structs.

## More tricks (from chunk 0)
- Fixing m2c types often suffices: `*(void **)0x1F800000 = ...`; byte constants like `0xF0` through `s8` emit `addiu -16`, use `u8`.
- m2c types pointer-like globals as `M2C_UNK` (s32): `&D + n` scales by 4. Declare `char D[]`/`char *D` instead.
- Packet builder `...; func(a0, p); return p + N;` = typed `struct P *p; func(a0, p++); return p;` with sizeof(struct) == N (not `void *`).
- `if (c) { ...; return x = 1; } return 0;` often does not match; `if (c) return 0; ...; return x = 1;` does. Avoid m2c `goto block_N`; write explicit early returns.
- `if (c) D = 8; else D = 0xB;` as explicit stores per branch, not temp/ternary.
- Indexed struct array `D[i].p[3] = 0` gives a different loop base than pointer walking - try both.
- Local `char *p = D;` makes gcc compute `D + 0x227D6` in a register instead of folding into the relocation.
- `struct { unsigned char c[8]; }` assigned from an extern gives the `lwl/lwr/swl/swr` 8-byte copy.
- A local reachable only via `&local` store may be dead-stored away; declare array `short sp[4]` and pass `sp`.
- `int pad[6]` locals fix a too-small frame; try moving each independent store to every position and re-run fd.py.
- A struct-field store plus scalar load at constant address are assumed not to alias; use a raw `*(int *)p` store when the original re-reads.
- Use unique helper script names (the shared scratchpad is shared between agents).

## Two compilers
The PsyQ library region (libapi/libcard-like code, ~0x80097000+) matches **gcc 2.7.2**, game code matches 2.8.1.
Put `/* GCC_VERSION: 2.7.2 */` as the first line of a C file to compile it with `tools/toolchain/gcc-2.7.2/cc1` (cc.sh and fd.py honour it).
Tell-tale for 2.7.2: jump tables loaded through `lui $at`, `jr; nop` with unfilled delay slots. Try both compilers on library functions.

### Third compiler: gcc-970318 (PsyQ 4.5+ library build)
Sony's later SDK libraries (libsnd's `_SsContDataEntry` = _SsContDataEntry) were built with a GCC development snapshot of
1997-03-18, between 2.7.2 and 2.8.0: GCC 2.8's 4-byte reload spill slots (consecutive `sw` spills at 4-byte steps; stock
2.7.2 rounds every spill slot to 8) with a 1996 loop.c that hoists loop invariants differently from both releases.
Build it once with `tools/gcc-970318/build.sh` (-> `tools/toolchain/gcc-970318/cc1`; source hashes are checked; needs
docker or bison >= 3.6 for the parsers), then mark the file `/* GCC_VERSION: 970318 */`; cc.sh adds Sony's library flags
`-mhard-float -fno-exceptions`. The only change to the compiler is `mips_split_addresses = 0` (it has no
`-mno-split-addresses` yet). Try it when a library function is far off under both 2.7.2 and 2.8.1, especially if spill
slots / hoisting are the difference. Calls through SsFCALL-style tables match with an unprototyped `void (*tbl[])()`.
maspsx cannot emulate the original assembler's delay-slot filling (`lui $at; sw` expanded before `jr`/`jal`), so such functions cannot match.

## Delay-slot filling (library code)
`MASPSX_FILL=1` (set by cc.sh for `GCC_VERSION: 2.7.2` files) runs `tools/maspsx/aspsx_fill.py` after maspsx: it emulates the original
assembler (moves the instruction before a branch/jump into its delay slot, `lui $at; sw` macro form, no spurious nops after div/rem).
Use `tools/scripts/place.py file.c...` to try a candidate under both compilers and install it if it matches.

## More tricks (wave 2, chunk 4)
- `*(volatile int *)` cast on the packet-link store just before `*(unsigned char **)0x1F800000 = p` stops gcc moving the global store up.
- `(short)x / 2` on a `u16` reproduces `sll 16; sra 16; srl 31; addu; sra 1` with one `lhu`.
- Store the result separately in each if/else branch instead of through a shared variable: gcc cross-jumps the identical tail with the right load order.
- Put `case 1:` inside the `if` block of `case 0` for original fall-through layout; a `default:` body can sit between cases.
- RODATA hex in the annotation can be lowercase.
- Untried in that range: InitComCars and HsvToRgb (jump-table switches) and the large functions (>0x300 bytes).

## More tricks (wave 2, chunk 2)
- Tail duplication: copy a tail statement into every arm of an if/else chain; crossjumping merges them back and gcc keeps `lui hi(sym)` in a callee-saved register across calls.
- Switch on a plain `int` local copy, not an `unsigned short` global (avoids `andi` masks).
- Call with fewer args than the callee declares if the extra register is a leftover (cse otherwise replaces it with a constant).
- `b = (int)tab; t = (T*)(off + b);` gives offset-first `addu` and fixes `lui` hoisting into delay slots.
- A local `int n = N;` keeps a loop bound in a register and avoids down-counting loop reversal.
- Stop gcc folding `n-1-r`: `r = x % n; r++; i = n - r;` as separate statements.
- A `signed char` local compared to a constant gets constant-propagated; use an `int` local to keep the register copy.
- Unaligned 4-byte copy: `__attribute__((packed)) struct {int x;}` assignment gives lwl/lwr/swl/swr (`struct {char b[4];}` crashes cc1).
- A shared `return s1;` after switch `break`s prevents constant-folded returns; `goto` into an else block + swapped polarity matches shared-block layouts.
- Declaration order changes callee-saved register assignment: run a permutation script.

## More tricks (wave 2, chunk 6)
- `jal` + `nop` with `a0` untouched, or a `lh a0` before a seemingly arg-less call, means `a0` is a real argument.
- 2D arrays (`tbl[m][k]`) for stride tables give the original base+offset folding and operand order.
- Switch cases sharing a store tail only crossjump if the `val`/`ns` locals are loaded first.
- `*(short*)((char*)e + i*4 + 0x18)` keeps an induction base from being folded.
- Separate locals per phase give distinct register assignments; separate arrays (not one struct) avoid a combined base register.
- A `short` global cast to `unsigned short` gives `addiu -1` instead of `ori`.
- cc1 2.8.1 can segfault on nested switches: use one flat switch.

## More tricks (wave 2, chunk 7)
- Separate locals for separate call results (reusing one `r` forces an extra copy); single `return ret;`.
- `goto` to a shared `ret = 1` tail reproduces "delay-slot copy, then jump past".
- Put `default:` with `case 0:` when the out-of-range target equals case 0.
- Derive strength-reduced pointers from `i` (`a0 + 0x80 + i*0x80`) instead of a separate offset variable (changes preheader order).
- Brute-force independent store order in a loop body (4 stores = 24 permutations).
- Declare narrow stack params as `int` and cast at use: `(short)a2 == 1`.
- m2c for a function with a jump table: append the jtbl rodata `.s` file to the m2c command.
- cc1 2.8.1 breaks on mixed declarations (declare locals first) and crashes on anonymous struct types in casts (use a typedef).

## More tricks (wave 2, chunk 3)
- Reuse a parameter as the loop counter/pointer (`a0 = (int)D_X; ... a0 += 0x14;`) to force the original register choice.
- Keep a constant as a distinct local (`int c = 0xB; ... t != c`) to change where its load lands vs hoisting.
- One-pointer strength reduction: `q = (uchar*)p + 7; q[i*20 + k]`.
- Read a global inside the loop body (`d = D_X;`) to stop gcc caching it in a saved register / hoisting its `lui`.
- Hoisted-constant order follows first use; pre-compute `t = (unsigned)a1 & 0xFFFFFF;` before the statement to flip two hoisted masks.
- Abs diff: `v1 = (short)a1; if (v1 < x) v1 = x - v1; else v1 -= x;`.
- Unused `int pad[2]` grows the frame by 8; `(unsigned)x % 11` gives multu 0xBA2E8BA3; `x / 1024` gives the shift-with-bias sequence.
- A `lui` in a branch delay slot is gcc's reorg copying the branch target's first instruction, not hoisting.

## More tricks (wave 2, chunk 1)
- `signed char` arrays holding -1 (else `addiu 255`); `int` locals when the original loads with `lb` not `lbu`.
- `int d = X - 2; x = d + a;` reproduces a hoisted subtraction.
- `j = i; if (i < 0) j = K + i; i = j;` forces the extra copy.
- `goto fail` into `if (t == -1) { fail: return 0; }` shares one failure return.
- Declaring an extern as returning `int` (even if unused) can change output.

## decomp-permuter (near-miss search)
`tools/scripts/perm.sh func_XXXXXXXX [seconds] [source.c]` sets up `build/perm/<func>/` from your current best C file and runs
the permuter (random statement/expression rewrites scored by instruction diff). If a zero-score result appears
(`build/perm/<func>/output-0-*/source.c`), copy it to `src/main/PAL/<func>.c` and run fd.py. Use it only for near-misses (score < ~300);
set PERM_JOBS to limit CPU. The `build/perm` dirs are separate per function so agents can run it in parallel.

## GTE (COP2) functions
Inline asm works: `__asm__ volatile ("mtc2 %0, $0" : : "r"(a));`, `__asm__ volatile ("mfc2 %0, $25" : "=r"(x));`, `__asm__ volatile (".word 0x4A180001");` (RTPS; opcodes in include/gte_macros.inc),
`lwc2 $n, off(%0)` / `swc2` with an `"r"` base and `"i"` offset or `"m"` operand. Surrounding register allocation is still decided by gcc, so write the C around the asm as for any function.
Units with inline asm are acceptable; no hand-written `.s` replacements.

## More tricks (wave 3, lib 2)
- `x / 2016` (magic multiply + sign correction from the unshifted value) is written `(x >> 2) / 504`.
- Hardware-register pointers: `extern volatile int *D_X;` + a dead `*D_X;` statement reproduces an unused `lw $v1,0(ptr)`.
- A `& 0xFFFF` on a return value: declare the callee `extern unsigned short f();` and return it directly.
- Return inside the `if`, then `return 1;`, reproduces library layouts where `r=1` sits in a delay slot (a shared `r` does not).
- Returning the input pointer (`return p;`) explains an `addu $v0,$a0,$zero` copy in struct-fill functions.
- When the callee's `$a0` is untouched, pass the incoming argument through (`f(a, D, 0x5C)`).
- Helper loops that copy a file must strip the GCC_VERSION marker or keep it consistently.

## More tricks (wave 3, game 4)
- Opaque constant variable (`int e = 0x44; ... a + (b + e)`) stops gcc reassociating a sum.
- Nest a call in another call's argument (`f(g(2, x), D)`) so cse reuses the constant argument register.
- Duplicate the call in each switch case so crossjumping rebuilds the shared tail with per-case setup.
- `if (v == 0) goto out; ... out: return v;` stops gcc folding `return v` into `return 0`.
- A struct local forces stack-slot layout (no leading pad needed; the outgoing-arg area already takes the first 0x10).
- A `volatile` reload of a byte forces re-read after a store; a store through `*(volatile int*)` keeps a final global store last.
- Signed /4096 in place: `if (v<0) v += 0xFFF; v >>= 12;`.

## More tricks (wave 3, game 6)
- atan2-style table functions: negate args in place (`a1 = -a1;`), `int` return so the `lh` is not re-extended; `if (a<b) return X; return Y;` gives the right block order where `if (a>=b)` does not.
- Packet-link code: `int *g = (int *)0x1F800000; char *p = (char *)g[0];` then a second link store via `int *o = (int *)(g[1] + off);`.
- `typedef struct { int x, y; } __attribute__((packed)) T;` gives the lwl/lwr/swl/swr pair copy; plain aligned struct assignment gives paired lw/sw.
- Three separate int locals passed by `&` get their stores deleted; declare `int sp50[3]` and pass `sp50` to keep them.
- m2c can scale a `u32 *` walk wrongly (stride 16 vs 4 bytes): check the asm stride.
- Around 0x80061384-0x8006D4FC (sizes 0x40/0x50/0x54/0x5c/0xa8) and 0x800678A8-0x8006A3D0 are hand-written polygon-packet fragments; SubmitModel/F3D0/F434/FC74 are interpreter fragments: skip.

## More tricks (wave 3, lib 1)
- Adjacent SDK symbols that are really one object: declare one struct (`extern struct { u8 f[0x20]; struct {short a,b;char pad[12];} e[24]; } _svm_sreg_dirty;`); gcc 2.7.2 then keeps the original memory-op order. Save the loaded byte in a temp (`t = f[i]; ... f[i] = t|3`).
- 2.7.2: `*(T*)((int)ARR + idx*4)` yields `lui v1; addu v1,v1,idx; lw v1,lo(v1)`; `ARR[idx]` does not. Then `p += i; p->field` gave the matching final `addu`.
- Return `f() == 0` after an early branch: `int r; if (...) { r = f() == 0; } else r = 0; return r;`.
- Short params copied with `addu a3,a1,zero`: use `int` params with `(short)` casts at use and compute products into locals up front.
- Many libgpu/libspu functions use a full symbol address in one register and reach every field as `off(r)`; gcc folds those into separate `lui $at` accesses whatever you write - currently unmatched.

## More tricks (wave 3, game 3)
- Hoisting into a saved reg only happens if the pseudo is set once; reuse a variable assigned elsewhere to stop it (`new_var = i; ... new_var = 0xC000;`).
- Strength reduction: index 2D arrays / array-of-struct by loop counter (`sp[0][i]`) to get single base+offsets; pointer walks create extra induction vars.
- Switch layout: guard `if` before switch plus extra `case -1: return;` reproduces `beq 0; blez; beq 1`. `slti ..,2; bnez` before `beq 2` => `case 0:` merged with `default:`.
- Frame size: unused `int pad[N]` adds stack bytes.
- Prototype with `unsigned short` param makes callee arg get `andi`. m2c misjudges arg counts from leftover $v0.
- Saved-reg count: `(a3 = 8), a3, a3` as call args reduced saved regs 7->6.

## More tricks (wave 3, gte 2)
- GTE inline asm: one asm statement per op; for macros using $t regs use `register int t0 __asm__("$8")`; pointers via single asm with "r" input. GTE ops can be `.word 0x4A...` (mvmva = 0x4A400012|sf<<19|mx<<17|v<<15|cv<<13|lm<<10).
- Return arg with `jr;nop`/addu before: `__asm__ volatile("" : "=r"(r) : "0"(a1)); return r;` (2.7.2 marker puts addu in delay slot).
- Stack-arg loads hoisted above asm: declare params `T * volatile pN`, copy into `register ... __asm__("$8")` after cop2 op.
- Mask 0xFFFF0000: inline `"and %0,%0,0xFFFF0000"` gives `lui $at; and`.
- `p + (i<<2)` => `addu base,idx`; `p + i*4` => `addu idx,base`.
- Packed 8-byte copy: `typedef struct{int x,y;} __attribute__((packed))` -> lwl/lwr/swl/swr.
- Soft-float doubles: declare libgcc callees returning `long long`, use `LL` constants.
- NEVER `pkill -f permuter.py` (kills other agents); use `pgrep -f perm/<func>`.

## More tricks (wave 3, game 2)
- Constant-offset address in a loop gets its own s-reg; writing `p[0x5C+2]` folds it into the load (changes frame/s-reg count).
- gcc reassociates `y - 2 - x` into `y - (x + 2)`.
- Jump table with case 0 pointing at default label: write `case 0: default:`.
- K&R-style leftover-arg calls (declare 4 params, pass `(a0,a1,a2,a3)`) put loop temps in arg regs like the original.

## More tricks (wave 3, game 7 / gte 0)
- Copy original CFG with explicit gotos/labels (incl. do-while) to reproduce block order.
- Scratchpad 0x1F800000: read into local before a branch chain to hoist `lw`; use literal `*(int*)0x1F800000 =` for last store (`lui $at; sw`).
- Assign constants inside each if-arm (`a0 = 2; goto L;`) to avoid CSE of delay-slot addiu.
- `for (i=0;i<D_global;i++)` gives bound copy `addu a3,v0,zero`; cached local does not.
- Signed /8192: `v=a1; if (v<0) v+=0x1FFF; a1=v>>13;`.
- Cast table access `*(char**)((int)TAB+idx*4)` blocks scalar store hoisting above symbol-array load.
- sched1 may hoist `li -1` above a preceding call when a loop exists (no source fix found).
- Double-indexed table: `unsigned char (*tab)[4]; tab = TABLE; e = tab[i];`.

## More tricks (wave 3, game 0)
- Too-small stack frame: unused `int pad[18]; int *pp = pad;`.
- Tail-block-only diffs: leave to perm.sh (found UpdatePlayerCar in seconds).
- Switch with few cases = compare chain; list shared-body cases too (`case 0: case 1: case 5: break;`); use goto to end label for targets after fall-through.
- Write stores in natural field order; temp vars for repeated constants get propagated back.
- Struct-cast scratchpad access `((struct G*)0x1F800000)->ot` forces reloads (alias).
- Assign `g=(int*)0x1F800000;` inside loop body just before first use to control hoist order.
- `short` (not unsigned short) field arrays keep `& ~0x1F` masks in regs.

## More tricks (wave 3, lib 3)
- `unsigned char c = 0xFF;` declared between `p =` and `i =` reproduces a hoisted constant reg.
- MMIO: `*(volatile unsigned short *)0x1F801120` keeps lui/ori and prevents loop hoist.
- `int *p = D_X; r = *p; *p = a0;` gets register-held address.
- `unsigned a0; v = a0 & 0xFFFF; if (v >= 3) return 0;` gives slti on masked value.
- `goto out;` to shared `return 0` reproduces epilogue `addu v0,zero`.
- Unresolved: _cardCmd59Begin-family `li v0,1` delay-slot placement; tiny if-conversion functions (xori/sltiu/negu/andi).
- `-mno-split-addresses` is rejected by 2.7.2 cc1.

## More tricks (wave 3, gte 1)
- Pin GTE results with `register int x __asm__("$3")` to match reg choice.
- Loop shape: `if (n-- > 0) { const = N; do {...} while (n-- > 0); }` reproduces hoisted const + loop entry.
- Put op and its `&f.fl` address in one asm statement so gcc doesn't hoist the address.
- Permuter can't parse asm: replace with stub calls, expand back in custom compile.sh.
- gcc 2.8.1 rejects mixed declarations and empty clobber list `: : :)`.

## More tricks (wave 3, game 1)
- Arg setup (`addu a0,zero,zero`) hoisted into branch delay slot: make the callee call zero-arg.
- Out-of-line flag-set-then-jump-back blocks: literal gotos in asm order; duplicate trailing stores into both arms.
- Original reloads memory after every store: write with `char *p` and `*(T*)(p+off)` casts, mirror asm 1:1 incl. switch case order.
- `*(short*)p * 8` stored to short gives `lh`; `t=*(short*)p; t<<3` swaps regs.
- `if (X<0) v=-X; else v=X;` on memory lvalue reuses in-place sll/sra.
- Permuter base.c fails on `__attribute__((packed))` typedefs.

## More tricks (wave 3, game 5)
- "Return v0 directly": make every path reach one shared `return r;` (switch: `r=0; break;`; gotos: `if (r==0) goto out; goto inc; ... out: return r;` — polarity matters). Plain `return 0` emits `li`.
- Tail-duplicate call+return in each switch case so gcc keeps `lui` base in a saved reg.
- Small switch with consecutive cases (0/1 shared, 2) reproduces `bltz / slti 2 / beq 2`.
- Interpolation: load field into `unsigned short` first; separate statements `v=(to-from)<<12; v=(v/d)*(short)t; v=(v>>12)+from;`.
- `char *g = D_xxx;` with `g + i*0x14` indexing avoids strength reduction; `int i = 0;` first fixes prologue order.
- Frame 8 bytes too small: `int pad[2];`.
- `if (n>0)` guard: set loop pointer inside, then `do {} while (++i < n)`.
- Temp assigned before final stores fixes store/load order.
- Don't run permuter jobs alongside fd.py checks (timeouts).

## More tricks (wave 3, lib 0)
- `register T x asm("$N")` local pins a pseudo to a hard reg; fixes pure regalloc diffs (can pin several at once).
- Reload after store: volatile read, e.g. `c->c8 = *(volatile int*)&c->c4`.
- 2.7.2 allocates unused frame for `for` over struct-by-value param: use `if (n>0){ i=0; do{...}while(i<n); }`.
- Struct-by-value helpers (`typedef struct{unsigned lo,hi;}`): `x = f(x,y); return x;`; statement order of t[1]/t[0] matters.
- Narrow param types (`signed char`/`short`/`int`) decide sext vs raw at call site.
- Early `return -1`: `if (cond==1){...return 0;} return -1;`.

## More tricks (wave 4, c6)
- Forced reg copy: `__asm__("" : "=r"(w) : "0"(v));` keeps `addu v1,v0,zero`.
- Pin loop bound/counter/pointer with `register int n asm("$3")` etc. (pins stop permuter parsing).
- Late-hoisted lui in delay slot: `register short *g asm("$17")` + `asm("" : "=r"(g) : "0"(0x1F800000))` placed after the `if`.
- `b*2 + ((a<<4) + base)` fixes addu operand order; `base=(int)D;` read after second call fixes lui position.
- Two separate return tails: `if (h==-1) goto out; ...; return r; out: return r;`.

## More tricks (wave 4, c2)
- Don't pin the loop counter with `register asm` (blocks strength reduction); use separate unpinned loop var, pin the others.
- Pinning owner variable to $20/$21 can reproduce lui/addiu placement for constants kept in saved regs.
- SetNextModeByGameType: switch head whose jump targets are sibling functions; not standalone-matchable.

## More tricks (wave 4, c1)
- Prologue addiu scheduled one slot early vs store: chain assignments `D_x = p = a + 0x808;`.
- `register ... asm("$5")` pinning fixes operand/reg choice (GetTrackHeadingAt, 80026998).
- `b = (int)ptr; p = (char **)(b + n*4)` controls addu operand order.
- Scratchpad RMW: `{ int t = *(int*)0x1F800000; *(int*)0x1F800000 = f(..., t, ...); }`.

## More tricks (wave 4, c0)
- Copy params into pinned regs (`register int p asm("$17") = a0;`) to fix s-reg order; pinning to a param's own incoming reg forces an extra entry copy.
- Assign pinned constants in the exact order the original loads them, right before their stores.
- `((x+2)/3)*0x18` before loop reproduces hoisted division.
- Outer loop ending `s0++; s1 -= 1; } while (s1 > 0);` gives right inc/dec order.

## More tricks (wave 4, c8)
- cc.sh markers: `/* NOSPLIT */` (2.8.1 only) adds `-mno-split-addresses` (full symbol address in one reg; libsnd/libcd/libcard style). `/* FILL */` = MASPSX_FILL=1 + 3 regex fixes on cc1 output (epilogue `lw ra; j ra; addu sp` -> `lw; addu; j; nop`; nop before loop label after load; nop after lw before jal delay-slot store of loaded reg). Both = "nf". place.py doesn't try these; install by hand.
- `u8` loop var gives repeated `andi 0xFF`; `(char*)(idx*32+(int)ptr)` gives offset-first addu; 54-byte struct array `D[i].a` gives `lui at; addu at; lh`; `v1 -= 16` in place forces re-extract; `*(volatile T*)` forces reload; constants inside `if` flip branch polarity.

## More tricks (wave 4, c5)
- Split base into two pointer vars (`char *b = D; char *q = b;`) to control which induction pointer inherits the base reg.
- Delay-slot `addu a2,zero,zero` before `j L64`: goto the label containing the `a2 = 0;` statement, not `a2=0; goto L64`.
- First loop as indexed array loop (`p[i+1] = i`) gives `lui; addiu lo; addiu 0x1C; sw 4(v0)`.
- 2.8.1 splits pointer biv into extra address bivs; index struct array by loop counter (`b[i].c`) to avoid.
- Pin temps to $2/$3 for v0/v1 allocation; don't pin pointers the loop strength-reduces.
- Near-miss scoring: normalized difflib diff of fd.py original-vs-mine disassembly.
- Everything from SubmitModel on is hand-written fragments (except UpdateScrollTrack).

## More tricks (wave 4, c9)
- `r = const; if (volatile_global != x) r = 0; return r;` : pin r to $2 and loaded value to $4, make global volatile (keeps unfilled `beq; nop` and early `li`). Matched _cardCmd58Tx/D7A0/DE30.
- Pinned regs can backfire: cse reuses known value (e.g. 0) and deletes `li v0,0` or reuses reg for `sw zero`.
- 2.7.2 if-converts to xori/sltiu/negu; if original has plain branches, use 2.8.1 + volatile global.
- Declarations at top avoid cc1 Bus error from mixed declarations.
- `PIPELINE: gas27` marker unreliable. place2.py (scratchpad) also tries NOSPLIT/FILL markers.

## More tricks (wave 4, c4)
- Right-size draft: pin-permutation search over `register T x asm("$N")` on a few chosen variables only; pinning all adds copies and merges lui/addiu.
- Line-placement search must skip declaration lines and require literal ": MATCH" (compile error can look like match).
- perm.sh runs time out at 120s: wrap in `timeout 118`, budget ~100s.

## More tricks (wave 4, c3)
- Stack layout of many scalar locals: group into one local struct (`s.m[16], s.n[8]`) + separate arrays; declaration order controls frame offsets.
- `do{}while` reading `t70.a[i][k]` and `t90.a[k][i]` reproduces transposed-index addressing.
- `(int*)0x1F800000` with indexed writes (`g[7]=..; g[6]+=..`) gives original scratchpad codegen.
- `int t = D_x; D_y = t; D_z[t+0xC] = 0;` with gotos reproduces block order.
- Don't loop try.sh over candidates with a MATCH file installed (it overwrites).

## More tricks (wave 4, c7)
- Symbol address kept in one reg: `T *p = SYM; __asm__("" : "=r"(p) : "0"(p));` (2.7.2 rejects "+r").
- `static inline int f(int x){return x != 0;}` in a condition changes CSE/regalloc (permuter found it, DispatchCardMenuStep).
- K&R definitions with `short` params reproduce libsnd extra t-reg param copies.
- `unsigned short *` stores give `ori 0xC000`; `off + (int)ptr` gives offset-first addu.
- `for(;;)` + `break` hoists loop-invariant constants; `return` inside the loop does not.
- Scattered-case switch = compare chain: write as if/goto chain.

## More tricks (wave 5, c4)
- Pin ONE variable only (_SsSetNrpnVabAttr1: just the `a12` copy to $17); pinning all broke it.
- Strength-reduced index: `P = (Ch*)(i*0xB0 + (int)*q)` and `i++` after last use of P (_SsClose).
- Compute struct ptr inside `if` body (`s0 = &s1->b;`), use `s1 = &D[s2];` in loop instead of `s1++` to avoid strength reduction to `s1+574` (UpdateComCarTrackProjections).
- Load order of scratch fields changes schedule; load `v = s1->s1D8;` before `<0` test fixes v0/v1 split.
- Forced copy `__asm__("" : "=r"(bk) : "0"(back));` before 2nd compare gives `addu v0,a0,zero` in branch delay slot (UpdateCarTrackNode). Run perm.sh first on big near-misses, finish by hand.
- fd.py writes build/fd/<stem>.o: use unique file stems.
- In branch tail `__asm__("" : "=r"(r) : "0"(0))` keeps `li v0,0` in each arm.
- `pgrep -f perm/<func>` matches your own shell; use `ps aux | grep permuter.py`.
- /* FILL */ regex breaks on volatile-comment lines in cc1 output.

## More tricks (wave 5, c2)
- After `p = &SYM;`, `__asm__("" : "=r"(p) : "0"(p));` forces address in reg for repeated `p[k]` stores (avoids `lui $at; sh`).
- `register int y0 asm("$4") = a5;` just before call forces arg-setup position.
- Original reads stack args at sp+frame+0x14/+0x18 => unused 5th param; declare it.
- Make pointer global `volatile unsigned *` to keep loads behind stores.
- Wait loops: `while ((*A & m1) || !(*B & m2)) { if (f()) return -1; }`.
- `char nv = b & 0xFF; v = (...) | nv` delays mask until after the first call.
- Separate `m = -1;` local before second -1 loop keeps constant in its own reg.
- Permuter helps on tiny functions too (score ~60 hinted at `char new_var` mask trick); base must not contain asm.

## More tricks (wave 5, c6)
- Loop-invariant hoist (`s3*4`) into saved reg: assign to temp pinned `register int t asm("$3"); t = s3*4;` keeps it in loop; pin outer counter to $19 to fix numbering.
- Statement-order search with dependency constraints: enumerate permutations of ~7-statement loop bodies.
- Ch-struct channel functions (0x8008xxxx): 2.7.2 with `short` params, `unsigned char a2`, `unsigned char i = p->b17` (2.8.1 never gives `lui s1; addu s1,s1,idx; lw s1,lo(s1)`).
- Soft-float int->double: return `long long` via `union { LL l; struct { unsigned lo, hi; } s; }`; write `hi = t | s3; hi |= e << 20;` as two statements.

## More tricks (wave 5, c1)
- Library code (0x8008xxxx-0x8009Bxxx): `/* NOSPLIT */`+`/* FILL */` or 2.7.2 plus `volatile` accesses reproduces unscheduled load/store order and load-use nops.
- `volatile int *p = SYM;` reproduces cached base pointers.
- `__asm__ volatile("" : "=r"(r) : "0"(0));` launders constants/symbol addresses against CSE.
- Compute derived pointers (`q = p - 1`) before stores so base ptr lifetime ends before calls; never pin a pointer to a caller-saved reg across calls.
- Split expressions into separate statements to control loop hoisting.
- IDEA: original assembler seems to fill a branch delay slot from an earlier independent instruction, not only the immediate predecessor; aspsx_fill.py handles only the immediate predecessor. Experimental patched copy: scratchpad/w5c1/exp/maspsx/aspsx_fill.py (not adopted; SpuSetTransferStartAddr still off).

## More tricks (wave 5, c3)
- Force constant return into reg: `register int r asm("$2"); __asm__("" : "=r"(r) : "0"(1));` stops constant propagation; lets `li v0,1` sit in branch delay slot. Shared fail block before success block (gotos in asm order) reproduces layout.
- gcc 2.7.2 reorg fills delay slot with first insn of fall-through block after sched1 moved it there; hoisted `lui` mask can land in a delay slot where original has nop (no source fix found).
- Pinning a var to a reg shared with an incoming param changes the prologue.

## More tricks (wave 5, c0)
- Loop over pointer array: index form `for(i=0;i<N;i++) A[i]->f` places base init after invariant hoists like the original.
- Original uses raw incoming arg reg but others via copy: `register int h asm("$4") = a0;` use `h` at the raw use; two copies: pin to t-regs ($8..$10) with K&R int params.
- Loop shape `addiu v0,s1,1; addu s1,v0,zero; sll v0,v0,16`: unpinned plain `short i;` with `i++; while (i < D)`; pinned short loop var doesn't match.
- `*(volatile int *)` field read stops hoisting/arg-reg copy; not for signed 16-bit (gives lhu).
- 2.7.2 lib functions with several `return -1`: shared `goto out; return r;` with r assigned before the store prevents crossjumping merge.
- 2.8.1: `f(2, CONST32)` puts `ori` in jal delay slot after `li a0`; original `lui; ori; jal; li` not reproducible.

## More tricks (wave 5, c5)
- Try every marker variant (plain 2.8.1, 2.7.2, NOSPLIT, FILL, NOSPLIT+FILL); one function matched only under NOSPLIT+FILL.
- `(one = 1) << k` inline (reuse `one` for later `= 1` store) stops 2.8 turning `x & (1<<k)` into bit-test; gives `sllv` with shared 1.
- Pass leftover K&R args through to fix pre-prologue scheduling.
- Re-extended short args: use int params with `(short)` casts at use; ANSI short params are not re-extended.
- `volatile` field when original reloads after a store.

## More tricks (wave 5, c7)
- Marker grid for library code: try all combos of 2.7.2 / NOSPLIT / FILL (NOSPLIT+FILL matched _SsSndSetVol).
- K&R `short`/`unsigned short` params create copies matching original prologues.
- Same pseudo for a constant in both branches stops hoisting out of a loop.
- Permuter zero-score result: build/perm/<func>/output-0-*/source.c.
- Use `objdump -z` and delete old .o before recompiling (zero-run elision / stale objects hide diffs).

## Update: aspsx_fill.py now adopts the look-back rule
Fill of a branch delay slot from an earlier independent instruction (look-back 12; never crosses label/directive/branch/load/$at use; no load-use hazard) is now in tools/maspsx/aspsx_fill.py (supersedes the "not adopted" note above). SpuSetTransferStartAddr matches (2.7.2 marker, `__asm__ volatile("" ...)`).

## Tricks (wave 6, c1 / c4)
- `char` store of -6 compiles as 250 under -funsigned-char: declare base as `signed char *`.
- Keep base reg/index load order: `{ char *b = G; int k = (short)H; *(char *)((int)b + k + 0xC) = 2; }`.
- `new_var = 0xE2000000` inside an OR expression flips operand register assignment (permuter find).
- `r = (unsigned short)f(); __asm__ volatile("" : "=r"(r) : "0"(r)); return r;` keeps `andi` right after the call.
- K&R `unsigned short` params spill to a new frame slot; `int` param with `(unsigned short)` cast at use spills to the original arg home.
- Try markers ("", 2.7.2, NOSPLIT) on every draft: caught 5 matches that failed under plain 2.8.1.
- Original `lw rd,sym(rs)` as `lui rd; addu rd,rd,rs; lw rd,lo(rd)` (rd != rs) can't be produced by stock maspsx (always uses $at) -> unreachable; skip those.
- Debt tracking (modelled on parasite-pc): `make debt` / `make debt-check` (tools/scripts/crutch_debt.py -> docs/CRUTCH_DEBT.md, baseline tools/crutch_debt_baseline.json).

## Tricks (wave 6, c0/c2/c7)
- Triage every m2c draft/failed draft with markers "", 2.7.2, NOSPLIT: many lib functions match with NOSPLIT alone.
- `__asm__ volatile("" : : : "memory");` as last statement stops 2.8.1 moving epilogue `lw ra/s*` past final global stores.
- Function ending `addu v0,s0,zero` before last store returns its pointer (`return p;`).
- Pin only loop vars to $16/$17; unpinned `int *q` may get the original `a0` where pin to $4 fails.
- `volatile int i; int pad[2]; for (i = 10; --i != -1;);` = volatile countdown delay loop under 2.7.2.
- Empty-asm copy then test original var: `__asm__("" : "=r"(a) : "0"(t)); if (t < 0) a += 7`.
- `__asm__ volatile("" : "=r"(r) : "0"(0)); return r;` pins delay-slot-hoisted `li v0,0` after final store.
- gcc picks induction base for packet stores from last store in source order.
- `volatile` pointers on loads/stores in a loop reproduce unscheduled bodies (lhu; nop; sh).
- Bigger local array (`short m[14]` vs `m[9]`) reproduces larger frame.
- Unmatchable with stock tools: functions whose original has `jr ra; addiu sp` / `sw` after `jal` in delay slots filled by the original assembler (gcc 2.7.2 leaves nop).
- For functions matching only under 2.7.2 with 'jr; addiu sp' epilogue difference, try plain 2.8.1 first (epilogue native there).
- (w6c3) Nested loops: plain indexing (`s1 = D + j*0x22778; s0 = s1 + (i*0x14 + 0x18)`) beats hand-built induction vars (gcc hoists big constants and spills).
- (w6c3) Many-arg functions: compute both differences before the clamp, keep one var `r` for all call results.
- (w6c3) fd.py exits 0 and prints nothing for an empty/failed-m2c .c file: always look for the literal MATCH line.
- (w6c6) `K - f()` as `li s0,K; subu s0,s0,v0`: `register int c asm("$16"); c = K;` as separate statement after the call, then `s0 = c - t;`.
- (w6c6) To get `ori v0,a0,k` in a branch delay slot: give result its own local pinned to $2.
- (w6c6) Reorg moves a block's first `li` into preceding branch's delay slot only if scheduler placed that `li` first in its block.
- (w6c6) Permuter rewrite `if (z || a->f20) {..} else {..}` with identical arms changes load order.
- (w7c4) Two signed mult-divides into separate locals (`m1`, `m2`) stops gcc sharing the multiply result.
- (w7c4) `k = 0xAF000; *(volatile u8 *)&X = 0; D = Y + k;` forces original constant and store order.
- (w7c4) fd.py's `/* RODATA addr size */` line must be pasted at top of the C file; fix size by hand if needed; table starting at index 1 needs `case 0: break;`.
- (w7c5) `(q==1)*4` -> `e = q == 1; t = tab + e*4;` gives slti/xori/sltiu; direct form becomes conditional add.
- (w7c5) Split `k = ((j&1)<<5)+0x70; x = A + k;` into two statements to keep inner-loop shape.
- (w7c5) Signed /8: `b = x >> 3; if (x < 0) b = (x + 7) >> 3;` gives delay-slot-precompute; /256: in-place `if (v < 0) v += 0xFF; ... >> 8`.
- (w7c5) Pass the pointer var explicitly as 4th arg to keep a3 alive across K&R calls; one var pinned $6 covers leftover arg regs.
- (w7c5) cc1 `-dS -dj -dl -dg` dumps RTL after sched/lreg/greg (x.c.sched, x.c.lreg, x.c.greg) — shows whether scheduling diff is dependence or source form.
- (w7c0) fd.py takes a source PATH (src/main/PAL/func_X.c), not a name. score.py ignores immediates/frame sizes: trust only fd.py MATCH.
- (w7c0) Pad a stack-local struct with `char pad[N]` to fix frame size. If struct size != real stride use `char*` macro accesses `P8(base, idx*0x24+off)`.
- (w7c0) `__asm__("" : "=r"(pw) : "0"(0x80110000))` forces a `lui` gcc would fold into a later offset.
- (w7c0) Permuter can't compile `__attribute__((packed))` typedefs: permute a plain-int copy. A copy of the loop head's first insn in the delay slot is normal gcc output.
- (w7c3) Opaque copy `asm volatile("" : "=r"(x) : "0"(x))` forces reg copy without folding; bare `asm volatile("")` right after a store fixes its scheduling order.
- (w7c3) Pin locally (one var), not globally: a global `$3` pin changed mflo regs elsewhere.
- (w7c3) Pattern `t1 = a2; if (a2 < 0) t1 = -t1;` with t0/t1/t pinned $8/$9/$3 (InterpolateLanePointXZ/610).
- (w7c1) Pin only 1-3 vars; compute through unpinned temps then assign to pinned var (`q = r/16; q = r - q*16; s0 = q - 8;`).
- (w7c1) Unused leading param as K&R `int a0` used directly (ANSI param + copy schedules entry copy last).
- (w7c1) Literal `1` as argument (not shared var) lets CSE reuse `li` for switch compare. Put pinned-var update after the call, not in arg list.
- (w7c1) Duplicate a global store into every if-chain arm: each gets own `lui hi`, reorg fills delay slots. Non-volatile fields where original stores in a call delay slot.
- (w7c1) gcc 2.7.2 spurious extra 8-byte frame (vars=8) = symptom of assembler-fill problem (unreachable with stock tools).

## Wave 8 (assumption: everything is reproducible with stock tools)
- (w8c7) CD/SPU library module (~0x8009xxxx): module-wide `/* CC1_FLAGS: -mno-split-addresses */` on 2.8.1 + volatile hardware pointers/globals + goto-shaped CFG with shared return labels. Non-volatile cast on last store before a `jal` lets reorg fill the slot. `asm volatile("" : "=r"(r) : "0"(1))` gives separate `li 1`; `"memory"` clobber as last statement fixes tails; pin $3 fixed 9B758/9BA04.
- Treat "assembler fill" diffs as a source/option problem, not an impossibility: sweep with tools/scripts/sweep_markers.py.
- (w8c5) RODATA lines are needed for switch jump tables (`/* RODATA 0x800113f0 0x1c */`); extra `case 2:` merged with `default`, reload `a0` from `sp[1]` before switch; duplicate the call in each branch to fix `a0`; `i*0x24 + (int)base` for addu operand order; pins h=$16 r=$17.
- (w8c5) FINDING (not adopted, needs owner decision): gcc 2.7.2 cc1 + tools/toolchain/gas-2.7/as (plain GNU as 2.7, reorder mode, no maspsx) reproduces _mainasu exactly — the soft-float/libgcc-style functions (DB54 E494 E9C4 E1B4 E818 E0B4 E944) seem to be assembled by gas 2.7 filling delay slots. Not used: project rule is stock maspsx only.
- (w7c2) Separate local var per code path (separate pseudos) changes s-reg assignment. Goto-loop with pre-hoisted pointer vars stops loop.c hoisting constants but keeps address hoisting.
- (w7c2) `*(short*)&ushort_var = -1` gives `addiu -1` (plain const store gives `ori 0xFFFF`). Operand order via `(char*)x + (i<<2)` or `k + (int)tb`; `(idx<<4) + (int)base` with int globals matches `(arg0*0x10)+D`.
- (w7c2) Shared goto labels for a common store (`s2:`/`m1:`) reproduce `j` into a shared `sh`. Plain `_svm_cur.fXX` struct members reproduce negative-offset addressing from CSE'd base regs.
- (w8c3) `/* CC1_FLAGS: -fno-strength-reduce */` stops constant field offset being folded into a walking loop pointer (UpdateLoopVoices) — then pin pointer + put `i = 0;` before `s5 = Options;` for prologue order.
- (w8c3) GTE: empty-asm copy of `p+8` before the final scratchpad store forces the computation after the stores (AddShadowDiscPrims).
- (w8c4) `a0 = (int)&D_x; return *(short *)a0;` forces original reg use; fake struct with big padding then `buf[0x160]` then pointer reproduces global accesses; `unsigned char` byte macros (signed gives `addiu -128`); local copy `q = P`; struct `{int p; int a[8];}` with descending fill loop for addressing.
- (w8c0) Empty function with frame: `asm volatile("":::"memory")` makes cc1 emit `j $31; addu sp` -> maspsx `jr; addiu sp` (DrawTextStub).
- (w8c0) Pin loop constant to a t-reg (`register int k asm("$25")`, declared WITHOUT initializer, assigned before loop) fixes hoist order + register.
- (w8c0) GPU packet header as bitfield `{unsigned addr:24; unsigned len:8;}`; `p->addr = ot->addr; ot->addr = (unsigned)p;` reproduces masks/hoist order. Pass `p++` as arg for `addu a1,p,zero; addiu p,p,N`.
- (w8c0) cc1 2.8.1 fails on pinned register vars with initializers, mixed declarations, anonymous unions.

## Research note: `lui rd; addu rd,rd,idx; lw rd,%lo(sym)(rd)` (rd as address temp) -- NOT reachable with markers (checked)

Evidence gathered (stock cc1 2.7.2/2.8.1, stock maspsx, project GNU as):
- GNU as itself DOES produce the rd form: raw `lw $16,sym($3)` (rd != base) assembles to `lui $16,%hi; addu $16,$16,$3; lw $16,%lo($16)`; only rd == base falls back to `$at`. So the original object was assembled from a plain `lw rd,sym(rs)` macro by an assembler that uses rd as temp.
- Stock maspsx (`__init__.py` ~925-1000, branch `is_addend and r_source`) always rewrites such a load itself into `lui $at / addu $at,$at,rs / lw rd,%lo(sym)($at)` (every ASPSX_VERSION: 2.21..2.79 only toggles `addiu $at`), so GNU as never sees the macro. It only leaves a load alone if the operand is already `%lo(sym)(reg)`.
- cc1 never emits `%lo(sym)(reg)` with an `addu` in between: tried 2.7.2 and 2.8.1 with default / -mno-split-addresses / -msplit-addresses / -mgas / -mmips-as / -mgpopt / -membedded-pic / -mabicalls / -O1 / -O3, over global int/short/char/pointer/struct arrays, variable and masked indices, loops, stores, absolute (numeric) addresses. Split mode hoists `lui+addiu` as a base and uses `addu idx,idx,base; lw 0(idx)`; non-split emits `lw rd,sym(rs)` (-> maspsx -> `$at`); numeric addresses hit maspsx's `$at` branch (and operand order `addu $at,rs,$at` differs).
- No /* ... */ marker can bypass maspsx's load rewriting (MASPSX_FLAGS are either maspsx switches without effect on this or forwarded to as).
- Existing MAIN matches whose original has this shape all use `$at` (PsyQ lib, 2.7.2), none use the rd form.
Conclusion: the rd-form functions need an assembler path (GNU as fed the raw macro) that the stock marker set cannot select; do not spend more time on source permutations for this specific pattern.

## Research note: delay-slot fill classes A/B (`lui $at; jal/jr; sw %lo($at)` and `jr ra; addiu sp`) (w9a)

What the pipeline does (verified on StSetRing / _mainasu):
- cc1 2.7.2 (`-mgas` default; `-mmips-as`/`-mno-gas` give identical text) emits stores as one-line macros `sw $5,sym`, leaves `jal`/`j $31` delay slots EMPTY (no `.set noreorder` wrapper) unless the reorg found a 4-byte filler (a 2-insn macro store is never one), and the epilogue as text `lw $31; addu $sp,$sp,N; j $31`. So the original object = that text run through a REORDER assembler: it expands the macro (`lui $at; sw`), moves the last real insn into the slot (the `sw %lo($at)`; `addiu sp` into `jr`), and keeps a filler between `lw $ra` and `jr $ra` (load hazard) -> `lw ra; addiu sp; jr ra; nop`.
- Stock maspsx never reorders: `.ent` -> `.set noreorder` for the whole function, `.set` lines from cc1 are only tracked (`is_reorder`) and DROPPED, every unfilled branch/jump gets an explicit `nop # DEBUG: branch/jump`, `#nop` comment lines are deleted and re-derived. GNU as therefore only ever sees noreorder. No ASPSX_VERSION / MASPSX_FLAGS value changes this (flags not known to maspsx are forwarded to as, which has no reorder switch). Inline `.set reorder` passes maspsx only with a SPACE (tab form is swallowed) but maspsx still adds its own nop -> not a stock recipe, not used.
- cc1 2.7.2 has no `-msplit-addresses` ("Invalid option"), no stock option emits `%hi/%lo` split, so with 2.7.2 the macro-store/fill pair is UNREACHABLE: functions whose codegen is 2.7.2-only (e.g. _mainasu needs `lw $a3,24($sp)`; 2.8.1 gives `move $7,$2`) and all `jr ra`-slot-with-`lw ra` epilogues stay blocked (2.8.1 NEVER emits `lw ra; addiu sp; jr; nop`; it always fills the `j $31` slot, so no 2.8.1 file in src/main has it).

What DOES work (stock): cc1 2.8.1 splits addresses itself (`lui rX,%hi; sw v,%lo(rX)`) and its reorg fills the jal/jr slot with the `sw`. The hi register is normally $2.., but `/* CC1_FLAGS: -fcall-used-$1 */` makes $1 (=$at) allocatable, so cc1 uses `$1` and the result is byte-identical to the assembler's `lui $at / sw %lo($at)` form, delay slot included. maspsx/as accept `$1` as `$at`. Recipe for leaf / simple stores with the original `lui $at,%hi(A); sw; lui $at,%hi(B); jr ra; sw %lo(B)($at)`:
    /* CC1_FLAGS: -fcall-used-$1 */      (default GCC 2.8.1, NOT the 2.7.2 marker)
  plus `-fno-schedule-insns2` when the original keeps `sw $ra` before the first lui (calls). Matched with it: _SsVmDamperOff, SsSetStereo, CD_set_test_parmnum, StSetMask.
Still blocked (checked all failed drafts + scratch drafts with 2.8.1 x {-fcall-used-$1, +-fno-schedule-insns2, +-fno-schedule-insns, -mno-split-addresses, -fno-strength-reduce}): everything that has `lw ra; addiu sp; jr ra; nop` or 2.7.2-only codegen (StSetRing top half matches with `-fcall-used-$1 -fno-schedule-insns2`, only the epilogue `lw; nop; jr; addiu` vs `lw; addiu; jr; nop` differs), _mainasu, _SsTickCallbackDivided, SpuStart, _spu_Fw1ts, _add_mant_d, _err_math, CdRead2, CdCommandCallback, cd_read, _SsContRpn2, _SsClose, _SsContNrpn1.

## KEY LEVER: `/* CC1_FLAGS: -fcall-used-$1 */` (stock cc1 2.8.1)
Makes $at allocatable: cc1 emits `lui $1,%hi(sym); sw v,%lo(sym)($1)` itself and its own scheduler puts the sw in jal/jr delay slots. Matched _SsVmDamperOff, 8008A634, 8008DDA0, 8008F914 (same lever parasite-pc uses for ER_retry). Try also with `-fno-schedule-insns2` / `-fno-expensive-optimizations`. Added to sweep_markers.py defaults.
- (w8c1) `goto` loop instead of do/while is not loop-optimised: nothing in it is hoisted. asm-launder a constant (e.g. 0x7EF8) to stop hoisting out of loop 1. `register asm` pins can alias other live pseudos (pin to $30 clobbered a2 / 0x1F800000 ptr).
- (w8c1) K&R signature + `s0 = 0x40;` inside the `if` body fills a delay slot; separate address variables for two table accesses.
- (w8c2) Declare 3x3 short matrix local as full 32-byte MATRIX struct (`short m[3][3]; short pad; int t[3]`) to fix frame + loop strength reduction; `m.m[0][i]`.. for transposed stores; small structs V3/V4 for array copies.
- (w8c2) `register int *g asm("$4")` placed after the first computation gives `lui a0` scratchpad base. Read base symbol into a local before `i = a0*6`.
- (w9c0) Derive every pointer/Y value from loop counter `i` (`0xEC + i*0x38`, `0x118 + i*0x14`) in a plain `do { } while (i < 2)`: loop.c emits giv inits after hoisted constants (original preheader order). Packed `U4` struct for lwl/lwr/swl/swr copies. A volatile `asm("" : "=r"(x) : "0"(c))` barrier acts as a scheduling barrier and can change the schedule.
- (w9c4) `/* CC1_FLAGS: -fno-schedule-insns */` for functions whose original has unscheduled `lw; nop; sw` runs following source order (sched1 off). Detect such runs, then brute-force statement order. Mask constant `lui`+`ori` kept apart by an empty-asm barrier so sched2 sees separate insns. `-fno-cse-skip-blocks` stops reuse of a global across an `if` join (redundant reload fix). `int b=(int)D; v=(int*)(b+a1*12)` puts base first in addu.
- (w9c7) Inline CPU instructions only via named central macros in include/rr_asm.h (e.g. `RR_NOP_DEP()` for a lone `nop`; used by _cardSelectSlot — maspsx puts the load-delay nop after the loop label, original has it before). No raw `asm("nop")` strings in function bodies.
- (w9c7) CD/SPU module 0x80092xxx-0x8009Cxxx: `-mno-split-addresses`; duplicated call in each branch; `while (A || B)` loops; `char nv = b & 0xFF`.
- (w9c2) 3-element loop reading a stride-4 table with one walking pointer + offsets 0,1,2 in the original: write `T[i*4+k]`/`T[i][k]`, not `*s++`. Empty-asm launder on a pointer inside a loop (`asm volatile("" : "=r"(p) : "0"(p))`) makes cc1 recompute `p + i*2` each iteration (matches `sll v1,t0,1; addu v1,s0,v1`). `"=r"(f) : "r"(e)` form does NOT copy; use `"0"`.
- (w9c2) The original may pass an unused 4th arg (kept-live pointer, e.g. `$a3` loop-end) to a callee: K&R call with extra arg.
- (w9c5) Reusing one variable as both table index and loop counter raises its register priority (fixes regalloc); base-pointer vars (`base = D; p = base + D2;`) fix load order; `b0 + (a0 & 0xFF)` base form; clear loop `do { *pp-- = 0; } while (--i >= 0)`.
- (w9c3) 6-case switch (LoadIntroSceneStep): `case` labels inside nested `if` bodies, `return v;` at end of each case group (plain `if (v==0) return v` folds to `return 0`); separate local per use; sum of two globals in separate statements; needs `/* RODATA addr size */` line.
- (w9c3) Hand-built goto loop for first loop + real `do {} while` for second with induction value (`r = k + (int)tab`) computed at body top; K&R callee declarations (raw-result callee declared `int`).
- (w9c1) 3-level loop as gotos + inner do/while + one pin; separate var (`w2`) for second use of a product so `x/65536` quotient isn't shared; one reused `int s` avoids an `andi` mask; `i != one` / `sel = i == 1` for compare chains; early hoist of a global by reading it first.

## Wave 9 chunk 6 tricks
- K&R params + `register int cN = aN;` copies fix order/registers of stack-arg loads (_SsVmSeKeyOn near-miss).
- `lw rd,%lo(sym)(rd)` form is only reachable via compound inline asm (`lui/addu/lw %lo`); not pure C, so NOT used in src/main (needs owner decision).
- Near-misses: _spu_FiDMA (extra `addu v1,v1,-1`, 2.7.2 reorg), _SsVmSeqKeyOff (back-edge delay slot), SpuFree (lui/lw scheduling).

## Wave 10 tricks
- `/* CC1_FLAGS: -fno-strength-reduce */` removes second pointer register in two-pointer stride loops (DrawSparkHudLinesAndLabels near-miss).
- Dead `char pad[N]` + `__asm__ volatile("" : : "m"(pad[0]))` at END of function reserves stack without instructions; `int pad[2]` adds 8 frame bytes.
- Derived induction variable (`a0 + 0x80 + i*0x80`) places init after loop-hoisted constants (BuildCardHeaderAnim).
- Dead address-of inside a call arg `(*(v = &a4), i)` stops constant hoisting (BuildCardHeaderAnim; found by permuter).
- Goto loop + `char *b2 = D; q = a0*0x2C + (int)b2` (SetLightColorsFromRow); `-fno-schedule-insns` + pins (DecayCarOffset).
- Near-misses: DrawSparkHudLinesAndLabels, 8004BE4C, 8007C944, 80047A50, 800211A4, 80054D50, 80042F58, 80043C14, 8004A080, 80049A60, 800217D0, 80020F20.
- Wave 10 chunk 3: per-arm tail duplication (repeat the call in each arm so crossjump merges); `-fno-cse-skip-blocks` forces global reload across an if join (SelectCourseBgmTrack); `__asm__("" : "=r"(x) : "0"(v))` launder keeps `& ~0x1F` out of andi narrowing, `volatile` variant keeps a constant inside its branch; separate temps per call result.

## Wave 11 tricks
- Temp-var copy: `{ int t = f1(); v = f2(); s0 = t; }` moves the first call's result copy into the second jal's delay slot (PadPollPort).
- Pins + empty-asm launder to fix schedule order (`v2 = b<<10; asm volatile("" : "=r"(v2) : "0"(v2));`) (ApplyRubberBandSpec).
- If-chain as gotos in original block order reproduces layout; `t = D; sp[1] = t<<8` stops lhu narrowing (SubmitRaceHudPackets).
- Separate result var per rand()/call result removes `addu a0,v0,zero` copies; `one = 1` variable gives srav/sllv (SpawnParticlePair).
- Switch + gotos with `/* RODATA addr size */` covers jump tables; `signed char *` for lb; `a1 = a0` in default arm keeps redundant copy (InitMenuScript).
- `if (x) {copy} else {identical copy}` changes loop invariant hoisting; `--p->f >= 0` decrements in loaded reg; `q += 2` biv with -fno-strength-reduce keeps base (UpdateScrollTrack near-miss).
- Pin address-holding pointers breaks `lui/addiu` hoist: pin products/scalars instead.

## Wave 12 tricks
- Rewrite trailing do/while as `for (i=0;i<N;i++)`; use incoming param directly instead of local copy; local names must not collide with register names (s0,s3 rejected by cc1) (ModeSelectMenuStep, 8003AE4C).
- Scope products differently (function-scope x, block-scope y, inline third) to steer the last `mflo` into $v0 (SetLightColorsScaled).
- Launder `col` inside an if-arm to keep a redundant constant reload (DrawRaceTimesScreen); launder after `if (t<0) t=-t` stops slti landing in bgez slot (SoundCalcCarParams).
- Switch tables: case blocks in original order, `case 0: default:` first, shared tails, `break` not early return, `/* RODATA addr size */` (InitRaceCars).
- Second use of a constant in a loop makes loop.c hoist it to an s-reg; plain `for` duplicates the exit test with its own lui.
- Soft-float: `typedef long long D` with 64-bit hex literals avoids .rdata doubles / maspsx `li.d` failure.
- Never pin a register that an incoming parameter occupies.

## Wave 13 tricks
- Large block copy = struct assignment (`typedef struct { int w[200]; } B; *d = *s;`) (SaveCarSnapshots).
- Derive givs from the loop counter in a plain for (`s0 = base + i*0x320`) instead of copying `off = i` (UpdateComCars, InitPlayerCar: derive second walking pointer inside loop from the first).
- Load through a temp to reorder loads (CarPreviewConfirmModeUpdate); separate local per stanza (UpdateSparkEffects).
- Second C name for the same symbol `extern T X2 __asm__("X");` defeats CSE of lui/addiu (UpdateCameraTargetDistance) — documented hack, can be removed if a true source form is found.
- `short c7` declaration for the constant changed hoisting (TitleMenuInit). `k = 2;` right after `i = 0;` hoists li ahead of address loads; dead address-of `*((char*)((*(nv=&p))+0x2A)) = k;` stops folding (UpdateRaceIntroCamera).
- Write through `volatile short *`, read through plain `short *` to get the lh reload after a store.
- Permuter from a near-miss draft reaches 0 more often than hand tuning (LinkErrorModeUpdate, 80056F04, 80047DF8).
- `fp`/`s8` are the same register; function-level GTE macros belong in include/rr_asm.h.

## Wave 14 tricks
- Near-miss prune: remove each inherited pin/launder one at a time and re-run fd.py; old pins often block the match (StartAttractDemo, 80033A74).
- `perm.sh` bases must be comment-free (pycparser rejects /* */, typeof, asm); strip comments before permuting. cc1 segfaults on `struct __attribute__((packed)) V` as printed by the permuter: put the attribute after the body.
- If a delay slot holds `sh` where original has next-arg setup, store inside both `if` arms (DrawDigitString16).
- Compute `t = v >> 7` in the basic block before the following `if` so the sra does not fuse with halfword sign-extension (UpdateAmbienceVoices) — no barrier needed.
- `if (s) {A} else {A}` scaffold with identical arms reproduces quotient copies + `j; nop` (FormatRaceTime).
- Packet header unions `union { unsigned w; struct { char pad[3]; unsigned char len; } b; }` stop global-pointer reloads after stores; typed struct arrays give the original index/base computation order.
- Fresh pointer var for a second base access stops cc1 folding +offset into the symbol; `x = a/18; s = a - x*18` keeps rand %18 temps.

## Wave 15 tricks
- Struct assignment of 16/32-byte structs gives lw t0..t3 / sw t0..t3 block moves; array-indexed loops give the original givs (SetupObjectLightMatrices).
- One table pointer `s1[1..3]` instead of two walking pointers; stack data as `unsigned short v[4]` array; `do { } while (0)` wrapper fixes prologue order (DrawSparkHudLinesAndLabels, found by permuter).
- Merge reused locals (original reuses few variables): rename-merge greedy search cut a large function's diff from 118 to 56 lines (UpdateCarEngine near-miss). One variable for a clamp (`a = x; if (a > 0) a = 100; else a += 80;`).
- Tail duplication: write the store in each arm so cross-jump merges only the tail. `asm("" : "=r"(ten) : "0"(10))` keeps one pseudo for a constant across the function.
- Sibling functions (same GTE loop) share a draft: adapt the matched sibling (FindAxleTrackNode from FindTrackNodeScratch).
- perm.sh: cpp step fails (exit 33) on drafts with RR_* macros; stub macros or strip before permuting.

## Wave 16 tricks
- Remove inherited pins/launders one at a time from sibling drafts; often the pin was the mismatch.
- Placement of `arg1 += 0x14` relative to `p += 0x14` changes register allocation.
- Reuse one variable for register copies instead of declaring new ones.
- `*(t2 = base + t1) = ...` inside a loop forces the original address temp; `CarTable[i] = ComCarsMinus2 + i` for index-derived stores.
- An empty asm in one `else` arm stops cross-jumping of identical `/2` arms (document with ASM_BARRIER).
- `while (w = *r, r += 4, w != 0)` plus a switch reproduces the read-ahead loop shape.
- Index-based loops restore strength reduction; hill-climb statement order.
- Unsigned masked stores instead of bitfields.

## Disabled diagnostic ABI

PAL `DebugPrintStub` is an eight-byte assembly entry that ignores its arguments
and returns 1. Callers pass diagnostic format strings, not callback addresses;
for example overlay662's `801148F0` and `80114900` are `enter SaveBack\n` and
`enter LoadBack\n`. Use `rr/diagnostics.h` for newly recovered callers.
The entry stays in original assembly because both stock GCC versions emit
argument-home stores for a variadic C definition. It is excluded from recovered
C coverage. Do not use a no-argument C definition with argument-bearing calls,
or invent function-pointer parameters for those string addresses.

## Hand-written asm (`hasm`)
`configs/PAL/hasm.txt` lists functions that are certainly hand-written (evidence per line: trapping
add/addi/sub/neg, cop0, syscall, $k0/$k1, stores of $at or $ra to memory outside the frame, BIOS
A0/B0/C0 trampolines, frameless jumps through a register, conditional branches to their own
fall-through (gcc's jump pass deletes those), or an interior label entered from other hand-written code). `gen_config.py` turns them into splat `hasm` segments
(contiguous runs share one segment so cross-function local labels resolve); splat generates their
`.s` into the untracked `asm/` tree, so no assembly is committed. `code_coverage.py` excludes them
from the C-coverage denominator. Do not attempt them; add a function only with hard evidence
(GTE/cop2 use alone is not evidence: C reaches it through inline macros).
Overlay hand-written code goes in `configs/PAL/overlay_hasm.txt` (`<overlay> <addr> <size> <evidence>`):
its original bytes stay in the reconstructed R4.BIN and `code_coverage.py` drops it from the denominator.
