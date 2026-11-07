# Wave 8 brief — NEW WORKING ASSUMPTION

Read docs/WAVE6_BRIEF.md and docs/DECOMP_GUIDE.md first (BARRIER RULE + stock-tools rule apply), then this.

**Assumption (decided by the project owner): EVERY function is reproducible with the STOCK tools.** A closed-source decomp of this game already exists, built on stock old-gcc (2.7.2 / 2.8.1 cc1, same binaries as in tools/toolchain — shas match parasite-pc/tools/old-gcc*) and stock maspsx. So NEVER conclude "unreachable / needs assembler fill / impossible with stock tools". If a draft differs only in delay slots, `lw rd,sym(rs)` expansion (`lui rd; addu; lw`), nop placement, `jr ra; addiu sp`, frame size, etc., then the C source or the stock option choice is wrong — keep searching.

Levers that are allowed and untried at scale (all stock; cc.sh just forwards them, no tool is modified):
- `/* GCC_VERSION: 2.7.2 */` vs default 2.8.1 (and which one the *whole neighbourhood* of functions used — a library module is one translation unit, so neighbours share compiler + flags + maspsx version).
- `/* CC1_FLAGS: ... */` any stock cc1 option: -mno-split-addresses, -fno-strength-reduce, -fcall-used-$1, -fno-expensive-optimizations, -fno-schedule-insns(2), -fno-peephole, -fno-delayed-branch, -O1/-O3, -mgpopt/-G values, -fno-defer-pop, -fomit-frame-pointer ... (test the option exists: run cc1 directly).
- `/* ASPSX_VERSION: x.y */` (2.21 … 2.79; changes nop/hazard handling, `li`/`la` expansion, `$at` use) and `/* MASPSX_FLAGS: --dont-expand-li ... */` (see tools/maspsx/maspsx.py args, config_for_aspsx_version).
- Source-level: how the original author wrote it (macro idioms, K&R vs ANSI params, volatile, struct vs array access, `register`, inline functions, static helpers merged into one TU, different statement forms for the same semantics).
- `tools/scripts/sweep_markers.py <draft.c>...` tries GCC x CC1 flags x ASPSX versions x MASPSX flags on a draft and prints MATCH variants (SWEEP_CC1 env = ';'-separated cc1 flag sets). Use it on EVERY near-miss and on every function whose previous blocker was "assembler fill".
- Think in modules: functions in the same PsyQ object (consecutive addresses) were compiled together with identical settings; once one function of a module matches with settings S, try S on all its siblings.
- Previously "blocked" lists to revisit first: CdSearchFile, _padInitSioMode, _padTransferPacket, _SsContDamper, _SsInitSoundSeq, PutDispEnv, _padStartCom, _SsSetNrpnVabAttr7, _SsSetNrpnVabAttr9, CdRead2, CdCommandCallback, cd_read, SpuStart, _SsTickCallbackDivided, LoadClut, _SsContRpn2, restartIntr, DrawTextStub-like empty functions, plus everything in src/failed/PAL and scratchpad w3*/w4*/w5*/w6*/w7* drafts.

Rules unchanged: unique file stems, only MATCH files in src/main/PAL (fd.py prints literal MATCH), every barrier/pin needs `/* ASM_BARRIER: reason */`, never pkill, no stdin-reading shell commands, don't run make/verify.py/fd_all.py. Final reply: SHORT report — matched names, which setting/lever solved which class, what is still open.

## Wave 9 addendum
- Proven stock levers since this brief was written: `/* CC1_FLAGS: -fcall-used-$1 */` (frees $at: cc1 emits `lui $1,%hi(sym); sw` and schedules it into jal/jr delay slots), `-fno-strength-reduce`, `-mno-split-addresses`, K&R params + `register int x = a;` copies for saved-reg order, `asm volatile("":::"memory")` for empty frame functions, goto-loops (not loop-optimised), bitfield GPU packet headers. See the end of docs/DECOMP_GUIDE.md.
- `tools/scripts/sweep_markers.py` now defaults to ~19 cc1 flag sets; use SWEEP_VERS=2.79 SWEEP_MFL= to shorten.
- Known stock-tools findings: raw `lw rd,sym(rs)` -> `lui rd; addu rd,rd,rs` is produced by GNU as only when maspsx does not rewrite it; maspsx always uses $at, cc1 never emits the split form — still treat as solvable: look for source/flag shapes not yet tried (e.g. pointer-to-array typedefs, `-msplit-addresses` interplay, inline functions merged across a TU) rather than concluding impossibility; report concrete evidence.
