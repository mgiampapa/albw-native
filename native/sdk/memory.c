/* Native replacements for the memory part of the 3DS SDK (USA addresses), see native/sdk/memory.md.
 *
 * Layering (lowest level that removes the OS dependency, game allocator semantics kept):
 *
 *   OS memory   nn::os SetHeapSize / SetDeviceMemorySize / GetDeviceMemory* / app memory queries
 *               -> REPLACED: host address space reserved once with mmap, committed/decommitted
 *                  with mprotect/madvise. The only svc 0x01 ControlMemory wrapper (0x00101DC4) is
 *                  reached only from 0x00100DA4 and 0x00102304, both replaced, so it stays as the
 *                  decompiled tripwire. The kernel config page read (0x1FF80040 APPMEMALLOC) and
 *                  svc 0x3A GetResourceLimitCurrentValues are replaced by native values.
 *   sead heaps  HeapMgr::initialize, Arena, ExpHeap/FrameHeap create/alloc/free, findContainHeap
 *               -> STAY DECOMPILED: pure game code running on the memory above (their locks are
 *                  CriticalSections, owned by the threads area).
 *   new/delete  operator new / new[] (current heap or explicit heap), operator delete family,
 *               getCurrentHeap / setCurrentHeap
 *               -> REPLACED with pointer-typed C: the Ghidra versions lose the tail-call result
 *                  (they are void) and store pointers in 32-bit ints.
 *   C runtime   the game's own memcpy/memmove/memset/memclr/strlen/strcmp/strncmp copies
 *               -> REPLACED by host libc (pointer-size independent).
 *
 * Calling convention: callers are decompiled 32-bit ARM code, compiled cdecl. Every size argument
 * is the original 32-bit register value (uint32_t), every address argument a real pointer. */
#define _GNU_SOURCE
#include <execinfo.h>
#include <stdio.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include "../runtime/runtime.h"
#include "../include/gptr.h"

/* ================================================================== game symbols used here */

/* sead::Heap (polymorphic). Virtual slots used by the allocation wrappers, read from the game's
 * own code (ldr r3, [vtbl, #0x18] / ldr r2, [vtbl, #0x1C]): slot 6 alloc, slot 7 free. Indexed as
 * an array of function pointers so the expression is the same with native 64-bit vtables. */
typedef struct sead_Heap sead_Heap;
typedef void *(*sead_HeapAllocFn)(sead_Heap *self, uint32_t size, int32_t alignment);
typedef void (*sead_HeapFreeFn)(sead_Heap *self, void *p);
struct sead_Heap {
    void *const ALBW_GP *ALBW_GP vtbl; /* 4-byte game words (gptr.h) */
};
enum { SEAD_HEAP_VSLOT_ALLOC = 0x18 / 4, SEAD_HEAP_VSLOT_FREE = 0x1C / 4 };

static inline void *heap_alloc(sead_Heap *h, uint32_t size, int32_t alignment) {
    return ((sead_HeapAllocFn)h->vtbl[SEAD_HEAP_VSLOT_ALLOC])(h, size, alignment);
}
static inline void heap_free(sead_Heap *h, void *p) {
    ((sead_HeapFreeFn)h->vtbl[SEAD_HEAP_VSLOT_FREE])(h, p);
}

/* sead::ThreadMgr::sInstance (USA data 0x00715430). Its nn::os::ThreadLocalStorage member at +0x24
 * holds the current sead::Thread*; the thread's current heap is at Thread+0x68 (0x0010DB90,
 * 0x0010983C: `add r0, r0, #0x24; bl 0x10E294; ldr r0, [r0, #0x68]`). These two offsets are the
 * 32-bit layouts of game objects; they become field names once sead::Thread/ThreadMgr are typed
 * (64-bit open item, see memory.md). */
extern void * ALBW_GP data_00715430;
enum { SEAD_THREADMGR_TLS_OFFSET = 0x24, SEAD_THREAD_CURRENT_HEAP_OFFSET = 0x68 };

/* sead::HeapMgr::sInstancePtr (USA data 0x00715210): passed as `this` by the wrappers */
extern void * ALBW_GP data_00715210;

/* nn::os::ThreadLocalStorage::GetValue (threads area): this thread's value of the slot */
void *func_0010E294(const void *tls);
/* sead::HeapMgr::findContainHeap(this, ptr) (decompiled, pure game code) */
sead_Heap *func_0010E2A4(void *heap_mgr, const void *p);
/* destructor of the class deleted by 0x0058FA4C; returns `this` */
void *func_0048FBB4(void *self);
/* nn::os heap bookkeeping run after every heap-size change by the original 0x00100DA4:
 * MemoryBlock address-space manager init (once) over [base, base+size), then allocator hook */
void func_00101390(void *base, uint32_t size);
void func_001013C4(void);

/* ================================================================== OS memory (nn::os)
 *
 * The 3DS gives the application two memory regions:
 *   - the heap at the fixed address 0x08000000 (sized by SetHeapSize, 0x00100DA4), used by the
 *     nn::os MemoryBlock manager (the CRT/nn allocator and sead's "RootHeapMemory");
 *   - the device memory / linear heap (SetDeviceMemorySize, 0x00102304; kernel-chosen address
 *     returned by GetDeviceMemoryAddress, 0x0010EE40): sead::HeapMgr's arena, i.e. the RootHeap
 *     every game heap comes from.
 * Natively each region is a host reservation of the whole application memory size (contiguous
 * growth, stable base, like the kernel), committed on demand. The 32-bit native binary is linked at
 * 0x08048000, so the heap cannot sit at 0x08000000: its base is wherever the host put it
 * (albw_os_heap_address()). The original state lived in the u32 struct at data 0x00714A30
 * {device addr, device size, heap size}; its only four users are replaced here, so the state is
 * native (pointer-sized) instead. */

#define APP_MEMORY_SIZE   0x04000000u /* APPMEMALLOC (0x1FF80040): 64 MiB, old-3DS application mode */
/* memory committed by the process besides heap/device memory, as svc 0x3A reports it: the code
 * image (text 0x593000 + rodata 0x6C000 + data 0x29000 + bss 0x88000 = 0x6A0000, page-rounded
 * sections of code.bin / the exheader) and the main-thread stack (exheader: 0x40000) */
#define IMAGE_AND_STACK   (0x006A0000u + 0x00040000u)
#define PAGE              0x1000u

/* nn::Result values the original functions return */
#define RESULT_SUCCESS          0
#define RESULT_MISALIGNED_SIZE  ((int32_t)0xE0E01BF2) /* literal in 0x00100DA4 / 0x00102304 */
#define RESULT_OUT_OF_MEMORY    ((int32_t)0xD86007F3) /* kernel OutOfMemory (ControlMemory) */

typedef struct {
    const char *name;
    uint8_t *base;      /* reservation start (NULL until first use) */
    uint32_t committed; /* committed bytes from base */
} Region;

static Region g_heap = {"heap", NULL, 0};          /* 3DS 0x08000000 */
static Region g_device = {"device memory", NULL, 0}; /* 3DS linear heap */
static pthread_mutex_t g_os_memory_lock = PTHREAD_MUTEX_INITIALIZER;

/* commit/decommit so that exactly [base, base+size) is usable; new pages read as zero */
static int32_t region_resize(Region *r, uint32_t size) {
    if (size > APP_MEMORY_SIZE) return RESULT_OUT_OF_MEMORY;
    if (!r->base) {   /* the reserved fixed-address area (runtime/pmem.c: persistent save states) */
        size_t n = 0;
        void *p = pmem_area(r == &g_heap ? PMEM_HEAP : PMEM_DEVICE, &n);
        if (!p || n < APP_MEMORY_SIZE) return RESULT_OUT_OF_MEMORY;
        r->base = p;
    }
    if (size > r->committed) {
        if (mprotect(r->base + r->committed, size - r->committed, PROT_READ | PROT_WRITE) != 0)
            return RESULT_OUT_OF_MEMORY;
    } else if (size < r->committed) {
        madvise(r->base + size, r->committed - size, MADV_DONTNEED); /* freed pages come back zeroed */
        mprotect(r->base + size, r->committed - size, PROT_NONE);
    }
    r->committed = size;
    return RESULT_SUCCESS;
}

/* Native accessors for code that used the fixed 3DS heap address 0x08000000 as a literal:
 * 0x00100F68 (framework startup) creates sead's "RootHeapMemory" ExpHeap at 0x08040000, i.e.
 * albw_os_heap_address() + 0x40000, right after the 0x40000-byte MemoryBlock the nn allocator got
 * from 0x00100E60. Its native decompilation must use this instead of the literal. */
void *albw_os_heap_address(void) { return g_heap.base; }
uint32_t albw_os_heap_size(void) { return g_heap.committed; }

/* USA 0x00100DA4 (EU same) nn::os heap setup: SetHeapSize(size).
 * void (u32 size). Signature: r0 only, no return value (disasm: result only goes to the panic
 * 0x0010A98C); called once, from 0x00100E60 with 0x60000. Original: size must be page aligned
 * (else Result 0xE0E01BF2 -> panic); ControlMemory COMMIT (grow, op 3) or FREE (shrink, op 1) of the
 * delta at 0x08000000 + current size; store the size; then MemoryBlock manager init over
 * [0x08000000, +size) (0x00101390, first call only) and 0x001013C4. Native: same steps on the host
 * reservation; failures are fatal as the panic was. Confidence: high. */
void func_00100DA4(uint32_t size) {
    pthread_mutex_lock(&g_os_memory_lock);
    int32_t r = (size & (PAGE - 1)) ? RESULT_MISALIGNED_SIZE : region_resize(&g_heap, size);
    pthread_mutex_unlock(&g_os_memory_lock);
    if (r < 0) runtime_fatal("nn::os SetHeapSize(0x%X) failed: result 0x%08X", size, (uint32_t)r);
    func_00101390(g_heap.base, g_heap.committed);
    func_001013C4();
}

/* USA 0x00102304 (EU same) nn::os::SetDeviceMemorySize(size) -> nn::Result.
 * int32 (u32 size). Signature from the disasm (r0 in, Result in r0) and its only caller 0x00100F68
 * (size = app memory - used - 0x60000, page-rounded; result ignored). Original rules kept:
 * size must be page aligned; growing from a non-zero size needs both sizes 1 MiB aligned (else
 * 0xE0E01BF2); growth is contiguous (ControlMemory op 0x10003 LINEAR|COMMIT at addr+cur), the
 * address is recorded when growing from 0; shrinking frees the tail and size 0 clears the address.
 * Native: host reservation instead of the linear heap. Confidence: high. */
int32_t func_00102304(uint32_t size) {
    int32_t r;
    pthread_mutex_lock(&g_os_memory_lock);
    uint32_t cur = g_device.committed;
    if (size & (PAGE - 1))
        r = RESULT_MISALIGNED_SIZE;
    else if (size > cur && cur != 0 && ((cur | size) & 0xFFFFFu) != 0)
        r = RESULT_MISALIGNED_SIZE;
    else
        r = region_resize(&g_device, size);
    pthread_mutex_unlock(&g_os_memory_lock);
    return r;
}

/* USA 0x0010EE40 (EU same) nn::os::GetDeviceMemoryAddress() -> address (0 when none).
 * Original: `ldr r0, =0x714A30; ldr r0, [r0]`. Callers: sead Arena::initialize (0x0010E0B8, the
 * RootHeap arena start), GPU/cache flush ranges, address range checks. (sdk_areas.json labels it
 * CriticalSection::unlock: that is wrong, the disasm is the getter.) Returns a real pointer.
 * Confidence: high. */
void *func_0010EE40(void) {
    return g_device.committed ? g_device.base : NULL;
}

/* USA 0x001022F4 (EU same) nn::os::GetDeviceMemorySize() -> u32.
 * Original: `ldr r0, [0x714A30 + 4]`. Callers pair it with 0x0010EE40 (flush/range checks) and
 * 0x00100F68 (HeapMgr arena size). Confidence: high. */
uint32_t func_001022F4(void) {
    return g_device.committed;
}

/* USA 0x00102290 (EU same) nn::os::GetAppMemorySize() -> u32.
 * Original: reads APPMEMALLOC from the kernel configuration page (0x1FF80000 + 0x40), which does
 * not exist natively. Returns the old-3DS application memory size. Only caller: 0x00100F68.
 * Confidence: high (meaning), medium (exact value: 64 MiB assumes the default memory mode). */
uint32_t func_00102290(void) {
    return APP_MEMORY_SIZE;
}

/* USA 0x001022A0 (EU same) nn::os::GetUsingMemorySize() -> u32.
 * Original: svc 0x38 GetResourceLimit(current process) (0x00103CA4), svc 0x3A
 * GetResourceLimitCurrentValues(&v, handle, {COMMIT}, 1), svc 0x23 CloseHandle; returns the low
 * word of v in r0 (Ghidra's undefined8 is r0 plus a stale r1). Native: what the kernel counts:
 * image + main stack + committed heap + device memory. Only caller: 0x00100F68, which sizes the
 * device memory as app memory - this - 0x60000 (= 0x3860000 here, as on hardware).
 * Confidence: medium (value modelled, not measured). */
uint32_t func_001022A0(void) {
    pthread_mutex_lock(&g_os_memory_lock);
    uint32_t used = IMAGE_AND_STACK + g_heap.committed + g_device.committed;
    pthread_mutex_unlock(&g_os_memory_lock);
    return used;
}

/* ================================================================== current heap (sead::HeapMgr) */

static void * ALBW_GP *current_heap_slot(const char *who) {
    void *thread_mgr = data_00715430;
    if (!thread_mgr)
        runtime_fatal("%s before sead::ThreadMgr exists (sead::HeapMgr::initialize not run yet)", who);
    void *thread = func_0010E294((const uint8_t *)thread_mgr + SEAD_THREADMGR_TLS_OFFSET);
    if (!thread)
        runtime_fatal("%s on a thread without a sead::Thread (TLS slot empty)", who);
    return (void * ALBW_GP *)((uint8_t *)thread + SEAD_THREAD_CURRENT_HEAP_OFFSET);
}

/* USA 0x0010DB90 (EU same) sead::HeapMgr::getCurrentHeap(this) -> Heap*.
 * `this` (r0) is ignored by the original: it reads ThreadMgr::sInstance->mThreadPtrTLS
 * (0x00715430 +0x24) via 0x0010E294 and returns Thread+0x68. Callers pass *0x00715210 or nothing.
 * The original faults when no sead::Thread exists; here that is a clear fatal. Confidence: high. */
sead_Heap *func_0010DB90(void *heap_mgr) {
    return *current_heap_slot("sead::HeapMgr::getCurrentHeap");
}

/* USA 0x0010983C (EU same) sead::HeapMgr::setCurrentHeap_(this, heap) -> previous Heap*.
 * Disasm: swap Thread+0x68 with r1, return the old value. 152 call sites (save/restore around
 * scoped allocations; callers compare the result with 1 as "no previous"). Confidence: high. */
sead_Heap *func_0010983C(void *heap_mgr, sead_Heap *heap) {
    void * ALBW_GP *slot = current_heap_slot("sead::HeapMgr::setCurrentHeap");
    sead_Heap *old = *slot;
    *slot = heap;
    return old;
}

/* ================================================================== operator new / delete */

/* USA 0x0010BAB4 (EU same) operator new(u32 size) (aliases _Znwj, _Znwj<NoThrow...>) and
 * USA 0x0010BA78 (EU same) operator new[](u32 size).
 * Disasm: heap = getCurrentHeap(*0x715210); if (!heap) return NULL; tail call
 * heap->vtbl[0x18](heap, size, 4). Not throwing: NULL on failure, callers test it.
 * The Ghidra versions are void (tail call result lost): replacement required. Confidence: high. */
void *func_0010BAB4(uint32_t size) {
    sead_Heap *heap = func_0010DB90(data_00715210);
    return heap ? heap_alloc(heap, size, 4) : NULL;
}

void *func_0010BA78(uint32_t size) {
    sead_Heap *heap = func_0010DB90(data_00715210);
    return heap ? heap_alloc(heap, size, 4) : NULL;
}

/* USA 0x0010CEE0 (EU same) operator new[](u32 size, Heap *heap, s32 alignment) (alias _ZnajP4Heapi)
 * and USA 0x0010CF28 (EU same) operator new(u32 size, Heap *heap, s32 alignment) (alias _ZnwjP4Heapi).
 * Disasm: NULL heap -> current heap (NULL -> return NULL); tail call vtbl[0x18](heap, size,
 * alignment). Negative alignment = allocate from the heap's tail (sead). Confidence: high. */
void *func_0010CEE0(uint32_t size, sead_Heap *heap, int32_t alignment) {
    if (!heap && !(heap = func_0010DB90(data_00715210))) return NULL;
    return heap_alloc(heap, size, alignment);
}

void *func_0010CF28(uint32_t size, sead_Heap *heap, int32_t alignment) {
    if (!heap && !(heap = func_0010DB90(data_00715210))) return NULL;
    return heap_alloc(heap, size, alignment);
}

/* USA 0x0058FAAC (EU 0x0058FA84) operator new(u32 size, s32 alignment) from the current heap.
 * Disasm: heap = getCurrentHeap(*0x715210); tail call vtbl[0x18](heap, size, alignment).
 * Confidence: high. */
void *func_0058FAAC(uint32_t size, int32_t alignment) {
    sead_Heap *heap = func_0010DB90(data_00715210);
    return heap ? heap_alloc(heap, size, alignment) : NULL;
}

/* USA 0x0058FAA0 (EU 0x0058FA78) same with an ignored first argument (`mov r0, r1; mov r1, r2`,
 * falls into 0x0058FAAC): (x, u32 size, s32 alignment). Confidence: high. */
void *func_0058FAA0(void *unused, uint32_t size, int32_t alignment) {
    return func_0058FAAC(size, alignment);
}

/* USA 0x0058FA08 (EU 0x0058F9E0) operator delete(void *p).
 * Disasm: if (!p) return; heap = HeapMgr::findContainHeap(*0x715210, p); if (heap) tail call
 * heap->vtbl[0x1C](heap, p). Pointers no heap contains are ignored, as on hardware.
 * Confidence: high. */
void func_0058FA08(void *p) {
    if (!p) return;
    sead_Heap *heap = func_0010E2A4(data_00715210, p);
    if (heap) heap_free(heap, p);
}

/* USA 0x0058FA00 (EU 0x0058F9D8) operator delete with the pointer in r1 (`mov r0, r1`, falls into
 * 0x0058FA08): (x, void *p). Confidence: high. */
void func_0058FA00(void *unused, void *p) {
    func_0058FA08(p);
}

/* USA 0x0058FA5C (EU 0x0058FA34) second operator delete copy (delete[]), same code as 0x0058FA08.
 * Confidence: high. */
void func_0058FA5C(void *p) {
    func_0058FA08(p);
}

/* USA 0x0058FA4C (EU 0x0058FA24) deleting destructor: p = dtor 0x0048FBB4(self), then falls into
 * the operator delete copy above (NULL check included). Kept with the delete family because it
 * shares that code. Confidence: high. */
void func_0058FA4C(void *self) {
    func_0058FA08(func_0048FBB4(self));
}

/* ================================================================== C runtime copies
 *
 * The game links its own ARM C library. These are mapped to the host libc. Return values: the
 * ARM block copies/fills advance r0 and return it (dst + n); no correct caller uses it (armcc
 * treats them as void __aeabi_* helpers), but Ghidra-decompiled callers sometimes pass r0 along,
 * so the same value is returned. memset (0x001164A8) preserves and returns dst. Sizes are the
 * 32-bit register values. */

/* a NULL string reaching these two is a converted caller that lost a store (BOOT_LOG 167): the 3DS never passes one.
 * Report it (the first few times, with the caller) and treat it as empty instead of crashing the game. */
static const char *null_string(const char *what, void *caller) {
    static int n;
    if (n < 8) {
        n++;
        fprintf(stderr, "albw: warning: %s of a NULL string (called from %p); please report this log\n", what, caller);
        backtrace_symbols_fd(&caller, 1, 2);
    }
    return "";
}

/* USA 0x00103CBC strlen(const char *s) -> u32 (word-at-a-time NUL scan). Confidence: high. */
uint32_t func_00103CBC(const char *s) {
    if (!s) s = null_string("strlen", __builtin_return_address(0));
    return (uint32_t)strlen(s);
}

/* USA 0x0010FBAC strcmp(a, b) -> int: byte difference (unsigned chars) at the first mismatch;
 * the word fast path gives the same sign. Confidence: high. */
int32_t func_0010FBAC(const char *a, const char *b) {
    if (!a) a = null_string("strcmp", __builtin_return_address(0));
    if (!b) b = null_string("strcmp", __builtin_return_address(0));
    const unsigned char *x = (const unsigned char *)a, *y = (const unsigned char *)b;
    while (*x && *x == *y) x++, y++;
    return (int32_t)*x - (int32_t)*y;
}

/* USA 0x0011629C strncmp(a, b, n) -> int. sdk_areas.json calls it memcmp, but the disasm stops at
 * a NUL byte (byte loop `cmp r3, #0`, word loop uqsub8 0x01010101 zero-byte test): strncmp.
 * Returns the byte difference (word path: same sign). Confidence: high. */
int32_t func_0011629C(const char *a, const char *b, uint32_t n) {
    const unsigned char *x = (const unsigned char *)a, *y = (const unsigned char *)b;
    for (; n; n--, x++, y++)
        if (!*x || *x != *y) return (int32_t)*x - (int32_t)*y;
    return 0;
}

/* USA 0x0010E3C4 memcpy(dst, src, n) (__aeabi_memcpy, 366 callers). Confidence: high. */
void *func_0010E3C4(void *dst, const void *src, uint32_t n) {
    memcpy(dst, src, n);
    return (uint8_t *)dst + n;
}

/* USA 0x00110D44 __aeabi_memcpy4(dst, src, n) (word-aligned; Ghidra's 4th argument is spurious).
 * Confidence: high. */
void *func_00110D44(void *dst, const void *src, uint32_t n) {
    memcpy(dst, src, n);
    return (uint8_t *)dst + n;
}

/* USA 0x001163A0 __aeabi_memcpy(dst, src, n) (second copy; Ghidra's 4th argument is spurious).
 * Confidence: high. */
void *func_001163A0(void *dst, const void *src, uint32_t n) {
    memcpy(dst, src, n);
    return (uint8_t *)dst + n;
}

/* USA 0x0010E74C memmove(dst, src, n) (forward via 0x0010E3C4 unless overlapping). Confidence: high. */
void *func_0010E74C(void *dst, const void *src, uint32_t n) {
    memmove(dst, src, n);
    return (uint8_t *)dst + n;
}

/* USA 0x00116428 __aeabi_memmove(dst, src, n) (second copy). Confidence: high. */
void *func_00116428(void *dst, const void *src, uint32_t n) {
    memmove(dst, src, n);
    return (uint8_t *)dst + n;
}

/* USA 0x0010E3B0 fixed copy: (obj, dst): memcpy(dst, obj + 0x10, 0x60) (`mov r3, r1; add r1, r0,
 * #0x10; mov r2, #0x60; mov r0, r3`, falls into 0x0010E3C4). One caller (0x004C3BB8).
 * Confidence: high. */
void *func_0010E3B0(const void *obj, void *dst) {
    return func_0010E3C4(dst, (const uint8_t *)obj + 0x10, 0x60);
}

/* USA 0x0011638C fixed copy: (src, dst): memcpy(dst, src, 0x70) (`mov r3, r1; mov r1, r0; mov r2,
 * #0x70; mov r0, r3`, falls into 0x001163A0). One caller (0x003CF2E8). Confidence: high. */
void *func_0011638C(const void *src, void *dst) {
    return func_001163A0(dst, src, 0x70);
}

/* USA 0x001164A8 memset(dst, c, n) -> dst (byte replicated, `push {r0, lr} ... pop {r0, pc}`).
 * Confidence: high. */
void *func_001164A8(void *dst, int32_t c, uint32_t n) {
    return memset(dst, c & 0xFF, n);
}

/* USA 0x001164C4 __aeabi_memset(dst, n, c) (note argument order). Confidence: high. */
void *func_001164C4(void *dst, uint32_t n, int32_t c) {
    memset(dst, c & 0xFF, n);
    return (uint8_t *)dst + n;
}

/* USA 0x001164F4 __aeabi_memclr(dst, n). Confidence: high. */
void *func_001164F4(void *dst, uint32_t n) {
    memset(dst, 0, n);
    return (uint8_t *)dst + n;
}

/* USA 0x00110490 __aeabi_memclr4(dst, n) (word aligned, 32 bytes per loop). Confidence: high. */
void *func_00110490(void *dst, uint32_t n) {
    memset(dst, 0, n);
    return (uint8_t *)dst + n;
}

/* save states (native/runtime/savestate.c): the two regions and resizing them back on load */
int albw_mem_ss_region(int i, uint8_t **base, uint32_t *committed) {
    Region *r = i == 0 ? &g_heap : i == 1 ? &g_device : NULL;
    if (!r) return -1;
    *base = r->base;
    *committed = r->committed;
    return 0;
}
int albw_mem_ss_resize(int i, uint32_t committed) {
    Region *r = i == 0 ? &g_heap : i == 1 ? &g_device : NULL;
    if (!r) return -1;
    pthread_mutex_lock(&g_os_memory_lock);
    int32_t res = r->committed == committed ? RESULT_SUCCESS : region_resize(r, committed);
    pthread_mutex_unlock(&g_os_memory_lock);
    return res == RESULT_SUCCESS ? 0 : -1;
}
