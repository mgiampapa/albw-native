/* compilefix: USA 0012BC9C, the linked Ghidra text (ghidra_analyzed5_v4) with a lost stack store restored: the sound
 * name object {vtable, char *} was built with the vtable only (strlen of leftover stack data: BOOT_LOG 167).
 * The string "SE_Pamet_LAND_SHELL" is the one the ARM code stores next to the vtable. */
#include "ghidra_types.h"

extern int func_00150A84(int *param_1,int *param_2,int param_3);
extern int func_004FB064(int param_1);
extern int func_004FB208(int param_1,int param_2,float *param_3,undefined1 param_4,int param_5);
extern int func_00521078(int param_1);
extern int func_0058A8F0(int param_1);
extern int func_00656B60(undefined4 *param_1,undefined1 param_2);
extern float DAT_0012bd88;
extern float DAT_0012bd8c;
extern undefined DAT_0012bd90;
extern uint DAT_0012bda8;
extern undefined4 _DAT_0012bd90;

// USA 0012BC9C <- EU 0012BC9C (Ghidra, non-matching)
// GLOBAL DAT_0012bd88 float
// GLOBAL DAT_0012bd8c float
// GLOBAL DAT_0012bd90 undefined
// GLOBAL DAT_0012bda8 uint

/* WARNING: Variable defined which should be unmapped: objStack_18 */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 func_0012BC9C(int *param_1)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  int iVar4;
  _Alignas(8) unsigned char albw_frame[0x20];
  #define objStack_18 ((uint *)(albw_frame + 0x8))
  func_004FB064((int)param_1);
  fVar2 = ALBW_AS_F32(_DAT_0012bd90);
  fVar1 = DAT_0012bd8c;
  param_1[0xf8] = (int)ALBW_AS_U32((ALBW_AS_F32(param_1[0xf8]) - DAT_0012bd88));
  param_1[0x13c] = (int)ALBW_AS_U32((fVar2 + (ALBW_AS_F32(param_1[0xf5]) - ALBW_AS_F32(param_1[0x243])) * fVar1));
  if (ALBW_AS_F32(param_1[0xf5]) < ALBW_AS_F32(param_1[0x243])) {
    objStack_18[0] = 0;
    iVar4 = func_004FB208((int)param_1,0,(float *)0x0,0,0);
    if (iVar4 != 0) {
      return func_00656B60(param_1 + 0x22f,5);
    }
  }
  iVar4 = func_0058A8F0((int)param_1);
  if ((iVar4 != 0) && (bVar3 = func_00521078((int)param_1), bVar3)) {
    func_00656B60(param_1 + 0x22f,2);
    objStack_18[0] = DAT_0012bda8;
    objStack_18[1] = (uint)(uintptr_t)"SE_Pamet_LAND_SHELL";   /* the name: the converted text lost this store */
    return func_00150A84(param_1,(int *)objStack_18,0);
  }
  return 0;
}

