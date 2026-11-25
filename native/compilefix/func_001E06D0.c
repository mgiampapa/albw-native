/* compilefix: USA 001E06D0, the linked Ghidra text (ghidra_analyzed5_v4) plus one check: when the resource loader
 * 001E1228 returns NULL the original reads [NULL+0x58] (crash report in issue 13). It now stops with the archive name. */
#include "ghidra_types.h"
extern void runtime_fatal(const char *fmt, ...) __attribute__((noreturn));

extern int func_0010B954(int param_1,int param_2,int *param_3,int param_4,int param_5);
extern undefined8 func_0010E3C4(uint *param_1,uint *param_2,uint param_3);
extern int func_001E1228(int param_1,int param_2,int *param_3);
extern int func_00489CE0(int *param_1,int param_2,undefined4 param_3);
extern int func_00689650(undefined4 *param_1,undefined4 param_2,uint param_3,undefined4 param_4,...);
extern int *DAT_001e0aac;
extern int *DAT_001e0ab0;
extern int *DAT_001e0abc;
extern int DAT_001e0ad4;
extern undefined DAT_001e0ae0;
extern char s_ArcHeap_001e0ad8[];
extern char s_Archive__s_001e0ac0[];
extern char s_World_001e0ab4[];
extern char s__s__s_001e0acc[];
extern char s_cf_001e0ac0[];
extern char s_cf_001e0acc[];

// USA 001E06D0 <- EU 001E06D0 (Ghidra, non-matching)
// GLOBAL DAT_001e0aac int *
// GLOBAL DAT_001e0ab0 int *
// GLOBAL DAT_001e0abc int *
// GLOBAL DAT_001e0ad4 int
// GLOBAL DAT_001e0ae0 undefined
// GLOBAL s_cf_001e0acc string
// GLOBAL s_ArcHeap_001e0ad8 string
// GLOBAL s_cf_001e0ac0 string
// GLOBAL s_World_001e0ab4 string

/* WARNING: Variable defined which should be unmapped: objStack_190 */

void func_001E06D0(int param_1,int param_2,int *param_3)

{
  int *piVar1;
  undefined4 *puVar2;
  uint uVar3;
  int *piVar4;
  int iVar5;
  int *piVar6;
  undefined4 *puVar7;
  uint *puVar8;
  _Alignas(8) unsigned char albw_frame[0x190];
  #define objStack_190 ((uint *)(albw_frame + 0x0))
  #define local_188 (*(int * *)(albw_frame + 0x8))
  #define local_184 (*(int * *)(albw_frame + 0xc))
  #define local_180 (*(int * *)(albw_frame + 0x10))
  #define local_17c (*(int * *)(albw_frame + 0x14))
  #define uStack_178 (*(undefined4 *)(albw_frame + 0x18))
  #define local_174 (*(undefined4 *)(albw_frame + 0x1c))
  #define local_170 (*(undefined4 *)(albw_frame + 0x20))
  #define local_16c (*(undefined4 *)(albw_frame + 0x24))
  #define local_168 (*(undefined4 *)(albw_frame + 0x28))
  #define local_164 (*(undefined4 *)(albw_frame + 0x2c))
  #define uStack_160 (*(undefined4 *)(albw_frame + 0x30))
  #define piStack_15c (*(int * *)(albw_frame + 0x34))
  #define local_158 (*(char * *)(albw_frame + 0x38))
  #define local_154 (*(undefined1 *)(albw_frame + 0x3c))
  #define local_153 (*(undefined1 *)(albw_frame + 0x3d))
  #define local_152 (*(undefined1 *)(albw_frame + 0x3e))
  #define local_151 (*(undefined1 *)(albw_frame + 0x3f))
  #define local_150 (*(undefined1 *)(albw_frame + 0x40))
  #define uStack_14c (*(undefined4 *)(albw_frame + 0x44))
  #define local_148 (*(int * *)(albw_frame + 0x48))
  #define local_144 (*(undefined * *)(albw_frame + 0x4c))
  #define local_13c (*(undefined4 * *)(albw_frame + 0x54))
  #define local_3c (*(int *)(albw_frame + 0x154))
  #define local_30 (*(int * *)(albw_frame + 0x160))
  #define local_2c (*(char * *)(albw_frame + 0x164))
  piVar1 = DAT_001e0abc;
  piVar4 = DAT_001e0aac;
  puVar2 = &uStack_160;
  local_184 = DAT_001e0ab0;
  local_180 = (int *)0x0;
  local_188 = DAT_001e0aac;
  local_17c = (int *)0x0;
  uStack_178 = 0x20;
  local_16c = 0;
  local_158 = s_World_001e0ab4;
  local_174 = 0;
  local_170 = 0;
  local_168 = 0;
  local_164 = 0;
  uStack_160 = 0;
  piStack_15c = DAT_001e0aac;
  local_154 = 7;
  local_153 = 4;
  puVar7 = (undefined4 *)(param_1 + 0x1fa8);
  local_152 = 0;
  local_151 = 0;
  local_150 = 0;
  piVar6 = (int *)*puVar7;
  puVar8 = (uint *)(param_1 + 0x1fa4);
  switch(param_2) {
  case 0:
    (**(code **)(*param_3 + 8))(param_3);
    objStack_190[0] = param_3[1];
    local_30 = piVar4;
    local_2c = *(char **)(*piVar1 + 0x38);
    (*(code *)piVar4[2])(&local_30);
    func_00689650(&uStack_14c,s__s__s_001e0acc,(uint)local_2c,objStack_190[0]);
    local_184 = local_148;
    break;
  case 1:
    (**(code **)(*param_3 + 8))(param_3);
    objStack_190[0] = param_3[1];
    local_30 = piVar4;
    local_2c = *(char **)(*piVar1 + 100);
    (*(code *)piVar4[2])(&local_30);
    func_00689650(&uStack_14c,s__s__s_001e0acc,(uint)local_2c,objStack_190[0]);
    local_184 = local_148;
    break;
  case 2:
    (**(code **)(*param_3 + 8))(param_3);
    puVar2 = func_00689650(&uStack_14c,s_Archive__s_001e0ac0,param_3[1],puVar2);
    local_184 = (int *)puVar2[1];
    break;
  case 3:
    (**(code **)(*param_3 + 8))(param_3);
    local_3c = param_3[1];
    local_30 = piVar4;
    local_2c = *(char **)(*piVar1 + 100);
    (*(code *)piVar4[2])(&local_30);
    func_00689650(&uStack_14c,s__s__s_001e0acc,(uint)local_2c,local_3c);
    (*(undefined1 *)((char *)&objStack_190[0] + 0)) = 1;
    local_184 = local_148;
    uVar3 = func_00489CE0(*(int **)(DAT_001e0ad4 + param_1),(int)objStack_190,&uStack_14c);
    if ((uVar3 & (int)(char)objStack_190[0]) == 0) {
      return;
    }
    break;
  case 4:
    (**(code **)(*param_3 + 8))(param_3);
    puVar2 = func_00689650(&uStack_14c,s_Archive__s_001e0ac0,param_3[1],puVar2);
    local_184 = (int *)puVar2[1];
    local_2c = s_ArcHeap_001e0ad8;
    local_30 = piVar4;
    objStack_190[0] = 0;
    piVar6 = func_0010B954(0,(int)&local_30,(int *)*puVar7,1,0);
    iVar5 = param_1 + 0x10;
    if (*puVar8 < 7) {
      iVar5 = iVar5 + *puVar8 * 0x328;
    }
    *(int **)(iVar5 + 8) = piVar6;
  }
  local_180 = piVar6;
  local_17c = piVar6;
  puVar2 = func_001E1228(param_1,(int)&local_188,(int *)0x0);
  if (puVar2 == (undefined4 *)0x0) {   /* the archive did not load: say which, instead of reading address 0x58 */
    const char *albw_name = (const char *)local_184;
    runtime_fatal("could not load a game archive (%.200s) from the ROM: check that the dump is complete and unmodified",
                  albw_name && (uintptr_t)albw_name > 0x10000 ? albw_name : "name unknown");
  }
  local_13c = puVar2 + 0x15;
  local_144 = (undefined *)puVar2[0x16];
  local_148 = param_3;
  (**(code **)(*param_3 + 8))(param_3);
  uVar3 = 0;
  do {
    if (*(char *)(param_3[1] + uVar3) == '\0') {
      if ((int)uVar3 < 0x40001) goto LAB_001e09bc;
      break;
    }
    uVar3 = uVar3 + 1;
  } while ((int)uVar3 < 0x40001);
  uVar3 = 0;
LAB_001e09bc:
  if ((int)local_13c[2] <= (int)uVar3) {
    uVar3 = local_13c[2] - 1;
  }
  objStack_190[0] = (uint)local_144;
  (**(code **)(*local_148 + 8))(local_148);
  func_0010E3C4((uint *)objStack_190[0],(uint *)local_148[1],uVar3);
  *(undefined1 *)((int)local_144 + uVar3) = 0;
  *(char *)(puVar2 + 0x38) = (char)*puVar8;
  *(char *)((int)puVar2 + 0xe1) = (char)param_2;
  *(undefined4 **)(param_1 + 0x1fd4 + param_2 * 4) = puVar2;
  if ((int *)*puVar7 != piVar6) {
    (**(code **)(*piVar6 + 0x14))(piVar6);
    local_144 = &DAT_001e0ae0;
    local_148 = piVar4;
    objStack_190[0] = 0;
    piVar4 = func_0010B954(0,(int)&local_148,(int *)*puVar7,1,0);
    iVar5 = param_1 + 0x10;
    if (*puVar8 < 7) {
      iVar5 = iVar5 + *puVar8 * 0x328;
    }
    *(int **)(iVar5 + 4) = piVar4;
    iVar5 = param_1 + 0x10;
    if (*puVar8 < 7) {
      iVar5 = iVar5 + *puVar8 * 0x328;
    }
    *puVar7 = *(undefined4 *)(iVar5 + 4);
  }
  return;
}

