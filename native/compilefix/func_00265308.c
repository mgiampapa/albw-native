/* compilefix: USA 00265308, the linked Ghidra text (ghidra_analyzed5_v4) with a lost stack store restored: the sound
 * name object {vtable, char *} was built with the vtable only (strlen of leftover stack data: BOOT_LOG 167).
 * The string "SE_Bubble_DOWN" is the one the ARM code stores next to the vtable. */
#include "ghidra_types.h"

extern int func_00150A84(int *param_1,int *param_2,int param_3);
extern float func_00265734(float fparam_0,float fparam_1,float fparam_2,float fparam_3,float fparam_4);
extern float func_0026579C(float fparam_0,float fparam_1,float fparam_2,float fparam_3,float fparam_4);
extern int func_004FB208(int param_1,int param_2,float *param_3,undefined1 param_4,int param_5);
extern int func_006605D4(undefined4 *param_1,undefined1 param_2);
extern uint DAT_00265458;
extern float DAT_0026545c;
extern float DAT_00265460;
extern int DAT_00265464;
extern float DAT_00265468;
extern float DAT_0026546c;
extern float DAT_00265470;
extern int DAT_00265474;
extern float DAT_00265478;

// USA 00265308 <- EU 00265330 (Ghidra, non-matching)
// GLOBAL DAT_00265458 uint
// GLOBAL DAT_0026545c float
// GLOBAL DAT_00265460 float
// GLOBAL DAT_00265464 int
// GLOBAL DAT_00265468 float
// GLOBAL DAT_0026546c float
// GLOBAL DAT_00265470 float
// GLOBAL DAT_00265474 int
// GLOBAL DAT_00265478 float

/* WARNING: Variable defined which should be unmapped: objStack_10 */

undefined4 func_00265308(int *param_1)

{
  float fparam_3;
  float fparam_0;
  int iVar1;
  float fparam_1;
  float fparam_2;
  _Alignas(8) unsigned char albw_frame[0x10];
  #define objStack_10 ((uint *)(albw_frame + 0x0))
  if (*(byte *)(param_1 + 0x243) < 0x3c) {
    *(byte *)(param_1 + 0x243) = *(byte *)(param_1 + 0x243) + 1;
    fparam_0 = DAT_0026546c;
    fparam_3 = DAT_00265468;
    fparam_1 = DAT_00265460;
    fparam_2 = ALBW_AS_F32(param_1[0x23f]) + DAT_0026545c;
    param_1[0x23f] = (int)ALBW_AS_U32(fparam_2);
    if (DAT_00265464 < (int)ALBW_AS_U32(fparam_2)) {
      if ((int)ALBW_AS_U32(fparam_2) <= DAT_00265474) {
        fparam_1 = func_00265734(fparam_0,fparam_1,fparam_2,fparam_3,DAT_00265478);
      }
    }
    else {
      fparam_1 = func_0026579C(fparam_0,fparam_1,fparam_2,DAT_00265470,fparam_3);
    }
    param_1[0x15d] = (int)ALBW_AS_U32(fparam_1);
    return 0;
  }
  objStack_10[0] = 0;
  iVar1 = func_004FB208((int)param_1,0,(float *)0x0,0,0);
  if (iVar1 != 0) {
    return func_006605D4(param_1 + 0x246,5);
  }
  func_006605D4(param_1 + 0x246,2);
  objStack_10[0] = DAT_00265458;
    objStack_10[1] = (uint)(uintptr_t)"SE_Bubble_DOWN";   /* the name: the converted text lost this store */
  return func_00150A84(param_1,(int *)objStack_10,0);
}

