/* compilefix: USA 00153094, the linked Ghidra text (ghidra_analyzed5_v4) with a lost stack store restored: the sound
 * name object {vtable, char *} was built with the vtable only (strlen of leftover stack data: BOOT_LOG 167).
 * The string "SE_Bubble_DOWN" is the one the ARM code stores next to the vtable. */
#include "ghidra_types.h"

extern int func_00150A84(int *param_1,int *param_2,int param_3);
extern float func_001534C0(float fparam_0,float fparam_1,float fparam_2,float fparam_3,float fparam_4);
extern float func_00153528(float fparam_0,float fparam_1,float fparam_2,float fparam_3,float fparam_4);
extern int func_004FB208(int param_1,int param_2,float *param_3,undefined1 param_4,int param_5);
extern int func_00657BD4(undefined4 *param_1,undefined1 param_2);
extern uint DAT_001531c0;
extern float DAT_001531c4;
extern float DAT_001531c8;
extern int DAT_001531cc;
extern float DAT_001531d0;
extern float DAT_001531d4;
extern float DAT_001531d8;
extern int DAT_001531dc;
extern float DAT_001531e0;

// USA 00153094 <- EU 00153098 (Ghidra, non-matching)
// GLOBAL DAT_001531c0 uint
// GLOBAL DAT_001531c4 float
// GLOBAL DAT_001531c8 float
// GLOBAL DAT_001531cc int
// GLOBAL DAT_001531d0 float
// GLOBAL DAT_001531d4 float
// GLOBAL DAT_001531d8 float
// GLOBAL DAT_001531dc int
// GLOBAL DAT_001531e0 float

/* WARNING: Variable defined which should be unmapped: objStack_10 */

undefined4 func_00153094(int *param_1)

{
  float fparam_3;
  float fparam_0;
  int iVar1;
  float fparam_1;
  float fparam_2;
  _Alignas(8) unsigned char albw_frame[0x10];
  #define objStack_10 ((uint *)(albw_frame + 0x0))
  if ((uint)param_1[0x23f] < 0x3c) {
    param_1[0x23f] = param_1[0x23f] + 1;
    fparam_0 = DAT_001531d4;
    fparam_3 = DAT_001531d0;
    fparam_1 = DAT_001531c8;
    fparam_2 = ALBW_AS_F32(param_1[0x23b]) + DAT_001531c4;
    param_1[0x23b] = (int)ALBW_AS_U32(fparam_2);
    if (DAT_001531cc < (int)ALBW_AS_U32(fparam_2)) {
      if ((int)ALBW_AS_U32(fparam_2) <= DAT_001531dc) {
        fparam_1 = func_001534C0(fparam_0,fparam_1,fparam_2,fparam_3,DAT_001531e0);
      }
    }
    else {
      fparam_1 = func_00153528(fparam_0,fparam_1,fparam_2,DAT_001531d8,fparam_3);
    }
    param_1[0x15d] = (int)ALBW_AS_U32(fparam_1);
    return 0;
  }
  objStack_10[0] = 0;
  iVar1 = func_004FB208((int)param_1,0,(float *)0x0,0,0);
  if (iVar1 != 0) {
    return func_00657BD4(param_1 + 0x243,6);
  }
  func_00657BD4(param_1 + 0x243,2);
  objStack_10[0] = DAT_001531c0;
    objStack_10[1] = (uint)(uintptr_t)"SE_Bubble_DOWN";   /* the name: the converted text lost this store */
  return func_00150A84(param_1,(int *)objStack_10,0);
}

