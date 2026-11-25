/* USA 0013066C (EU 0013066C) and three more "perm_local" matching sources that compiled to
 * nothing: src/work8/perm_local/imported/func_{0013066C,003D2C58,0046CE10,0048C028}.c branch on
 * an uninitialised local (`if (uVar1) {A} else {A}`, a permuter trick for the ARM compiler),
 * which clang treats as undefined behaviour and folds the whole function away (0013066C was an
 * empty symbol that fell through into the next function). Same logic, defined C; checked
 * against the EU disassembly (0013066C: EU 0013066C..001306D4). Confidence: high.
 * Systemic: build_decomp should compile with -ftrivial-auto-var-init=zero (BOOT_LOG stop 60). */
#include <stdint.h>

extern int func_00130334(int self, int a);
extern int func_001309A4(int self, int a, int b, int c);
extern int func_002FE86C(int p);
extern int func_002FE9A8(int p);
extern int func_002FE9CC(int p);
extern int func_003D2F18(int a, uint32_t b, uint32_t *c);
extern int func_0034C57C(int p, int n);
extern int func_005568D4(int p);
extern int func_001119F4(uint32_t *p, uint32_t a);
extern int func_00115DC8(uint32_t *p);
extern char data_007158E8[], data_006F5108[], data_006F7500[], data_007A96F0[], data_0070FB60[];

#define W(p, o) (*(int32_t *)((char *)(uintptr_t)(p) + (o)))

void func_0013066C(int param_1)
{
    func_00130334(param_1, 0);
    if (*(int *)(uintptr_t)W(param_1, 0x2888) != 0 && (*(uint8_t *)(uintptr_t)(param_1 + 0x2c1e) & 1) == 0) {
        int v = func_002FE9CC(W(param_1, 0x4c));
        func_001309A4(param_1, v, W(param_1, 0x28d0), 0);
        func_002FE86C(W(param_1, 0x4c));
        return;
    }
    func_002FE9A8(W(param_1, 0x4c));
}

/* USA 003D2C58 (EU 003D2C34): *out = -1; if (G_007158E8.+8) 003D2F18(it, a, out); return 0xC8A0A7F8 */
uint32_t func_003D2C58(uint32_t param_1, uint32_t *param_2)
{
    *param_2 = 0xffffffffu;
    if (W(data_007158E8, 8) != 0)
        func_003D2F18(W(data_007158E8, 8), param_1, param_2);
    return 0xC8A0A7F8u;
}

/* USA 0046CE10 (EU 0046CDE8) */
void func_0046CE10(int param_1)
{
    W(param_1, 0xc4) = 0;
    func_0034C57C(W(param_1, 0xb4), 0);   /* r1 = 0 from EU 0046CDEC (BOOT_LOG 171) */
    int v = func_005568D4(*(int32_t *)data_0070FB60);
    W(param_1, 200) = v;
    *(uint8_t *)(uintptr_t)(W(param_1, 0xb4) + 0x58) = 1;
}

/* USA 0048C028 (EU 0048C000): ctor: base 001119F4(this+1, G_007A96F0), vtable G_006F5108, three
 * vec3 copies to +0x34/+0x40/+0x4C, 00115DC8(this+0x13 words); returns this. */
uint32_t *func_0048C028(uint32_t *param_1, uint32_t *param_2, uint32_t *param_3, uint32_t *param_4)
{
    *param_1 = (uint32_t)(uintptr_t)data_006F7500;
    func_001119F4(param_1 + 1, (uint32_t)(uintptr_t)data_007A96F0);
    *param_1 = (uint32_t)(uintptr_t)data_006F5108;
    for (int i = 0; i < 3; i++) {
        param_1[0xd + i] = param_2[i];
        param_1[0x10 + i] = param_3[i];
        param_1[0x13 + i] = param_4[i];
    }
    func_00115DC8(param_1 + 0x13);
    return param_1;
}
