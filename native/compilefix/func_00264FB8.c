/* compilefix: USA 00264FB8, the linked Ghidra text (ghidra_analyzed5_v4) with a lost stack store restored: the sound
 * name object {vtable, char *} was built with the vtable only (strlen of leftover stack data: BOOT_LOG 167).
 * The string "SE_Bubble_LAND" is the one the ARM code stores next to the vtable. */
// MAPFIX-INTEGRATED v1
#include "ghidra_types.h"

extern int func_00150A84(int *param_1,int *param_2,int param_3);
extern int func_00265800(int param_1);
extern int func_004A1B9C(int *param_1,int param_2,uint *param_3);
extern int func_004FB208(int param_1,int param_2,float *param_3,undefined1 param_4,int param_5);
extern int func_0051D2F4(int *param_1,int param_2,int param_3,int param_4,float fparam_0);
extern int func_00521184();
extern int func_006605D4(undefined4 *param_1,undefined1 param_2);
extern int DAT_002651f8;
extern float DAT_002651fc;
extern float DAT_00265200;
extern int DAT_00265204;
extern float DAT_00265208;
extern undefined DAT_0026520c;
extern uint DAT_00265224;
extern float DAT_00265228;
extern int DAT_0026522c;
extern int DAT_00265230;

// USA 00264FB8 <- EU 00264FE0 (Ghidra, non-matching)
// GLOBAL DAT_002651f8 int
// GLOBAL DAT_002651fc float
// GLOBAL DAT_00265200 float
// GLOBAL DAT_00265204 int
// GLOBAL DAT_00265208 float
// GLOBAL DAT_0026520c undefined
// GLOBAL DAT_00265224 uint
// GLOBAL DAT_00265228 float
// GLOBAL DAT_0026522c int
// GLOBAL DAT_00265230 int

/* WARNING: Variable defined which should be unmapped: objStack_28 */

undefined4 func_00264FB8(int *param_1)

{
  int iVar1;
  float fVar2;
  bool bVar3;
  int iVar4;
  uint uVar5;
  byte bVar6;
  int iVar7;
  float *pfVar8;
  uint in_fpscr;
  float fVar9;
  float fVar10;
  _Alignas(8) unsigned char albw_frame[0x30];
  #define objStack_28 ((uint *)(albw_frame + 0x8))
  func_0051D2F4(param_1 + 0x241,param_1[0xfa],DAT_002651f8,0x7fffffff,DAT_002651fc);
  if (*(char *)((int)param_1 + 0x90d) != '\0') {
    *(char *)((int)param_1 + 0x90d) = *(char *)((int)param_1 + 0x90d) + -1;
    return 0;
  }
  param_1[0x23f] = (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0x23f]) + DAT_00265200));
  bVar3 = func_00265800((int)param_1);
  iVar1 = DAT_00265204;
  iVar4 = param_1[0x23f];
  iVar7 = DAT_00265204 + 0x500000;
  if (iVar4 == DAT_00265204 || iVar4 == iVar7) {
    param_1[0x240] = (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0x240]) * DAT_00265208));
  }
  if (bVar3) {
    iVar4 = func_00521184((int)param_1);
    param_1[0xfa] = (iVar4 + 0xc0000000U & 0xc0000000) + 0xe0000000;
  }
  else {
    if ((char)param_1[0x24f] != '\0') {
      param_1[0xfa] = param_1[0x242];
      param_1[0x241] = param_1[0x242];
      *(undefined1 *)(param_1 + 0x24f) = 0;
    }
    if (iVar4 != iVar1 && iVar4 != iVar7) goto LAB_00265144;
  }
  objStack_28[0] = 0;
  iVar4 = func_004FB208((int)param_1,0,(float *)0x0,0,0);
  if (iVar4 != 0) {
    return func_006605D4(param_1 + 0x246,5);
  }
  if (param_1[0xd1] != 0) {
    func_004A1B9C((int *)objStack_28,param_1[0xd1],(uint *)&DAT_0026520c);
  }
  if (param_1[0x23f] == iVar1) {
    objStack_28[0] = DAT_00265224;
    objStack_28[1] = (uint)(uintptr_t)"SE_Bubble_LAND";   /* the name: the converted text lost this store */
    func_00150A84(param_1,(int *)objStack_28,0);
  }
LAB_00265144:
  fVar2 = DAT_00265228;
  uVar5 = param_1[0xfa];
  pfVar8 = (float *)(DAT_0026522c + (uVar5 >> 0x18) * 0x10);
  fVar9 = (float)VectorUnsignedToFloat(uVar5 & 0xffffff,(byte)(in_fpscr >> 0x15) & 3);
  fVar10 = (float)VectorUnsignedToFloat(uVar5 & 0xffffff,(byte)(in_fpscr >> 0x15) & 3);
  param_1[0xf4] =
       (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0xf4]) +
            (*pfVar8 + pfVar8[1] * fVar9 * DAT_00265228) * ALBW_AS_F32(param_1[0x240])));
  param_1[0xf6] =
       (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0xf6]) + (pfVar8[2] + pfVar8[3] * fVar10 * fVar2) * ALBW_AS_F32(param_1[0x240])
            ));
  if (!bVar3) {
    return 0;
  }
  bVar6 = (char)param_1[0x243] + 1;
  *(byte *)(param_1 + 0x243) = bVar6;
  iVar1 = DAT_00265230;
  if (2 < bVar6) {
    return func_006605D4(param_1 + 0x246,4);
  }
  *(undefined1 *)((int)param_1 + 0x90d) = 0x1e;
  param_1[0x240] = iVar1;
  return 0;
}

