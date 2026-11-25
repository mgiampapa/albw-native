/* compilefix: USA 00108FF0 (EU 00108FF0; eu_usa_map.json says EU 00108FDC, which is the
 * neighbouring 4-instruction function, wrongly paired: the matched source and the ARM agree on
 * EU 00108FF0..00109010). SDK result check:
 *   func_00108DF8(); func_00108B98(); r = func_0010B240();
 *   if (r < 0) func_0010873C(r, pc, 1);     -- `mov r1, pc` at 00109000 reads 0x00109008
 *   tail-call func_00108F4C();
 * Failed to compile: the matched C++ source (src/work7/gpt15/variants/00108FF0/v0.cpp) uses the
 * armcc intrinsic __current_pc(); the Ghidra fallback is the wrong function (map pairing).
 * Fix: C version of the matched source; the pc argument (only used as the error location of the
 * fatal-result diagnostic) is the constant the ARM computes, 0x00109008 (USA layout = EU here:
 * the callees 00108DF8/00108B98/0010B240/0010873C/00108F4C have equal USA and EU addresses).
 * Confidence: high. */
#include <stdint.h>

void func_00108DF8(void);
uint32_t func_00108B98(void);
uint32_t func_0010B240(uint32_t h);
void func_0010873C(uint32_t r, uint32_t pc, uint32_t sign);
void func_00108F4C(void);

void func_00108FF0(void)
{
    func_00108DF8();
    uint32_t h = func_00108B98();
    uint32_t r = func_0010B240(h);   /* r0 passes straight on (EU 00108FF8..00108FFC; BOOT_LOG 171) */
    if (r >> 31)
        func_0010873C(r, 0x00109008u, r >> 31);
    func_00108F4C();
}
