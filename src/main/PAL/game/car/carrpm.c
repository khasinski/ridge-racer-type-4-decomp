#include "rr/vehicle_rpm.h"
#include "rr/cpu_md_leaf.h"
#include "rr_asm.h"
void UpdateEngineRpm(RrRaceVehicle *vehicle) {
    /* ASM_BARRIER: retain actual values and pointers in their original register phases. */
    register RrDriveVehicleView *body __asm__("$11")=(RrDriveVehicleView *)vehicle;
    /* ASM_BARRIER: retain actual values and pointers in their original register phases. */
    register RrDriveControlView *control __asm__("$6")=&body->control1F4;
    /* ASM_BARRIER: retain actual values and pointers in their original register phases. */
    register s32 a0 __asm__("$4"),a1 __asm__("$5"),a3 __asm__("$7");
    /* ASM_BARRIER: retain actual values and pointers in their original register phases. */
    register s32 t0 __asm__("$8"),t2 __asm__("$10");
    /* ASM_BARRIER: retain actual values and pointers in their original register phases. */
    register u16 t1 __asm__("$9");
    /* ASM_BARRIER: retain actual values and pointers in their original register phases. */
    register s32 v0 __asm__("$2"),v1 __asm__("$3");
    a0=control->gear68;
    /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
    __asm__("" : "+r"(a0));
    t2=0;
    if(a0==0) {
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" : "+r"(t2));
        t0=t2;t1=8500;a1=control->shiftThresholds10C[0];a3=8500;
    } else {
        t0=control->shiftThresholds10C[a0-1];
        v1=(s32)((u32)control->gearCountFC-1U);t1=2500;
        if(a0<v1) {v0=control->shiftThresholds10C[a0];a3=8500;
            /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
            __asm__("" :: "r"(a0),"r"(v0));
            a1=v0-t0;}
        else {
            t1=4240;v0=control->maxSpeedF8;a3=10240;
            if(v0<0)v0+=255;
            a1=(v0>>8)-t0;t2=1;
        }
    }
    /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
    __asm__("" :: "r"(a1));
    v1=control->holdFlag48;a1&=65535;
    if(v1==0) {
        v0=body->input1D8;v1=v0-t0;
        if(a1==0)goto minimum;
        v0=t1&65535;
        RR_MD_MULT_S(v0,v1);RR_MD_LO(v1);
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" :: "r"(v1));
        /* ASM_BARRIER: retain both original HI/LO read-to-DIV pipeline slots. */
        RR_NOP_DEP_ON(v1);RR_NOP_DEP_ON(v1);
        RR_MD_DIV_S(v1,v1,a1);
        v0=a3-v0;a1=v1+v0;
        if(t2==0)goto ceiling;
        if((u16)control->accelerating54==0)goto ceiling;
        {
            /* ASM_BARRIER: retain actual values and pointers in their original register phases. */
            register s32 high __asm__("$12");
            v1=0x51EB851F;a0=control->rpm56;
            a1-=a0;v0=(s32)((u32)a1*24U);RR_MD_MULT_S(v0,v1);
            v0>>=31;RR_MD_HI(high);v1=(high>>3)-v0;
        }
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" :: "r"(v1) : "memory");
        v0=control->steering8C;
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" : "+r"(v1) : "r"(v0));
        a1=v1+a0;
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" :: "r"(a1));
        if(v0<0)v0=-v0;
        v0=(s32)((u32)v0<<6);a1-=v0;goto ceiling;
    }
    v0=control->shiftState64;
    if(v0>0) {
        v0=v1<12;
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" :: "r"(v0));
        if(!v0) {
            v1=1;v0=control->rpm56;a1=v0+300;
            v0=a1<10241;control->accelerating54=v1;
            if(v0)goto floor;
            a1=10240;goto ceiling;
        }
        v0=control->rpm56;a1=v0-500;
        v0=a1<6000;
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" :: "r"(v0));
        if(v0) {
minimum:
            a1=6000;goto ceiling;
        }
        v0=a1<10241;
        goto check_ceiling;
    }
    a0=control->rpm56;v1=control->shiftDelay66;v0=1;
    control->accelerating54=v0;a1=a0+v1;
ceiling:
    v0=a1<10241;
check_ceiling:
    if(!v0)a1=10240;
floor:
    if(a1<0)a1=0;
    a0=control->maxSpeedF8;v0=a0;
    if(a0<0)v0=a0+255;
    v1=v0>>8;
    if(v1>=2191)v1=2190;
    if(a1>=4001) {
        a1-=4000;RR_MD_MULT_S(a1,v1);RR_MD_LO(v1);
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" :: "r"(v1));
        v0=(s32)0xEF66A241U;RR_MD_MULT_S(v1,v0);RR_MD_HI(v0);
        v0=(s32)((u32)v0+(u32)v1);v0>>=11;v1>>=31;a1=v0-v1;a1+=4000;
    }
    if(RaceType==6 && CourseListIndex==3) {
        v1=a0;if(v1<0)v1+=255;
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        a3=8500;__asm__("" : "+r"(a3));
        v0=body->input1D8;RR_MD_MULT_S(v0,a3);RR_MD_LO(a3);
        a1=(u32)v1>>8;v0=a1&65535;RR_MD_DIV_S(a1,a3,v0);
    }
    if(control->latch60==0) {
        v1=control->rpm56;
        v0=v1<a1;
        /* ASM_BARRIER: retain the original value/read phase before register reuse or branch selection. */
        __asm__("" :: "r"(v0));
        if(v0){v0=1;control->latch60=v0;return;}
        v0=v1-20;control->rpm56=(u16)v0;return;
    }
    control->rpm56=(u16)a1;
}
