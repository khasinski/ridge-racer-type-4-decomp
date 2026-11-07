#ifndef RR_OVERLAY662_INPUT_H
#define RR_OVERLAY662_INPUT_H
#include "rr/overlay662_garage.h"
#include "rr/tables.h"
#include "rr/memory.h"
extern RrMenuItemState5C PlayerControllers[];
extern s16 garage_D_80126C14[2];
/* Analog movement with per-axis fractional accumulation outside112..144.
 * Vertical movement is doubled before its contribution is subtracted from
 * the accumulator. Neutral axes clear their accumulator. */
void garage_func_8011D338(RrGaragePoint *output);
extern u32 garage_D_80126C18, garage_D_80126C1C;
extern u32 garage_D_80126C20, garage_D_80126C24;
extern u32 garage_D_80126C28, garage_D_80126C2C;
extern u32 garage_D_80126C30, garage_D_80126C34;
/* Digital movement: Mask0x8 selects the faster step; later opposite directions win. */
void garage_func_8011D664(RrGaragePoint *output);
/* Direction repeat events at counter1 and40; values47 and above wrap to40.
 * Active directions write the accumulated mask even when no event fires;
 * neutral directions reset their counter and preserve the caller's output. */
void garage_func_8011D730(u32 *output);
void garage_func_8011D8E4(u32 *output);
#endif
