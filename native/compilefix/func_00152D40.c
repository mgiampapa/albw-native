/* compilefix: USA 00152D40, the linked Ghidra text (ghidra_analyzed5_v4) with a lost stack store restored: the sound
 * name object {vtable, char *} was built with the vtable only (strlen of leftover stack data: BOOT_LOG 167).
 * The string "SE_Bubble_LAND" is the one the ARM code stores next to the vtable. */
// MAPFIX-INTEGRATED v1
#include "ghidra_types.h"

extern int func_00150A84(int *param_1,int *param_2,int param_3);
extern int func_001535E4(int param_1);
extern int func_004A1B9C(int *param_1,int param_2,uint *param_3);
extern int func_004FB208(int param_1,int param_2,float *param_3,undefined1 param_4,int param_5);
extern int func_0051D2F4(int *param_1,int param_2,int param_3,int param_4,float fparam_0);
extern int func_00521184();
extern int func_00657BD4(undefined4 *param_1,undefined1 param_2);
extern int DAT_00152f50;
extern float DAT_00152f54;
extern float DAT_00152f58;
extern int DAT_00152f5c;
extern float DAT_00152f60;
extern undefined DAT_00152f64;
extern uint DAT_00152f7c;
extern float DAT_00152f80;
extern int DAT_00152f84;
extern int DAT_00152f88;

// USA 00152D40 <- EU 00152D44 (Ghidra, non-matching)
// GLOBAL DAT_00152f50 int
// GLOBAL DAT_00152f54 float
// GLOBAL DAT_00152f58 float
// GLOBAL DAT_00152f5c int
// GLOBAL DAT_00152f60 float
// GLOBAL DAT_00152f64 undefined
// GLOBAL DAT_00152f7c uint
// GLOBAL DAT_00152f80 float
// GLOBAL DAT_00152f84 int
// GLOBAL DAT_00152f88 int

/* WARNING: Variable defined which should be unmapped: objStack_28 */

undefined4 func_00152D40(int *param_1)

{
  int iVar1;
  float fVar2;
  bool bVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  float *pfVar7;
  uint in_fpscr;
  float fVar8;
  float fVar9;
  _Alignas(8) unsigned char albw_frame[0x30];
  #define objStack_28 ((uint *)(albw_frame + 0x8))
  func_0051D2F4(param_1 + 0x23d,param_1[0xfa],DAT_00152f50,0x7fffffff,DAT_00152f54);
  if (param_1[0x240] != 0) {
    param_1[0x240] = param_1[0x240] + -1;
    return 0;
  }
  param_1[0x23b] = (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0x23b]) + DAT_00152f58));
  bVar3 = func_001535E4((int)param_1);
  iVar1 = DAT_00152f5c;
  iVar4 = param_1[0x23b];
  iVar6 = DAT_00152f5c + 0x500000;
  if (iVar4 == DAT_00152f5c || iVar4 == iVar6) {
    param_1[0x23c] = (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0x23c]) * DAT_00152f60));
  }
  if (bVar3) {
    iVar4 = func_00521184((int)param_1);
    param_1[0xfa] = (iVar4 + 0xc0000000U & 0xc0000000) + 0xe0000000;
  }
  else {
    if ((char)param_1[0x24c] != '\0') {
      param_1[0xfa] = param_1[0x23e];
      param_1[0x23d] = param_1[0x23e];
      *(undefined1 *)(param_1 + 0x24c) = 0;
    }
    if (iVar4 != iVar1 && iVar4 != iVar6) goto LAB_00152ea0;
  }
  objStack_28[0] = 0;
  iVar4 = func_004FB208((int)param_1,0,(float *)0x0,0,0);
  if (iVar4 != 0) {
    return func_00657BD4(param_1 + 0x243,6);
  }
  if (param_1[0xd1] != 0) {
    func_004A1B9C((int *)objStack_28,param_1[0xd1],(uint *)&DAT_00152f64);
  }
  if (param_1[0x23b] == iVar1) {
    objStack_28[0] = DAT_00152f7c;
    objStack_28[1] = (uint)(uintptr_t)"SE_Bubble_LAND";   /* the name: the converted text lost this store */
    func_00150A84(param_1,(int *)objStack_28,0);
  }
LAB_00152ea0:
  fVar2 = DAT_00152f80;
  uVar5 = param_1[0xfa];
  pfVar7 = (float *)(DAT_00152f84 + (uVar5 >> 0x18) * 0x10);
  fVar8 = (float)VectorUnsignedToFloat(uVar5 & 0xffffff,(byte)(in_fpscr >> 0x15) & 3);
  fVar9 = (float)VectorUnsignedToFloat(uVar5 & 0xffffff,(byte)(in_fpscr >> 0x15) & 3);
  param_1[0xf4] =
       (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0xf4]) +
            (*pfVar7 + pfVar7[1] * fVar8 * DAT_00152f80) * ALBW_AS_F32(param_1[0x23c])));
  param_1[0xf6] =
       (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0xf6]) + (pfVar7[2] + pfVar7[3] * fVar9 * fVar2) * ALBW_AS_F32(param_1[0x23c])))
  ;
  if (!bVar3) {
    return 0;
  }
  iVar4 = param_1[0x23f];
  param_1[0x23f] = iVar4 + 1U;
  iVar1 = DAT_00152f88;
  if (2 < iVar4 + 1U) {
    return func_00657BD4(param_1 + 0x243,4);
  }
  param_1[0x240] = 0x1e;
  param_1[0x23c] = iVar1;
  return 0;
}

