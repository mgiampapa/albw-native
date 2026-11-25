/* compilefix: USA 0049F434 (EU 0049F40C, insertion sort of an intrusive list), the linked Ghidra text with the second
 * argument of the first compare restored (EU 0049F440..0049F450: r0 = piVar4 - offset, r1 = piVar1 - offset). */
#include "ghidra_types.h"


// USA 0049F434 <- EU 0049F40C (Ghidra, non-matching)

void func_0049F434(int *param_1,int param_2,undefined *param_3)

{
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  int *piVar4;
  int *piVar5;
  
  if (1 < param_1[2]) {
    if ((uint)param_1[2] < 2) {
      piVar5 = (int *)0x0;
    }
    else {
      piVar5 = *(int **)(param_1[1] + 4);
    }
    while (piVar1 = piVar5, piVar1 != param_1) {
      piVar4 = (int *)*piVar1;
      piVar5 = (int *)piVar1[1];
      iVar2 = (*(code *)param_3)((int)piVar4 - param_2,(int)piVar1 - param_2);
      if (0 < iVar2) {
        piVar4 = (int *)*piVar4;
        while ((piVar4 != param_1 &&
               (iVar2 = (*(code *)param_3)((int)piVar4 - param_2,(int)piVar1 - param_2), 0 < iVar2))
              ) {
          piVar4 = (int *)*piVar4;
        }
        if (*piVar1 != 0) {
          *(int *)(*piVar1 + 4) = piVar1[1];
        }
        if ((int *)piVar1[1] != (int *)0x0) {
          *(int *)piVar1[1] = *piVar1;
        }
        piVar1[1] = 0;
        *piVar1 = 0;
        puVar3 = (undefined4 *)piVar4[1];
        piVar4[1] = (int)piVar1;
        piVar1[1] = (int)puVar3;
        *piVar1 = (int)piVar4;
        if (puVar3 != (undefined4 *)0x0) {
          *puVar3 = piVar1;
        }
      }
    }
  }
  return;
}

