// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
#ifndef __GCENV_BASE_INCLUDED__
#define __GCENV_BASE_INCLUDED__
//
// Sets up basic environment for CLR GC
//

#ifdef _MSC_VER
#include <intrin.h>
#endif // _MSC_VER

#define FEATURE_REDHAWK 1

#define REDHAWK_PALIMPORT extern "C"
#define REDHAWK_PALAPI __stdcall

#ifndef _MSC_VER
#define __stdcall
#ifdef __clang__
#define __forceinline __attribute__((always_inline)) inline
#else // __clang__
#define __forceinline inline
#endif // __clang__
// [LOCALGC TODO] is there a better place for this?
#define NOINLINE __attribute__((noinline))
#else // !_MSC_VER
#define NOINLINE __declspec(noinline)
#endif // _MSC_VER

#ifndef SIZE_T_MAX
#define SIZE_T_MAX ((size_t)-1)
#endif
#ifndef SSIZE_T_MAX
#define SSIZE_T_MAX ((ptrdiff_t)(SIZE_T_MAX / 2))
#endif

#ifndef _INC_WINDOWS
// -----------------------------------------------------------------------------------------------------------
//
// Aliases for Win32 types
//

typedef int BOOL;
typedef uint32_t DWORD;
typedef uint64_t DWORD64;

// -----------------------------------------------------------------------------------------------------------
// HRESULT subset.

#ifdef PLATFORM_UNIX
typedef int32_t HRESULT;
#else
// this must exactly match the typedef used by windows.h
typedef long HRESULT;
#endif

#define SUCCEEDED(_hr)          ((HRESULT)(_hr) >= 0)
#define FAILED(_hr)             ((HRESULT)(_hr) < 0)

inline HRESULT HRESULT_FROM_WIN32(unsigned long x)
{
    return (HRESULT)(x) <= 0 ? (HRESULT)(x) : (HRESULT) (((x) & 0x0000FFFF) | (7 << 16) | 0x80000000);
}

#define S_OK                    0x0
#define E_FAIL                  0x80004005
#define E_OUTOFMEMORY           0x8007000E
#define COR_E_EXECUTIONENGINE   0x80131506

#define NOERROR                 0x0
#define ERROR_TIMEOUT           1460

#define TRUE true
#define FALSE false

#define CALLBACK __stdcall
#define FORCEINLINE __forceinline

#define INFINITE 0xFFFFFFFF

#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))

#ifndef _countof
#define _countof(_array) (sizeof(_array)/sizeof(_array[0]))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define C_ASSERT(cond) static_assert( cond, #cond )

#define UNREFERENCED_PARAMETER(P)          (void)(P)

#ifdef PLATFORM_UNIX
#define _vsnprintf_s(string, sizeInBytes, count, format, args) vsnprintf(string, sizeInBytes, format, args)
#define sprintf_s snprintf
#define swprintf_s swprintf
#define _snprintf_s(string, sizeInBytes, count, format, ...) \
  snprintf(string, sizeInBytes, format, ## __VA_ARGS__)
#endif

#ifdef UNICODE
#define _tcslen wcslen
#define _tcscpy wcscpy
#define _stprintf_s swprintf_s
#define _tfopen _wfopen
#else
#define _tcslen strlen
#define _tcscpy strcpy
#define _stprintf_s sprintf_s
#define _tfopen fopen
#endif

#define WINAPI __stdcall

typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)(void* lpThreadParameter);

#define WAIT_OBJECT_0           0
#define WAIT_TIMEOUT            258
#define WAIT_FAILED             0xFFFFFFFF

#if defined(_MSC_VER) 
 #if defined(_ARM_)

  __forceinline void YieldProcessor() { }
  extern "C" void __emit(const unsigned __int32 opcode);
  #pragma intrinsic(__emit)
  #define MemoryBarrier() { __emit(0xF3BF); __emit(0x8F5F); }

 #elif defined(_ARM64_)

  extern "C" void __yield(void);
  #pragma intrinsic(__yield)
  __forceinline void YieldProcessor() { __yield();}

  extern "C" void __dmb(const unsigned __int32 _Type);
  #pragma intrinsic(__dmb)
  #define MemoryBarrier() { __dmb(_ARM64_BARRIER_SY); }

 #elif defined(_AMD64_)
  
  extern "C" void
  _mm_pause (
      void
      );
  
  extern "C" void
  _mm_mfence (
      void
      );

  #pragma intrinsic(_mm_pause)
  #pragma intrinsic(_mm_mfence)
  
  #define YieldProcessor _mm_pause
  #define MemoryBarrier _mm_mfence

 #elif defined(_X86_)
  
  #define YieldProcessor() __asm { rep nop }
  #define MemoryBarrier() MemoryBarrierImpl()
  __forceinline void MemoryBarrierImpl()
  {
      int32_t Barrier;
      __asm {
          xchg Barrier, eax
      }
  }

 #else // !_ARM_ && !_AMD64_ && !_X86_
  #error Unsupported architecture
 #endif
#else // _MSC_VER

// Only clang defines __has_builtin, so we first test for a GCC define
// before using __has_builtin.

#if defined(__i386__) || defined(__x86_64__)

#if (__GNUC__ > 4 && __GNUC_MINOR > 7) || __has_builtin(__builtin_ia32_pause)
 // clang added this intrinsic in 3.8
 // gcc added this intrinsic by 4.7.1
 #define YieldProcessor __builtin_ia32_pause
#endif // __has_builtin(__builtin_ia32_pause)

#if defined(__GNUC__) || __has_builtin(__builtin_ia32_mfence)
 // clang has had this intrinsic since at least 3.0
 // gcc has had this intrinsic since forever
 #define MemoryBarrier __builtin_ia32_mfence
#endif // __has_builtin(__builtin_ia32_mfence)

// If we don't have intrinsics, we can do some inline asm instead.
#ifndef YieldProcessor
 #define YieldProcessor() asm volatile ("pause")
#endif // YieldProcessor

#ifndef MemoryBarrier
 #define MemoryBarrier() asm volatile ("mfence")
#endif // MemoryBarrier

#endif // defined(__i386__) || defined(__x86_64__)

#endif // _MSC_VER

#ifdef _MSC_VER
#pragma intrinsic(_BitScanForward)
#if WIN64
 #pragma intrinsic(_BitScanForward64)
#endif
#endif // _MSC_VER

// Cross-platform wrapper for the _BitScanForward compiler intrinsic.
inline uint8_t BitScanForward(uint32_t *bitIndex, uint32_t mask)
{
#ifdef _MSC_VER
    return _BitScanForward((unsigned long*)bitIndex, mask);
#else // _MSC_VER
    unsigned char ret = FALSE;
    int iIndex = __builtin_ffsl(mask);
    if (iIndex != 0)
    {
        *bitIndex = (uint32_t)(iIndex - 1);
        ret = TRUE;
    }

    return ret;
#endif // _MSC_VER
}

// Cross-platform wrapper for the _BitScanForward64 compiler intrinsic.
inline uint8_t BitScanForward64(uint32_t *bitIndex, uint64_t mask)
{
#ifdef _MSC_VER
 #if _WIN32
    // MSVC targeting a 32-bit target does not support this intrinsic.
    // We can fake it using two successive invocations of _BitScanForward.
    uint32_t hi = (mask >> 32) & 0xFFFFFFFF;
    uint32_t lo = mask & 0xFFFFFFFF;
    uint32_t fakeBitIndex = 0;
    if (BitScanForward(&fakeBitIndex, hi))
    {
        *bitIndex = fakeBitIndex + 32;
    }

    return BitScanForward(bitIndex, lo);
 #else
    return _BitScanForward64((unsigned long*)bitIndex, mask);
 #endif // _WIN32
#else
    unsigned char ret = FALSE;
    int iIndex = __builtin_ffsll(mask);
    if (iIndex != 0)
    {
        *bitIndex = (uint32_t)(iIndex - 1);
        ret = TRUE;
    }

    return ret;
#endif // _MSC_VER
}

// Aligns a size_t to the specified alignment. Alignment must be a power
// of two.
inline size_t ALIGN_UP(size_t val, size_t alignment)
{
    // alignment factor must be power of two
    assert((alignment & (alignment - 1)) == 0);
    size_t result = (val + (alignment - 1)) & ~(alignment - 1);
    assert(result >= val);
    return result;
}

// Aligns a pointer to the specified alignment. Alignment must be a power
// of two.
inline uint8_t* ALIGN_UP(uint8_t* ptr, size_t alignment)
{
    size_t as_size_t = reinterpret_cast<size_t>(ptr);
    return reinterpret_cast<uint8_t*>(ALIGN_UP(as_size_t, alignment));
}

// Aligns a size_t to the specified alignment by rounding down. Alignment must
// be a power of two.
inline size_t ALIGN_DOWN(size_t val, size_t alignment)
{
    // alignment factor must be power of two.
    assert((alignment & (alignment - 1)) == 0);
    size_t result = val & ~(alignment - 1);
    return result;
}

// Aligns a pointer to the specified alignment by rounding down. Alignment
// must be a power of two.
inline uint8_t* ALIGN_DOWN(uint8_t* ptr, size_t alignment)
{
    size_t as_size_t = reinterpret_cast<size_t>(ptr);
    return reinterpret_cast<uint8_t*>(ALIGN_DOWN(as_size_t, alignment));
}

// Aligns a void pointer to the specified alignment by rounding down. Alignment
// must be a power of two.
inline void* ALIGN_DOWN(void* ptr, size_t alignment)
{
    size_t as_size_t = reinterpret_cast<size_t>(ptr);
    return reinterpret_cast<void*>(ALIGN_DOWN(as_size_t, alignment));
}

typedef struct _PROCESSOR_NUMBER {
    uint16_t Group;
    uint8_t Number;
    uint8_t Reserved;
} PROCESSOR_NUMBER, *PPROCESSOR_NUMBER;

#endif // _INC_WINDOWS

// -----------------------------------------------------------------------------------------------------------
//
// The subset of the contract code required by the GC/HandleTable sources. If Redhawk moves to support
// contracts these local definitions will disappear and be replaced by real implementations.
//

#define LEAF_CONTRACT
#define LIMITED_METHOD_CONTRACT
#define LIMITED_METHOD_DAC_CONTRACT
#define WRAPPER_CONTRACT
#define WRAPPER_NO_CONTRACT
#define STATIC_CONTRACT_LEAF
#define STATIC_CONTRACT_DEBUG_ONLY
#define STATIC_CONTRACT_NOTHROW
#define STATIC_CONTRACT_CAN_TAKE_LOCK
#define STATIC_CONTRACT_SO_TOLERANT
#define STATIC_CONTRACT_GC_NOTRIGGER
#define STATIC_CONTRACT_MODE_COOPERATIVE
#define CONTRACTL
#define CONTRACT(_expr)
#define CONTRACT_VOID
#define THROWS
#define NOTHROW
#define INSTANCE_CHECK
#define MODE_COOPERATIVE
#define MODE_ANY
#define SO_INTOLERANT
#define SO_TOLERANT
#define GC_TRIGGERS
#define GC_NOTRIGGER
#define CAN_TAKE_LOCK
#define SUPPORTS_DAC
#define FORBID_FAULT
#define CONTRACTL_END
#define CONTRACT_END
#define TRIGGERSGC()
#define WRAPPER(_contract)
#define DISABLED(_contract)
#define INJECT_FAULT(_expr)
#define INJECTFAULT_HANDLETABLE 0x1
#define INJECTFAULT_GCHEAP 0x2
#define FAULT_NOT_FATAL()
#define BEGIN_DEBUG_ONLY_CODE
#define END_DEBUG_ONLY_CODE
#define BEGIN_GETTHREAD_ALLOWED
#define END_GETTHREAD_ALLOWED
#define LEAF_DAC_CONTRACT
#define PRECONDITION(_expr)
#define POSTCONDITION(_expr)
#define RETURN return
#define CONDITIONAL_CONTRACT_VIOLATION(_violation, _expr)

// -----------------------------------------------------------------------------------------------------------
//
// Data access macros
//
typedef uintptr_t TADDR;
#define PTR_TO_TADDR(ptr) ((TADDR)(ptr))

#define DPTR(type) type*
#define SPTR(type) type*
typedef DPTR(size_t)    PTR_size_t;
typedef DPTR(uint8_t)   PTR_uint8_t;

// -----------------------------------------------------------------------------------------------------------

#define DATA_ALIGNMENT sizeof(uintptr_t)
#define RAW_KEYWORD(x) x
#define DECLSPEC_ALIGN(x)   __declspec(align(x))
#ifndef _ASSERTE
#define _ASSERTE(_expr) ASSERT(_expr)
#endif
#define CONSISTENCY_CHECK(_expr) ASSERT(_expr)
#define PREFIX_ASSUME(cond) ASSERT(cond)
#define EEPOLICY_HANDLE_FATAL_ERROR(error) ASSERT(!"EEPOLICY_HANDLE_FATAL_ERROR")
#define UI64(_literal) _literal##ULL

class ObjHeader;
class MethodTable;
class Object;
class ArrayBase;

// Various types used to refer to object references or handles. This will get more complex if we decide
// Redhawk wants to wrap object references in the debug build.
typedef DPTR(Object) PTR_Object;
typedef DPTR(PTR_Object) PTR_PTR_Object;

typedef PTR_Object OBJECTREF;
typedef PTR_PTR_Object PTR_OBJECTREF;
typedef PTR_Object _UNCHECKED_OBJECTREF;
typedef PTR_PTR_Object PTR_UNCHECKED_OBJECTREF;

// With no object reference wrapping the following macros are very simple.
#define ObjectToOBJECTREF(_obj) (OBJECTREF)(_obj)
#define OBJECTREFToObject(_obj) (Object*)(_obj)

#define VALIDATEOBJECTREF(_objref) (void)_objref;

#define VOLATILE(T) T volatile

//
// This code is extremely compiler- and CPU-specific, and will need to be altered to 
// support new compilers and/or CPUs.  Here we enforce that we can only compile using
// VC++, or Clang on x86, AMD64, ARM and ARM64.
// 
#if !defined(_MSC_VER) && !defined(__clang__)
#error The Volatile type is currently only defined for Visual C++ and Clang
#endif

#if defined(__clang__) && !defined(_X86_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && !defined(_WASM_)
#error The Volatile type is currently only defined for Clang when targeting x86, AMD64, ARM, ARM64 or WASM
#endif

#if defined(__clang__)
#if defined(_ARM_) || defined(_ARM64_)
// This is functionally equivalent to the MemoryBarrier() macro used on ARM on Windows.
#define VOLATILE_MEMORY_BARRIER() asm volatile ("dmb ish" : : : "memory")
#else
//
// For Clang, we prevent reordering by the compiler by inserting the following after a volatile
// load (to prevent subsequent operations from moving before the read), and before a volatile 
// write (to prevent prior operations from moving past the write).  We don't need to do anything
// special to prevent CPU reorderings, because the x86 and AMD64 architectures are already
// sufficiently constrained for our purposes.  If we ever need to run on weaker CPU architectures
// (such as PowerPC), then we will need to do more work.
// 
// Please do not use this macro outside of this file.  It is subject to change or removal without
// notice.
//
#define VOLATILE_MEMORY_BARRIER() asm volatile ("" : : : "memory")
#endif // !_ARM_
#elif defined(_ARM_) && _ISO_VOLATILE
// ARM has a very weak memory model and very few tools to control that model. We're forced to perform a full
// memory barrier to preserve the volatile semantics. Technically this is only necessary on MP systems but we
// currently don't have a cheap way to determine the number of CPUs from this header file. Revisit this if it
// turns out to be a performance issue for the uni-proc case.
#define VOLATILE_MEMORY_BARRIER() MemoryBarrier()
#else
//
// On VC++, reorderings at the compiler and machine level are prevented by the use of the 
// "volatile" keyword in VolatileLoad and VolatileStore.  This should work on any CPU architecture
// targeted by VC++ with /iso_volatile-.
//
#define VOLATILE_MEMORY_BARRIER()
#endif

//
// VolatileLoad loads a T from a pointer to T.  It is guaranteed that this load will not be optimized
// away by the compiler, and that any operation that occurs after this load, in program order, will
// not be moved before this load.  In general it is not guaranteed that the load will be atomic, though
// this is the case for most aligned scalar data types.  If you need atomic loads or stores, you need
// to consult the compiler and CPU manuals to find which circumstances allow atomicity.
//
template<typename T>
inline
T VolatileLoad(T const * pt)
{
#if defined(_ARM64_) && defined(__clang__)
    T val;
    static const unsigned lockFreeAtomicSizeMask = (1 << 1) | (1 << 2) | (1 << 4) | (1 << 8);
    if((1 << sizeof(T)) & lockFreeAtomicSizeMask)
    {
        __atomic_load((T volatile const *)pt, &val, __ATOMIC_ACQUIRE);
    }
    else
    {
        val = *(T volatile const *)pt;
        asm volatile ("dmb ishld" : : : "memory");
    }
#else
    T val = *(T volatile const *)pt;
    VOLATILE_MEMORY_BARRIER();
#endif
    return val;
}

template<typename T>
inline
T VolatileLoadWithoutBarrier(T const * pt)
{
#ifndef DACCESS_COMPILE
    T val = *(T volatile const *)pt;
#else
    T val = *pt;
#endif
    return val;
}

//
// VolatileStore stores a T into the target of a pointer to T.  Is is guaranteed that this store will
// not be optimized away by the compiler, and that any operation that occurs before this store, in program
// order, will not be moved after this store.  In general, it is not guaranteed that the store will be
// atomic, though this is the case for most aligned scalar data types.  If you need atomic loads or stores,
// you need to consult the compiler and CPU manuals to find which circumstances allow atomicity.
//
template<typename T>
inline
void VolatileStore(T* pt, T val)
{
#if defined(_ARM64_) && defined(__clang__)
    static const unsigned lockFreeAtomicSizeMask = (1 << 1) | (1 << 2) | (1 << 4) | (1 << 8);
    if((1 << sizeof(T)) & lockFreeAtomicSizeMask)
    {
        __atomic_store((T volatile *)pt, &val, __ATOMIC_RELEASE);
    }
    else
    {
        VOLATILE_MEMORY_BARRIER();
        *(T volatile *)pt = val;
    }
#else
    VOLATILE_MEMORY_BARRIER();
    *(T volatile *)pt = val;
#endif
}

class Thread;

inline bool IsGCSpecialThread()
{
    // [LOCALGC TODO] this is not correct
    return false;
}

inline bool dbgOnly_IsSpecialEEThread()
{
    return false;
}

#define ClrFlsSetThreadType(type)

//
// Performance logging
//

#define COUNTER_ONLY(x)

//#include "etmdummy.h"
//#define ETW_EVENT_ENABLED(e,f) false

namespace ETW
{
    typedef  enum _GC_ROOT_KIND {
        GC_ROOT_STACK = 0,
        GC_ROOT_FQ = 1,
        GC_ROOT_HANDLES = 2,
        GC_ROOT_OLDER = 3,
        GC_ROOT_SIZEDREF = 4,
        GC_ROOT_OVERFLOW = 5
    } GC_ROOT_KIND;
};

inline bool IsGCThread()
{
    // [LOCALGC TODO] this is not correct
    return false;
}

inline bool FitsInU1(uint64_t val)
{
    return val == (uint64_t)(uint8_t)val;
}

// -----------------------------------------------------------------------------------------------------------
//
// AppDomain emulation. The we don't have these in Redhawk so instead we emulate the bare minimum of the API
// touched by the GC/HandleTable and pretend we have precisely one (default) appdomain.
//

#define RH_DEFAULT_DOMAIN_ID 1

struct ADIndex
{
    DWORD m_dwIndex;

    ADIndex () : m_dwIndex(RH_DEFAULT_DOMAIN_ID) {}
    explicit ADIndex (DWORD id) : m_dwIndex(id) {}
    BOOL operator==(const ADIndex& ad) const { return m_dwIndex == ad.m_dwIndex; }
    BOOL operator!=(const ADIndex& ad) const { return m_dwIndex != ad.m_dwIndex; }
};

class AppDomain
{
public:
    ADIndex GetIndex() { return ADIndex(RH_DEFAULT_DOMAIN_ID); }
    BOOL IsRudeUnload() { return FALSE; }
    BOOL NoAccessToHandleTable() { return FALSE; }
    void DecNumSizedRefHandles() {}
};

class SystemDomain
{
public:
    static SystemDomain *System() { return NULL; }
    static AppDomain *GetAppDomainAtIndex(ADIndex /*index*/) { return (AppDomain *)-1; }
    static AppDomain *AppDomainBeingUnloaded() { return NULL; }
    AppDomain *DefaultDomain() { return NULL; }
    DWORD GetTotalNumSizedRefHandles() { return 0; }
};

class NumaNodeInfo
{
public:
    static bool CanEnableGCNumaAware()
    {
        // [LOCALGC TODO] enable NUMA node support
        return false;
    }

    static void GetGroupForProcessor(uint16_t processor_number, uint16_t * group_number, uint16_t * group_processor_number)
    {
        // [LOCALGC TODO] enable NUMA node support
        assert(!"should not be called");
    }

    static bool GetNumaProcessorNodeEx(PPROCESSOR_NUMBER proc_no, uint16_t * node_no)
    {
        // [LOCALGC TODO] enable NUMA node support
        assert(!"should not be called");
        return false;
    }
};

class CPUGroupInfo
{
public:
    static bool CanEnableGCCPUGroups()
    {
        // [LOCALGC TODO] enable CPU group support
        return false;
    }

    static uint32_t GetNumActiveProcessors()
    {
        // [LOCALGC TODO] enable CPU group support
        assert(!"should not be called");
        return 0;
    }

    static void GetGroupForProcessor(uint16_t processor_number, uint16_t * group_number, uint16_t * group_processor_number)
    {
        // [LOCALGC TODO] enable CPU group support
        assert(!"should not be called");
    }
};


#endif // __GCENV_BASE_INCLUDED__
