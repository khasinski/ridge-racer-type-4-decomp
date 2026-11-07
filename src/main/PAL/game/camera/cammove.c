#include "scratchpad.h"
#include "rr/camera_motion.h"
#include "rr/cpu_md_leaf.h"
#include "game/camera/cammove.h"
#include "rr/overlay665_render.h"

#define W_ADD(a,b) ((s32)((u32)(a)+(u32)(b)))
#define W_SUB(a,b) ((s32)((u32)(a)-(u32)(b)))
/* Constant products below expand to shifts/adds; variable MD operations use
 * the typed HI/LO interface with an independent native arithmetic reference. */
#define W_MUL(a,b) ((s32)((u32)(a)*(u32)(b)))

void UpdateCameraMotion(RrCameraMotionInput14 *input) {
    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
    register RrCameraMotionInput14 *arg0 __asm__("$9")=input;
    RrCameraRootPrefix *root=&RaceContext;
    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
    register RrCameraObjectPrefix *object __asm__("$8")=root->camera3D8;
    s32 boundary;
    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
    register s32 temp_a0 __asm__("$4");
    s32 temp_lo;
    s32 temp_lo_2;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 temp_v0_7;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_5;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
    register s32 temp_v1_3 __asm__("$3");
    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
    register RrCameraMotion70 *temp_a3 __asm__("$7");
    RrCameraKey16 *temp_v1_4;

    temp_a3 = &object->motion74;
    temp_v1 = temp_a3->mode04;
    switch (temp_v1) {
    case 0:
        temp_v1_2 = object->motion74.phase00;
        switch (temp_v1_2) {
        case 0:
            /* ASM_BARRIER: retain the live full-width input pointer in its original argument-read phase. */
            __asm__("" : "+r"(arg0));
            if (arg0->active00 == 0) {
copy_start:
                temp_a3->current40[0] = (s32) temp_a3->start10[0];
                temp_a3->current40[1] = (s32) temp_a3->start10[1];
                temp_a3->current40[2] = (s32) temp_a3->start10[2];
                return;
            }
            {
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 value __asm__("$2"),start __asm__("$3");
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 first __asm__("$5"),second __asm__("$4"),third __asm__("$2");
                value=arg0->timing10;start=object->scale4C;
                value=W_SUB(W_MUL(value,2),300);RR_MD_MULT_S(value,start);
                {
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register s32 key __asm__("$2")=temp_a3->key08;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *keys __asm__("$3")=temp_a3->keys0C;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *point __asm__("$2")=(RrCameraKey16 *)((unsigned long)key*16+(unsigned long)keys);
                    value=point->x;
                }
                start=temp_a3->start10[0];temp_a3->target20[0]=value;value=W_SUB(value,start);first=W_MUL(value,2);first=W_ADD(first,value);first=W_MUL(first,8);
                RR_MD_LO_ACCUM_SHL2(temp_lo,first,value);
                RR_MD_DIV_S(first,first,temp_lo);
                {
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register s32 key __asm__("$2")=temp_a3->key08;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *keys __asm__("$3")=temp_a3->keys0C;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *point __asm__("$2")=(RrCameraKey16 *)((unsigned long)key*16+(unsigned long)keys);
                    value=point->y;
                }
                start=temp_a3->start10[1];temp_a3->target20[1]=value;value=W_SUB(value,start);second=W_MUL(value,100);
                RR_MD_DIV_S(second,second,temp_lo);
                {
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register s32 key __asm__("$2")=temp_a3->key08;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *keys __asm__("$3")=temp_a3->keys0C;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *point __asm__("$2")=(RrCameraKey16 *)((unsigned long)key*16+(unsigned long)keys);
                    value=point->z;
                }
                /* ASM_BARRIER: retain the original read/publish phase before this physical register is reused. */
                temp_a3->target20[2]=value;__asm__("" : "=r"(value) : "0"(value) : "memory");start=value;value=temp_a3->start10[2];start=W_SUB(start,value);
                third=W_MUL(start,100);
                RR_MD_DIV_S(third,third,temp_lo);
                temp_a3->delta30[0]=first;
                temp_a3->delta30[1]=second;
                temp_a3->delta30[2]=third;
            }
            {
                RrCameraObjectPrefix *settings=root->camera3D8;
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 result __asm__("$2"),product __asm__("$3");
                result=settings->scale4C;product=arg0->timing10;
                result=W_MUL(result,2);RR_MD_MULT_S(product,result);RR_MD_LO(product);
                result=0x51EB851FU;RR_MD_MULT_U(product,result);

                product=(u16)settings->scale4C;
                temp_a3->counter50=0;
                product+=1;

                RR_MD_HI(result);result=(u32)result>>5;
                result=W_SUB(result,product);temp_a3->horizon52=(s16)result;
            }
            object->motion74.phase00 = W_ADD(object->motion74.phase00,1);
        case 1:
            {
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 position __asm__("$2")=temp_a3->current40[0];
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 second __asm__("$3");
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 deltaY __asm__("$5");
                temp_a0=temp_a3->delta30[0];
                second=temp_a3->current40[1];deltaY=temp_a3->delta30[1];

                position=W_ADD(position,temp_a0);temp_a3->current40[0]=position;
                position=temp_a3->current40[2];second=W_ADD(second,deltaY);temp_a3->current40[1]=second;
                second=temp_a3->delta30[2];position=W_ADD(position,second);temp_a3->current40[2]=position;
            }
            if (temp_a0 > 0) {
                /* ASM_BARRIER: retain the captured clamp boundary through comparison and conditional publication. */
                boundary=temp_a3->target20[0];__asm__("" : "+r"(boundary));
                var_v0 = boundary < temp_a3->current40[0];
            } else {
                /* ASM_BARRIER: retain the captured clamp boundary through comparison and conditional publication. */
                boundary=temp_a3->target20[0];__asm__("" : "+r"(boundary));
                var_v0 = temp_a3->current40[0] < boundary;
            }
            if (var_v0) temp_a3->current40[0]=boundary;
            if (temp_a3->delta30[1] > 0) {
                /* ASM_BARRIER: retain the captured clamp boundary through comparison and conditional publication. */
                boundary=temp_a3->target20[1];__asm__("" : "+r"(boundary));
                var_v0_2 = boundary < temp_a3->current40[1];
            } else {
                /* ASM_BARRIER: retain the captured clamp boundary through comparison and conditional publication. */
                boundary=temp_a3->target20[1];__asm__("" : "+r"(boundary));
                var_v0_2 = temp_a3->current40[1] < boundary;
            }
            if (var_v0_2) temp_a3->current40[1]=boundary;
            if (temp_a3->delta30[2] > 0) {
                /* ASM_BARRIER: retain the captured clamp boundary through comparison and conditional publication. */
                boundary=temp_a3->target20[2];__asm__("" : "+r"(boundary));
                var_v0_3 = boundary < temp_a3->current40[2];
            } else {
                /* ASM_BARRIER: retain the captured clamp boundary through comparison and conditional publication. */
                boundary=temp_a3->target20[2];__asm__("" : "+r"(boundary));
                var_v0_3 = temp_a3->current40[2] < boundary;
            }
            if (var_v0_3) temp_a3->current40[2]=boundary;
            {
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 multiplier __asm__("$2")=0x51EB0000;
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 first __asm__("$4"),second __asm__("$5"),third __asm__("$6");
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 highFirst __asm__("$10"),highSecond __asm__("$8"),highThird __asm__("$9");
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 quotient __asm__("$2");
                first=temp_a3->current40[0];RR_MD_MULT_S_FINISH16(first,multiplier,0x851F);RR_MD_HI(highFirst);
                second=temp_a3->current40[1];RR_MD_MULT_S(second,multiplier);
                temp_v1_3=temp_a3->counter50;RR_MD_HI(highSecond);
                third=temp_a3->current40[2];temp_v1_3++;
                RR_MD_MULT_S(third,multiplier);
                first>>=31;temp_a3->counter50=(u16)temp_v1_3;
                /* ASM_BARRIER: retain the signed16 counter preparation before angle-result publication. */
                __asm__("" : "=r"(temp_v1_3) : "0"((s32)((u32)temp_v1_3<<16)) : "memory");second>>=31;
                quotient=highFirst>>5;quotient=W_SUB(quotient,first);temp_a3->angles64[0]=(s16)quotient;
                quotient=highSecond>>5;quotient=W_SUB(quotient,second);third>>=31;temp_a3->angles64[1]=(s16)quotient;
                RR_MD_HI(highThird);quotient=highThird>>5;quotient=W_SUB(quotient,third);temp_a3->angles64[2]=(s16)quotient;
            }
            if ((temp_v1_3>>16) >= temp_a3->horizon52) {
                temp_v0_4 = W_ADD(temp_a3->key08,1);
                temp_a3->key08 = temp_v0_4;
                if (temp_v0_4 >= 3) {
                    temp_v1_4 = temp_a3->keys0C;
                    temp_v0_5 = ((volatile RrCameraMotion70 *)temp_a3)->keys0C->x;
                    temp_a3->current40[0] = temp_v0_5;
                    temp_a3->start10[0] = temp_v0_5;
                    temp_v0_6 = temp_v1_4->y;
                    temp_a3->current40[1] = temp_v0_6;
                    temp_a3->start10[1] = temp_v0_6;
                    temp_v1_5 = temp_v1_4->z;
                    temp_a3->key08 = 1;
                    temp_a3->current40[2] = temp_v1_5;
                    temp_a3->start10[2] = temp_v1_5;
                } else {
                    temp_a3->start10[0] = (s32) temp_a3->target20[0];
                    temp_a3->start10[1] = (s32) temp_a3->target20[1];
                    temp_a3->start10[2] = (s32) temp_a3->target20[2];
                }
                temp_a3->counter50 = 0U;
                temp_a3->phase00 = 0;
                return;
            }
            return;
        }
        break;
    case 1:
        temp_v0_7 = object->motion74.phase00;
        if (temp_v0_7 != 0) {
            if (temp_v0_7 != temp_v1) {
                /* ASM_BARRIER: keep the distinct mode1 return path at its original control-flow boundary. */
                __asm__ volatile("");
                return;
            }
            goto block_34;
        }
        /* ASM_BARRIER: retain the live full-width input pointer in its original argument-read phase. */
        __asm__("" : "+r"(arg0));
        if (arg0->active00 != 0) {
            {
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 value __asm__("$2"),start __asm__("$3");
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 first __asm__("$5"),second __asm__("$4"),third __asm__("$2");
                value=arg0->timing10;start=object->scale4C;
                value=W_SUB(W_MUL(value,2),300);RR_MD_MULT_S(value,start);
                {
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register s32 key __asm__("$2")=temp_a3->key08;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *keys __asm__("$3")=temp_a3->keys0C;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *point __asm__("$2")=(RrCameraKey16 *)((unsigned long)key*16+(unsigned long)keys);
                    value=point->x;
                }
                start=temp_a3->start10[0];temp_a3->target20[0]=value;value=W_SUB(value,start);first=W_MUL(value,2);first=W_ADD(first,value);first=W_MUL(first,8);
                RR_MD_LO_ACCUM_SHL2(temp_lo_2,first,value);
                RR_MD_DIV_S(first,first,temp_lo_2);
                {
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register s32 key __asm__("$2")=temp_a3->key08;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *keys __asm__("$3")=temp_a3->keys0C;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *point __asm__("$2")=(RrCameraKey16 *)((unsigned long)key*16+(unsigned long)keys);
                    value=point->y;
                }
                start=temp_a3->start10[1];temp_a3->target20[1]=value;value=W_SUB(value,start);second=W_MUL(value,100);
                RR_MD_DIV_S(second,second,temp_lo_2);
                {
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register s32 key __asm__("$2")=temp_a3->key08;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *keys __asm__("$3")=temp_a3->keys0C;
                    /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                    register RrCameraKey16 *point __asm__("$2")=(RrCameraKey16 *)((unsigned long)key*16+(unsigned long)keys);
                    value=point->z;
                }
                /* ASM_BARRIER: retain the original read/publish phase before this physical register is reused. */
                temp_a3->target20[2]=value;__asm__("" : "=r"(value) : "0"(value) : "memory");start=value;value=temp_a3->start10[2];start=W_SUB(start,value);
                third=W_MUL(start,100);
                RR_MD_DIV_S(third,third,temp_lo_2);
                temp_a3->delta30[0]=first;
                temp_a3->delta30[1]=second;
                temp_a3->delta30[2]=third;
            }
            object->motion74.phase00 = W_ADD(object->motion74.phase00,1);
block_34:
            {
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 value __asm__("$2"),delta __asm__("$3");
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 first __asm__("$5"),second __asm__("$4"),third __asm__("$3");
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 highFirst __asm__("$9"),highSecond __asm__("$8"),highThird __asm__("$6");
                /* ASM_BARRIER: retain the initialized value or pointer in the original register phase. */
                register s32 multiplier __asm__("$6")=0x51EB851F;
                /* ASM_BARRIER: retain the original read/publish phase before this physical register is reused. */
                value=temp_a3->current40[0];delta=temp_a3->delta30[0];value=W_ADD(value,delta);temp_a3->current40[0]=value;__asm__("" : "=r"(value) : "0"(value) : "memory");first=value;
                RR_MD_MULT_S(first,multiplier);
                /* ASM_BARRIER: retain the original read/publish phase before this physical register is reused. */
                value=temp_a3->current40[1];delta=temp_a3->delta30[1];value=W_ADD(value,delta);RR_MD_HI(highFirst);temp_a3->current40[1]=value;__asm__("" : "=r"(value) : "0"(value) : "memory");second=value;
                RR_MD_MULT_S(second,multiplier);
                /* ASM_BARRIER: retain the original read/publish phase before this physical register is reused. */
                value=temp_a3->current40[2];delta=temp_a3->delta30[2];value=W_ADD(value,delta);RR_MD_HI(highSecond);temp_a3->current40[2]=value;__asm__("" : "=r"(value) : "0"(value) : "memory");third=value;
                RR_MD_MULT_S(third,multiplier);
                first>>=31;value=highFirst>>5;value=W_SUB(value,first);second>>=31;temp_a3->angles64[0]=(s16)value;
                value=highSecond>>5;value=W_SUB(value,second);third>>=31;temp_a3->angles64[1]=(s16)value;
                RR_MD_HI(highThird);value=highThird>>5;value=W_SUB(value,third);temp_a3->angles64[2]=(s16)value;
            }
        } else {
            goto copy_start;
        }
        break;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0x3D8]; char *p; } (*))&RaceContext)

void DrawCameraMotionModel(void)
{
    char sp10[32];
    char sp30[32];
    int sp50[3];
    int v;

    SetupObjectLightMatrices(RaceContext.p + 0xC8, RaceContext.p + 0x20, sp10, sp30);
    v = *(int *)(RaceContext.p + 0xE0);
    sp50[2] = v;
    sp50[1] = v;
    sp50[0] = v;
    ScaleMatrix(sp10, sp50);
    SelectModelBank(7);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), RaceContext.p + 0xC8, sp10);
    *(int *)(SCRATCHPAD + 0x78) = 0;
    SubmitModelTwoTri(SCRATCHPAD, ModelBankFormat < 1, 0, 0x100);
}

#undef RaceContext
