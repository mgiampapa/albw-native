/* compilefix: USA 00551970 (pushable object update), the linked Ghidra text with the lost object argument of the two
 * height tests restored: native/compilefix/func_0054E650.c takes (this, value, centre, radius) and the converted text
 * passed only the three floats, so every argument arrived one place early and the radius was leftover data
 * (EU 005519CC..005519D8: r0 = this, s0 / s1 / s2). BOOT_LOG 171. */
#include "ghidra_types.h"

extern int func_00115C54(float *param_1,float *param_2,float *param_3);
extern int func_00115D70(float *param_1,float *param_2,float fparam_0);
extern int func_0054E228(int *param_1);
extern float func_0054E4C4(undefined4 param_1,undefined4 *param_2,undefined4 param_3);
extern int func_0054E650(void *self,float value,float centre,float radius);
extern int func_0054E6B0(int *param_1,float *param_2,int *param_3);
extern int func_0054EA0C(int *param_1);
extern float DAT_00551a90;
extern int *DAT_00551a94;
extern float DAT_00551a98;
extern float DAT_00551a9c;

// USA 00551970 <- EU 00551948 (Ghidra, non-matching)
// GLOBAL DAT_00551a90 float
// GLOBAL DAT_00551a94 int *
// GLOBAL DAT_00551a98 float
// GLOBAL DAT_00551a9c float

undefined4 func_00551970(int *param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  float fVar3;
  _Alignas(8) unsigned char albw_frame[0x60];
  #define local_58 (*(float *)(albw_frame + 0x8))
  #define uStack_54 (*(undefined4 *)(albw_frame + 0xc))
  #define uStack_50 (*(undefined4 *)(albw_frame + 0x10))
  #define local_4c (*(float *)(albw_frame + 0x14))
  #define uStack_48 (*(undefined4 *)(albw_frame + 0x18))
  #define uStack_44 (*(undefined4 *)(albw_frame + 0x1c))
  #define afStack_38 ((float *)(albw_frame + 0x28))
  #define local_28 (*(float *)(albw_frame + 0x38))
  #define uStack_24 (*(undefined4 *)(albw_frame + 0x3c))
  #define uStack_20 (*(undefined4 *)(albw_frame + 0x40))
  #define local_1c (*(int *)(albw_frame + 0x44))
  fVar3 = ALBW_AS_F32(param_1[0xf5]) - ALBW_AS_F32(param_1[0xdb]);
  if (fVar3 <= DAT_00551a90) {
    fVar3 = -fVar3;
  }
  if (((fVar3 <= ALBW_AS_F32(param_1[0x131])) && ((char)param_1[0x142] != '\x05')) &&
     (iVar1 = func_0054EA0C(param_1), iVar1 == 0)) {
    local_1c = 0;
    func_0054E6B0(param_1,afStack_38,&local_1c);
    fVar3 = func_0054E4C4(param_1,afStack_38,0xffffffff);
    iVar1 = func_0054E650(param_1,fVar3,ALBW_AS_F32(param_1[0xf5]),ALBW_AS_F32(param_1[0x131]));
    if (iVar1 == 0) {
      if (*(char *)(*DAT_00551a94 + 0x18) == '\x12') {
        func_00115D70(&local_28,(float *)(param_1 + 0xf4),DAT_00551a98);
        local_4c = local_28;
        uStack_48 = uStack_24;
        uStack_44 = uStack_20;
        func_00115D70(&local_28,afStack_38,DAT_00551a9c);
        local_58 = local_28;
        uStack_54 = uStack_24;
        uStack_50 = uStack_20;
        func_00115C54(afStack_38,&local_4c,&local_58);
        fVar3 = func_0054E4C4(param_1,afStack_38,0xffffffff);
        iVar1 = func_0054E650(param_1,fVar3,ALBW_AS_F32(param_1[0xf5]),ALBW_AS_F32(param_1[0x131]));
        if (iVar1 != 0) {
          return 0;
        }
      }
      uVar2 = func_0054E228(param_1);
      return uVar2;
    }
  }
  return 0;
}

