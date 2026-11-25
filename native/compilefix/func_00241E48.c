/* compilefix: USA 00241E48 (waterfall switch actor update), the linked Ghidra text (ghidra_analyzed5_v4) with the lost
 * stack stores restored. Three SafeString {vtable, char *} objects were built with the vtable only, so the name was
 * whatever lay on the stack: strlen(NULL) in 00587FCC (crash when a switch was hit in the Swamp Palace). The strings
 * from the ARM: 002420AC "WaterFall2", 00242144 "SE_Water_FALL_START_AFTER_STOP", 00242288 "SE_Water_FALL_START". */
#include "ghidra_types.h"

extern int func_001507D8(int param_1,int *param_2,int param_3);
extern int func_00150CD0(int param_1,int *param_2,undefined4 param_3,int param_4,int param_5);
extern int func_00260C78(float fparam_0,float fparam_1,float fparam_2,float fparam_3);
extern int func_004A1B9C(int *param_1,int param_2,uint *param_3);
extern int func_004CEA18(int param_1,int param_2,uint param_3);
extern int func_00581D60(int param_1);
extern int func_00584C14(int param_1,int *param_2);
extern int func_00588BE4(int param_1);
extern int func_0065F2D8(undefined4 *param_1,undefined1 param_2);
extern int *DAT_002421a0;
extern undefined DAT_002421a4;
extern int *DAT_002421b0;
extern float DAT_002421b4;
extern int *DAT_002421b8;
extern float DAT_002421bc;
extern int *DAT_002421c0;
extern undefined DAT_002421c4;
extern int *DAT_002421d0;
extern uint DAT_002421d4;
extern undefined DAT_002421d8;
extern int DAT_002421e8;
extern undefined4 DAT_002421ec;
extern int DAT_002421f0;
extern float DAT_002421f4;
extern float DAT_002421f8;
extern undefined DAT_002421fc;
extern float DAT_00242220;
extern int *DAT_002422f8;
extern int *DAT_002422fc;
extern int *DAT_00242314;
extern char s_BSplash_002421c7[];
extern char s_RCSplash2_002421a6[];
extern char s_SE_Water_FALL_00242318[];
extern undefined4 _DAT_002421a4;
extern undefined4 _DAT_002421c4;
extern undefined4 _DAT_002421d8;
extern undefined4 _DAT_002421fc;

// USA 00241E48 <- EU 00241E48 (Ghidra, non-matching)
// GLOBAL DAT_002421a0 int *
// GLOBAL DAT_002421a4 undefined
// GLOBAL DAT_002421b0 int *
// GLOBAL DAT_002421b4 float
// GLOBAL DAT_002421b8 int *
// GLOBAL DAT_002421bc float
// GLOBAL DAT_002421c0 int *
// GLOBAL DAT_002421c4 undefined
// GLOBAL DAT_002421d0 int *
// GLOBAL DAT_002421d4 uint
// GLOBAL DAT_002421d8 undefined
// GLOBAL DAT_002421e8 int
// GLOBAL DAT_002421ec undefined4
// GLOBAL DAT_002421f0 int
// GLOBAL DAT_002421f4 float
// GLOBAL DAT_002421f8 float
// GLOBAL DAT_002421fc undefined
// GLOBAL DAT_00242220 float
// GLOBAL DAT_002422f8 int *
// GLOBAL DAT_002422fc int *
// GLOBAL DAT_00242314 int *
// GLOBAL s_BSplash_002421c7 string
// GLOBAL s_RCSplash2_002421a6 string
// GLOBAL s_SE_Water_FALL_00242318 string

/* WARNING: Variable defined which should be unmapped: objStack_30 */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void func_00241E48(int param_1)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint *puVar5;
  float fVar6;
  _Alignas(8) unsigned char albw_frame[0x30];
  #define objStack_30 ((uint *)(albw_frame + 0x0))
  #define local_2c (*(undefined4 *)(albw_frame + 0x4))
  #define local_28 (*(uint *)(albw_frame + 0x8))
  #define local_24 (*(char * *)(albw_frame + 0xc))
  iVar3 = func_00588BE4(param_1);
  piVar1 = DAT_002421a0;
  if (iVar3 == *DAT_002421a0) {
    iVar3 = *(int *)(*(int *)(param_1 + 0x418) + 0x14);
    iVar3 = func_00260C78(ALBW_AS_F32(_DAT_002421a4),*(float *)(iVar3 + 8),*(float *)(iVar3 + 0xc),
                          *(float *)(iVar3 + 0x14));
    if ((iVar3 != 0) && (*(int *)(param_1 + 0x428) == 0)) {
      iVar3 = *(int *)(param_1 + 0x344);
      puVar5 = (uint *)(s_RCSplash2_002421a6 + 2);
joined_r0x00241fe8:
      if (iVar3 == 0) {
        *(undefined4 *)(param_1 + 0x42c) = 0;
      }
      else {
        func_004A1B9C((int *)objStack_30,iVar3,puVar5);
        *(uint *)(param_1 + 0x428) = objStack_30[0];
        *(undefined4 *)(param_1 + 0x42c) = local_2c;
      }
    }
  }
  else {
    iVar3 = func_00588BE4(param_1);
    if (iVar3 == *DAT_002421b0) {
      iVar3 = *(int *)(*(int *)(param_1 + 0x418) + 0x14);
      iVar3 = func_00260C78(DAT_002421b4,*(float *)(iVar3 + 8),*(float *)(iVar3 + 0xc),
                            *(float *)(iVar3 + 0x14));
      if ((iVar3 != 0) && (*(int *)(param_1 + 0x428) == 0)) {
        iVar3 = *(int *)(param_1 + 0x344);
        puVar5 = (uint *)(s_RCSplash2_002421a6 + 2);
        goto joined_r0x00241fe8;
      }
    }
    else {
      iVar3 = func_00588BE4(param_1);
      if (iVar3 == *DAT_002421b8) {
        iVar3 = *(int *)(*(int *)(param_1 + 0x418) + 0x14);
        iVar3 = func_00260C78(DAT_002421bc,*(float *)(iVar3 + 8),*(float *)(iVar3 + 0xc),
                              *(float *)(iVar3 + 0x14));
        if ((iVar3 != 0) && (*(int *)(param_1 + 0x428) == 0)) {
          iVar3 = *(int *)(param_1 + 0x344);
          puVar5 = (uint *)(s_RCSplash2_002421a6 + 2);
          goto joined_r0x00241fe8;
        }
      }
      else {
        iVar3 = func_00588BE4(param_1);
        if (iVar3 == *DAT_002421c0) {
          iVar3 = *(int *)(*(int *)(param_1 + 0x418) + 0x14);
          iVar3 = func_00260C78(ALBW_AS_F32(_DAT_002421c4),*(float *)(iVar3 + 8),*(float *)(iVar3 + 0xc),
                                *(float *)(iVar3 + 0x14));
          if ((iVar3 != 0) && (*(int *)(param_1 + 0x428) == 0)) {
            iVar3 = *(int *)(param_1 + 0x344);
            puVar5 = (uint *)(s_BSplash_002421c7 + 1);
            goto joined_r0x00241fe8;
          }
        }
        else {
          fVar6 = ALBW_AS_F32(_DAT_002421c4);
          iVar3 = func_00588BE4(param_1);
          if (iVar3 == *DAT_002421d0) {
            iVar3 = *(int *)(*(int *)(param_1 + 0x418) + 0x14);
            iVar3 = func_00260C78(fVar6,*(float *)(iVar3 + 8),*(float *)(iVar3 + 0xc),
                                  *(float *)(iVar3 + 0x14));
            if ((iVar3 != 0) && (*(int *)(param_1 + 0x428) == 0)) {
              iVar3 = *(int *)(param_1 + 0x344);
              puVar5 = (uint *)(s_BSplash_002421c7 + 1);
              goto joined_r0x00241fe8;
            }
          }
        }
      }
    }
  }
  iVar3 = func_00588BE4(param_1);
  uVar2 = DAT_002421d4;
  if (iVar3 == *piVar1) {
    iVar3 = *(int *)(*(int *)(param_1 + 0x418) + 0x14);
    iVar3 = func_00260C78(ALBW_AS_F32(_DAT_002421d8),*(float *)(iVar3 + 8),*(float *)(iVar3 + 0xc),
                          *(float *)(iVar3 + 0x14));
    if (iVar3 != 0) {
      objStack_30[0] = uVar2;
      objStack_30[1] = (uint)(uintptr_t)"WaterFall2";   /* the name: the converted text lost this store */
      uVar4 = func_00584C14(*(int *)(param_1 + 0x40c),(int *)objStack_30);
      func_004CEA18(*(int *)(param_1 + 0x40c),1,uVar4);
    }
    iVar3 = *(int *)(param_1 + 0x414);
    fVar6 = *(float *)(*(int *)(iVar3 + 0x14) + 8);
    if ((int)ALBW_AS_U32(fVar6) < DAT_002421e8) {
      *(undefined4 *)(*(int *)(iVar3 + 0x14) + 0x10) = DAT_002421ec;
    }
    else if ((int)ALBW_AS_U32(fVar6) < DAT_002421f0) {
      *(float *)(*(int *)(iVar3 + 0x14) + 0x10) = DAT_002421f8 - fVar6 * DAT_002421f4;
    }
    else {
      *(undefined4 *)(*(int *)(iVar3 + 0x14) + 0x10) = _DAT_002421fc;
      objStack_30[0] = uVar2;
      objStack_30[1] = (uint)(uintptr_t)"SE_Water_FALL_START_AFTER_STOP";   /* the name: the converted text lost this store */
      func_001507D8(param_1,(int *)objStack_30,0);
    }
  }
  iVar3 = *(int *)(*(int *)(param_1 + 0x418) + 0x14);
  if (*(char *)(iVar3 + 0x1c) == '\0') {
    if (*(float *)(iVar3 + 0x10) < DAT_00242220) {
      if (DAT_00242220 < *(float *)(iVar3 + 8)) goto LAB_00242244;
    }
    else if (*(float *)(iVar3 + 8) != *(float *)(iVar3 + 0x14)) goto LAB_00242244;
    func_0065F2D8((undefined4 *)(param_1 + 0x43c),3);
  }
LAB_00242244:
  iVar3 = func_00588BE4(param_1);
  if (((iVar3 != *piVar1) && (iVar3 = func_00588BE4(param_1), iVar3 != *DAT_002422f8)) &&
     (iVar3 = func_00588BE4(param_1), iVar3 != *DAT_002422fc)) {
    objStack_30[0] = uVar2;
      objStack_30[1] = (uint)(uintptr_t)"SE_Water_FALL_START";   /* the name: the converted text lost this store */
    func_001507D8(param_1,(int *)objStack_30,0);
    if ((*(int *)(param_1 + 0x428) != 0) &&
       (iVar3 = func_00581D60(*DAT_00242314), iVar3 == *(short *)(param_1 + 0x54))) {
      local_24 = s_SE_Water_FALL_00242318;
      local_28 = uVar2;
      objStack_30[0] = 0;
      func_00150CD0(param_1,(int *)&local_28,param_1 + 0x430,1,0);
    }
  }
  return;
}

