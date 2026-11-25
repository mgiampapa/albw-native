/* compilefix: USA 00163870, the linked Ghidra text (ghidra_analyzed5_v4) with one literal fixed: Ghidra read the literal
 * pool after the function as a string ("33s?" = the float 0.95) and the next pool word, the address 0x00706E20 of the
 * actor manager pointer, as bytes inside it, so it was never moved to its PC address: the game read 3DS address
 * 0x706E20 and crashed (user report, BOOT_LOG 169). */
#include "ghidra_types.h"

extern int func_00150A84(int *param_1,int *param_2,int param_3);
extern int func_00163A30(int *param_1,int param_2);
extern int func_004F87F8(int param_1,int param_2);
extern int func_004FABDC(int param_1,undefined4 param_2,uint param_3,int param_4);
extern int func_0052FBA0(int param_1,uint param_2);
extern int iRam0016392c;
extern char data_00706E20[];
extern char s_33s__np_00163910[];
extern char s_SE_Omugai_SHELL_IN_00163918[];
extern char s_cf_00163910[];

// USA 00163870 <- EU 00163874 (Ghidra, non-matching)
// GLOBAL UNK_0016392c int
// GLOBAL s_cf_00163910 undefined
// GLOBAL s_SE_Omugai_SHELL_IN_00163918 string

undefined4 func_00163870(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  _Alignas(8) unsigned char albw_frame[0x10];
  #define iStack_10 (*(int *)(albw_frame + 0x0))
  #define pcStack_c (*(char * *)(albw_frame + 0x4))
  uVar1 = (*(undefined4 *)((char *)&s_33s__np_00163910 + 0));
  param_1[0x139] = (*(undefined4 *)((char *)&s_33s__np_00163910 + 0));
  param_1[0x13a] = uVar1;
  param_1[0x13b] = uVar1;
  func_00163A30(param_1,1);
  func_004F87F8((int)param_1,1);
  func_004FABDC((int)param_1,0xb,0,0);
  iVar2 = func_0052FBA0(*(int *)data_00706E20 + 0x3dbbc,param_1[0x23e]);
  iStack_10 = iRam0016392c;
  if (iVar2 != 0) {
    param_1[0xfa] = *(int *)(iVar2 + 1000);
  }
  pcStack_c = s_SE_Omugai_SHELL_IN_00163918;
  return func_00150A84(param_1,&iStack_10,0);
}

