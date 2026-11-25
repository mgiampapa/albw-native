/* compilefix: USA 00689C8C (EU 00689C64, the heap sort behind the effect draw order), the linked Ghidra text
 * (ghidra_analyzed5_v4) with two lost arguments restored: the compare of the two children was called with one element
 * only (the ARM passes r0 = &child, r1 = &child + 1: EU 00689CCC..00689CD4 and 00689D6C..00689D74). The compare read
 * its second element through leftover data and crashed (user report, BOOT_LOG 170). */
#include "ghidra_types.h"


// USA 00689C8C <- EU 00689C64 (Ghidra, non-matching)

void func_00689C8C(uint *param_1,undefined *param_2,uint param_3,uint param_4)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  bool bVar10;
  bool bVar11;
  _Alignas(8) unsigned char albw_frame[0x30];
  #define local_28 (*(undefined4 *)(albw_frame + 0x8))
  #define uStack_24 (*(undefined4 *)(albw_frame + 0xc))
  uVar3 = *param_1;
  bVar11 = param_3 <= uVar3;
  bVar10 = uVar3 == param_3;
  if (bVar11 && !bVar10) {
    bVar11 = param_4 <= uVar3;
    bVar10 = uVar3 == param_4;
  }
  if ((bVar11 && !bVar10) && (iVar8 = (param_4 - param_3) + 1, 1 < iVar8)) {
    puVar5 = (undefined4 *)(param_1[1] + param_3 * 8);
    iVar9 = iVar8 / 2;
    if (0 < iVar9) {
      do {
        iVar1 = iVar9 * 2 - iVar8;
        local_28 = puVar5[iVar9 * 2 + -2];
        uStack_24 = puVar5[iVar9 * 2 + -1];
        iVar2 = iVar9 * 2;
        iVar7 = iVar9;
        while (iVar6 = iVar2, iVar6 <= iVar8) {
          if ((iVar1 < 0 != SBORROW4(iVar6,iVar8)) &&
             (iVar1 = (*(code *)param_2)(puVar5 + iVar6 * 2 + -2, puVar5 + iVar6 * 2), iVar1 < 0)) {
            iVar6 = iVar6 + 1;
          }
          iVar1 = (*(code *)param_2)(&local_28,puVar5 + iVar6 * 2 + -2);
          if (-1 < iVar1) break;
          uVar4 = puVar5[iVar6 * 2 + -1];
          iVar1 = iVar6 * 2 - iVar8;
          puVar5[iVar7 * 2 + -2] = puVar5[iVar6 * 2 + -2];
          puVar5[iVar7 * 2 + -1] = uVar4;
          iVar2 = iVar6 * 2;
          iVar7 = iVar6;
        }
        iVar9 = iVar9 + -1;
        puVar5[iVar7 * 2 + -2] = local_28;
        puVar5[iVar7 * 2 + -1] = uStack_24;
      } while (0 < iVar9);
      if (iVar8 < 2) {
        return;
      }
    }
    do {
      local_28 = puVar5[iVar8 * 2 + -2];
      uStack_24 = puVar5[iVar8 * 2 + -1];
      iVar1 = iVar8 + -1;
      uVar4 = puVar5[1];
      iVar9 = 2;
      puVar5[iVar8 * 2 + -2] = *puVar5;
      puVar5[iVar8 * 2 + -1] = uVar4;
      iVar8 = 1;
      if (1 < iVar1) {
        do {
          iVar7 = iVar9;
          if ((iVar9 < iVar1) && (iVar2 = (*(code *)param_2)(puVar5 + iVar9 * 2 + -2, puVar5 + iVar9 * 2), iVar2 < 0)) {
            iVar7 = iVar9 + 1;
          }
          iVar9 = (*(code *)param_2)(&local_28,puVar5 + iVar7 * 2 + -2);
          if (-1 < iVar9) break;
          uVar4 = puVar5[iVar7 * 2 + -1];
          iVar9 = iVar7 * 2;
          puVar5[iVar8 * 2 + -2] = puVar5[iVar7 * 2 + -2];
          puVar5[iVar8 * 2 + -1] = uVar4;
          iVar8 = iVar7;
        } while (iVar9 <= iVar1);
      }
      puVar5[iVar8 * 2 + -2] = local_28;
      puVar5[iVar8 * 2 + -1] = uStack_24;
      iVar8 = iVar1;
    } while (1 < iVar1);
  }
  return;
}

