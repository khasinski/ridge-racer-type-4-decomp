#ifndef RR_ASM_H
#define RR_ASM_H
/* Central, named inline-instruction macros (policy: no raw asm strings in function bodies; see docs/BARRIERS.md).
 * RR_NOP_DEP(): a single `nop` reproducing a load/store dependency gap the original assembler left (maspsx would place it after a loop label). */
#define RR_NOP_DEP() __asm__ volatile("nop")
/* RR_NOP_DEP_ON(value): keep that pipeline gap after the load producing value. */
#define RR_NOP_DEP_ON(value) __asm__ volatile("nop" : : "r"(value))
/* GTE (COP2) helpers. All use $t4-$t6 ($12-$14) as scratch exactly like the original hand-written PsyQ-style macros. */
/* RR_GTE_SETLCM(p): load the 0x14-byte light-colour matrix block at p into GTE control regs $16-$20 (LCM). */
#define RR_GTE_SETLCM(p) __asm__ volatile ( \
    "lw $12, 0(%0); lw $13, 4(%0); ctc2 $12, $16; ctc2 $13, $17; lw $12, 8(%0); lw $13, 12(%0); lw $14, 16(%0); ctc2 $12, $18; ctc2 $13, $19; ctc2 $14, $20" \
    : : "r"(p) : "$12", "$13", "$14")
/* RR_GTE_SETLLM(p): light matrix block (control regs $8-$12). */
#define RR_GTE_SETLLM(p) __asm__ volatile ( \
    "lw $12, 0(%0); lw $13, 4(%0); ctc2 $12, $8; ctc2 $13, $9; lw $12, 8(%0); lw $13, 12(%0); lw $14, 16(%0); ctc2 $12, $10; ctc2 $13, $11; ctc2 $14, $12" \
    : : "r"(p) : "$12", "$13", "$14")
/* RR_GTE_SETROT(p): rotation matrix block (control regs $0-$4). */
#define RR_GTE_SETROT(p) __asm__ volatile ( \
    "lw $12, 0(%0); lw $13, 4(%0); ctc2 $12, $0; ctc2 $13, $1; lw $12, 8(%0); lw $13, 12(%0); lw $14, 16(%0); ctc2 $12, $2; ctc2 $13, $3; ctc2 $14, $4" \
    : : "r"(p) : "$12", "$13", "$14")
/* RR_GTE_SETTR(p): translation vector (control regs $5-$7) from 3 words at p. */
#define RR_GTE_SETTR(p) __asm__ volatile ( \
    "lw $12, 0(%0); lw $13, 4(%0); lw $14, 8(%0); ctc2 $12, $5; ctc2 $13, $6; ctc2 $14, $7" \
    : : "r"(p) : "$12", "$13", "$14")
/* RR_GTE_SETFC(r,g,b): far colour (control regs $21-$23) from three register values. */
#define RR_GTE_SETFC(r, g, b) __asm__ volatile ( \
    "ctc2 %0, $21; ctc2 %1, $22; ctc2 %2, $23" : : "r"(r), "r"(g), "r"(b))
/* RR_GTE_SETBK(r,g,b): background colour (control regs $13-$15). */
#define RR_GTE_SETBK(r, g, b) __asm__ volatile ( \
    "ctc2 %0, $13; ctc2 %1, $14; ctc2 %2, $15" : : "r"(r), "r"(g), "r"(b))

/* Raw GTE command words and single-matrix-half loader (central, ex w3t0_gte.h). */
#define RR_GTE_SETMAT(p, c0) __asm__ volatile ( \
    "lw $12, 0(%0); lw $13, 4(%0); ctc2 $12, $" #c0 "; ctc2 $13, $" #c0 "+1" : : "r"(p) : "$12", "$13")
#define RR_GTE_OP(w) __asm__ volatile (".word " #w)
#define GRTPS ".word 0x4A180001"
#define GRTPT ".word 0x4A280030"
#define GNCLIP ".word 0x4B400006"
#define GAVSZ3 ".word 0x4B58002D"
#define GAVSZ4 ".word 0x4B68002E"
#define GDPCS ".word 0x4A780010"
#define GDPCT ".word 0x4AF8002A"
#define GINTPL ".word 0x4A980011"
#define GGPF ".word 0x4B98003D"
#define GCC ".word 0x4B38041C"
#define GSQR ".word 0x4AA80428"
#define GNCCS ".word 0x4B08041B"
#define GNCCT ".word 0x4B18043F"
/* RR_GTE_NCLIP3(v12, v13, v14, out): load three packed screen-XY words into SXY0..SXY2 ($12-$14, written $12,$14,$13 like PsyQ gte_ldsxy3), run NCLIP, store MAC0 ($24) to *out. */
#define RR_GTE_NCLIP3(v12, v13, v14, out) do { \
    __asm__ volatile ("mtc2 %0, $12\n\tmtc2 %2, $14\n\tmtc2 %1, $13\n\tnop\n\tnop\n\t.word 0x4B400006" : : "r"(v12), "r"(v13), "r"(v14)); \
    __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(out) : "memory"); } while (0)
/* Single-instruction / fused GTE helpers (DrawEngineRevGlow). */
#define RR_GTE_MTC2_IR0(x) __asm__ volatile ("mtc2 %0, $8" : : "r"(x))
#define RR_GTE_LWC2_RGB(p) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(p))
#define RR_GTE_SWC2_RGB2(p) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(p))
/* RR_GTE_DPCS_FC4(q): far colour = 3 bytes at q, each << 4 (ctc2 $21-$23), then DPCS. */
#define RR_GTE_DPCS_FC4(q) __asm__ volatile ( \
    "lbu $12, 0(%0); lbu $13, 1(%0); lbu $14, 2(%0); sll $12, $12, 4; sll $13, $13, 4; sll $14, $14, 4; ctc2 $12, $21; ctc2 $13, $22; ctc2 $14, $23; nop; nop; .word 0x4A780010" \
    : : "r"(q) : "$12", "$13", "$14")
/* RR_GTE_MVMVA_LDV0(p): load vector V0 (VXY0/VZ0) from 3 shorts at p and run MVMVA sf=1 mx=rot v=V0 cv=none lm=0; RR_GTE_STORE_IR(p): store IR1..IR3 ($9-$11) to p. */
#define RR_GTE_MVMVA_LDV0(p) do { __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0)" : : "r"(p)); __asm__ volatile ("nop; nop"); __asm__ volatile (".word 0x4A080012"); } while (0)
#define RR_GTE_STORE_IR(p) __asm__ volatile ("swc2 $9, 0(%0); swc2 $10, 4(%0); swc2 $11, 8(%0)" : : "r"(p))
/* DrawTrackObjectPointInstances: MVMVA sf=1 mx=LLM v=V0 cv=BK lm=0 on the vector at p (lwc2 $0/$1); RR_GTE_SWC2_IR3(p): swc2 $11; RR_GTE_TR_FROM_IR(): TR = IR1..IR3. */
#define RR_GTE_MVMVA_LLM_BK_LDV0(p) do { __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0)" : : "r"(p)); __asm__ volatile ("nop; nop"); __asm__ volatile (".word 0x4A4A2012"); } while (0)
#define RR_GTE_SWC2_IR3(p) __asm__ volatile ("swc2 $11, 0(%0)" : : "r"(p) : "memory")
#define RR_GTE_TR_FROM_IR() __asm__ volatile ("mfc2 $12, $9; mfc2 $13, $10; mfc2 $14, $11; ctc2 $12, $5; ctc2 $13, $6; ctc2 $14, $7" : : : "$12", "$13", "$14")
/* StepIntroScene: RR_GTE_LWC2_IR(p): load IR1..IR3 ($9-$11) from 3 words at p; RR_GTE_INTPL(): two nops then INTPL. */
#define RR_GTE_LWC2_IR(p) __asm__ volatile ("lwc2 $9, 0(%0); lwc2 $10, 4(%0); lwc2 $11, 8(%0)" : : "r"(p) : "memory")
#define RR_GTE_INTPL() __asm__ volatile ("nop; nop; .word 0x4A980011")
/* StepIntroScene: RR_GTE_SETFC_P(p): far colour from 3 words at p via $t2-$t4; RR_COPY16_T(dst, src, off): copy 16 bytes src+off -> dst via $t2-$t4 (3 loads, 3 stores, 4th word last). */
#define RR_GTE_SETFC_P(p) __asm__ volatile ( \
    "lw $10, 0(%0); lw $11, 4(%0); lw $12, 8(%0); ctc2 $10, $21; ctc2 $11, $22; ctc2 $12, $23" \
    : : "r"(p) : "$10", "$11", "$12")
#define RR_COPY16_T(dst, src, off) __asm__ ( \
    "lw $10, %2(%1); lw $11, %2+4(%1); lw $12, %2+8(%1); sw $10, 0(%0); sw $11, 4(%0); sw $12, 8(%0); lw $10, %2+12(%1); nop; sw $10, 12(%0)" \
    : : "r"(dst), "r"(src), "i"(off) : "$10", "$11", "$12", "memory")
/* DrawTrackObjectBillboards: RR_GTE_SETBK_P(p): background colour (ctc2 $13-$15) from 3 words at p via $t4-$t6; RR_GTE_RTPT_LDV(a,b,c): load V0..V2 from three vectors (lwc2 0..5), two nops, RTPT; RR_GTE_RTPS_LDV(p): load V0, two nops, RTPS; RR_GTE_SWC2_SXYZ(p): store SXY0..SXY2 ($12-$14) to p; RR_GTE_SWC2_SXY2(p): store SXY2 ($14). */
#define RR_GTE_SETBK_P(p) __asm__ volatile ( \
    "lw $12, 0(%0); lw $13, 4(%0); lw $14, 8(%0); ctc2 $12, $13; ctc2 $13, $14; ctc2 $14, $15" \
    : : "r"(p) : "$12", "$13", "$14")
#define RR_GTE_RTPT_LDV(a, b, c) do { __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0); lwc2 $2, 0(%1); lwc2 $3, 4(%1); lwc2 $4, 0(%2); lwc2 $5, 4(%2)" : : "r"(a), "r"(b), "r"(c)); __asm__ volatile ("nop; nop"); __asm__ volatile (".word 0x4A280030"); } while (0)
#define RR_GTE_RTPS_LDV(p) { __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0)" : : "r"(p)); __asm__ volatile ("nop; nop"); __asm__ volatile (".word 0x4A180001"); }
#define RR_GTE_SWC2_SXYZ(p) __asm__ volatile ("swc2 $12, 0(%0); swc2 $13, 4(%0); swc2 $14, 8(%0)" : : "r"(p) : "memory")
#define RR_GTE_SWC2_SXY2(p) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(p) : "memory")
/* DrawTrackScenery: RR_GTE_CLRTR(): TR = 0; RR_GTE_MTC2_IR1(x): mtc2 $9; RR_GTE_CTC2_FCR(x): ctc2 $21; RR_GTE_SWC2_IR1(p): swc2 $9. */
#define RR_GTE_CLRTR() __asm__ volatile ("ctc2 $0, $5; ctc2 $0, $6; ctc2 $0, $7")
#define RR_GTE_MTC2_IR1(x) __asm__ volatile ("mtc2 %0, $9" : : "r"(x))
#define RR_GTE_CTC2_FCR(x) __asm__ volatile ("ctc2 %0, $21" : : "r"(x))
#define RR_GTE_SWC2_IR1(p) __asm__ volatile ("swc2 $9, 0(%0)" : : "r"(p) : "memory")
/* EmitRroLitQuadsG4 GTE helpers (one macro per distinct instruction/sequence). */
#define RR_GTE14_LWC2_V012(p) __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0); lwc2 $2, 8(%0); lwc2 $3, 12(%0); lwc2 $4, 16(%0); lwc2 $5, 20(%0)" : : "r"(p) : "memory")
#define RR_GTE14_LWC2_V0(p) __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0)" : : "r"(p) : "memory")
#define RR_GTE14_LWC2_V0_V1_V2(a, b, c) __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0); lwc2 $2, 0(%1); lwc2 $3, 4(%1); lwc2 $4, 0(%2); lwc2 $5, 4(%2)" : : "r"(a), "r"(b), "r"(c) : "memory")
#define RR_GTE14_LWC2_RGB(p) __asm__ volatile ("lwc2 $6, 0(%0)" : : "r"(p) : "memory")
#define RR_GTE14_NOP2() __asm__ volatile ("nop; nop")
#define RR_GTE14_RTPT() __asm__ volatile (".word 0x4A280030")
#define RR_GTE14_RTPS() __asm__ volatile (".word 0x4A180001")
#define RR_GTE14_NCLIP() __asm__ volatile (".word 0x4B400006")
#define RR_GTE14_AVSZ4() __asm__ volatile (".word 0x4B68002E")
#define RR_GTE14_SWC2_MAC0(p) __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(p) : "memory")
#define RR_GTE14_SWC2_OTZ(p) __asm__ volatile ("swc2 $7, 0(%0)" : : "r"(p) : "memory")
#define RR_GTE14_SWC2_SXYZ(p) __asm__ volatile ("swc2 $12, 0(%0); swc2 $13, 4(%0); swc2 $14, 8(%0)" : : "r"(p) : "memory")
#define RR_GTE14_SWC2_SXY2(p) __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(p) : "memory")
#define RR_GTE14_LIGHT0() __asm__ volatile (".word 0x4A4A6412; .word 0x4AA80428; .word 0x4AA80428; .word 0x4B38041C")
#define RR_GTE14_LIGHT1() __asm__ volatile (".word 0x4A4AE412; .word 0x4AA80428; .word 0x4AA80428; .word 0x4B38041C")
#define RR_GTE14_LIGHT2() __asm__ volatile (".word 0x4A4B6412; .word 0x4AA80428; .word 0x4AA80428; .word 0x4B38041C")
#define RR_GTE14_SWC2_RGB012(p) __asm__ volatile ("swc2 $20, 4(%0); swc2 $21, 12(%0); swc2 $22, 20(%0)" : : "r"(p) : "memory")
#define RR_GTE14_SWC2_RGB2(p) __asm__ volatile ("swc2 $22, 0(%0)" : : "r"(p) : "memory")
/* FindWheelTrackNodes: MVMVA rot-matrix x V0, no translation, shift 12 (sf=1,mx=0,v=0,cv=3,lm=0); preceded by the V0 load from p. */
#define RR_GTE_MVMVA_ROT_V0_NOCV(p) { __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0)" : : "r"(p)); __asm__ volatile ("nop; nop"); __asm__ volatile (".word 0x4A486012"); }

/* SQR: square signed IR1..IR3, with fixed-point shift 12 or no shift.
 * MAC1..MAC3 retain the arithmetic results; IR saturation and FLAG are hardware side effects. */
#define RR_GTE_SQR12() RR_GTE_OP(0x4AA80428)
#define RR_GTE_SQR0() RR_GTE_OP(0x4AA00428)
#define RR_GTE_SWC2_MAC123(p) __asm__ volatile ("swc2 $25, 0(%0); swc2 $26, 4(%0); swc2 $27, 8(%0)" : : "r"(p) : "memory")

/* Memory-clobbering GTE command barriers used by polygon packet builders (DrawMeshQuadGT4..DrawMeshQuadF4). */
#define RR_GTE_RTPT_MEM() __asm__ volatile (".word 0x4A280030" : : : "memory")
#define RR_GTE_RTPS_MEM() __asm__ volatile (".word 0x4A180001" : : : "memory")
#define RR_GTE_NCLIP_MEM() __asm__ volatile (".word 0x4B400006" : : : "memory")
#define RR_GTE_AVSZ4_MEM() __asm__ volatile (".word 0x4B68002E" : : : "memory")

/* SetSkyMatrixAndDraw: column-wise IR1..IR3 load/MVMVA and short store helpers. */
#define RR_GTE_MV3_COL(src) __asm__ volatile ("lhu $12, 0(%0); lhu $13, 6(%0); lhu $14, 12(%0); mtc2 $12, $9; mtc2 $13, $10; mtc2 $14, $11\n\tnop\n\tnop\n\t.word 0x4A49E012" : : "r"(src) : "$12", "$13", "$14")
#define RR_GTE_ST3_COL(dst) __asm__ volatile ("mfc2 $12, $9; mfc2 $13, $10; mfc2 $14, $11; sh $12, 0(%0); sh $13, 6(%0); sh $14, 12(%0)" : : "r"(dst) : "$12", "$13", "$14", "memory")

/* UpdateComCarSpeed: store IR1..IR3 ($9-$11) as contiguous 16-bit components. */
#define RR_GTE_STORE_IR_S16(p) __asm__ volatile ("mfc2 $12, $9\n\tmfc2 $13, $10\n\tmfc2 $14, $11\n\tsh $12, 0(%0)\n\tsh $13, 2(%0)\n\tsh $14, 4(%0)" : : "r"(p) : "$12", "$13", "$14", "memory")

/* SoundCalcCarParams / SetTrackObjectTransform: combined rotation + translation matrix loaders and FLAG reader. */
#define RR_GTE_SETROT_TR(p) __asm__ volatile ("lw $12, 0(%0)\n\tlw $13, 4(%0)\n\tctc2 $12, $0\n\tctc2 $13, $1\n\tlw $12, 8(%0)\n\tlw $13, 12(%0)\n\tlw $14, 16(%0)\n\tctc2 $12, $2\n\tctc2 $13, $3\n\tctc2 $14, $4\n\tlw $12, 20(%0)\n\tlw $13, 24(%0)\n\tctc2 $12, $5\n\tlw $14, 28(%0)\n\tctc2 $13, $6\n\tctc2 $14, $7" : : "r"(p) : "$12", "$13", "$14")
#define RR_GTE_SETTR_AT20(p) __asm__ volatile ("lw $12, 20(%0)\n\tlw $13, 24(%0)\n\tctc2 $12, $5\n\tlw $14, 28(%0)\n\tctc2 $13, $6\n\tctc2 $14, $7" : : "r"(p) : "$12", "$13", "$14")
#define RR_GTE_MVMVA_ROT_TR_V0(p) __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(p)); __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define RR_GTE_STORE_FLAG(p) __asm__ volatile ("cfc2 $12, $31\n\tnop\n\tsw $12, 0(%0)" : : "r"(p) : "$12", "memory")
#define RR_GTE_SWC2_IR0(p) __asm__ volatile ("swc2 $8, 0(%0)" : : "r"(p) : "memory")
#define RR_GTE_STORE_SZ3_SRA2(p) __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(p) : "$12", "memory")

/* PsyQ libgte hand-coded matrix multiply helpers (MulMatrix0..ApplyMatrixSV). */
#define RR_GTE_MASK_HI16(x) __asm__ volatile ("and %0, %0, 0xFFFF0000" : "+r"(x))
#define RR_GTE_MVMVA_ROT_V0_1NOP() __asm__ volatile ("nop\n\t.word 0x4A486012")
#define RR_GTE_SW_AT(val, base, idx) __asm__ volatile ("sw %0, " #idx "*4(%1)" : : "r"(val), "r"(base))
#define RR_GTE_SH_AT(val, base, idx) __asm__ volatile ("sh %0, " #idx "*2(%1)" : : "r"(val), "r"(base))

#endif
