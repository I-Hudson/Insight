#pragma once

/**
* Compiler defines for different compilers.
*/

#if defined(__clang__)

#define DLLEXPORT __attribute__ ((__visibility__ ("default")))
#define DLLIMPORT
#define THREADLOCAL __thread
#define STDCALL __attribute__((stdcall))
#define CDECL __attribute__((cdecl))
#define RESTRICT __restrict__
#define INLINE inline
#define FORCE_INLINE inline
#define FORCE_NOINLINE __attribute__((noinline))
#define NO_EXPECT noexpect
#define NO_RETURN __attribute__((noreturn))
#define PACK_BEGIN()
#define PACK_END() __attribute__((__packed__))
#define ALIGN_BEGIN(_align)
#define ALIGN_END(_align) __attribute__( (aligned(_align) ) )
#define OFFSET_OF(X, Y) __builtin_offsetof(X, Y)
#define DEPRECATED

#pragma clang diagnostic ignored "-Wswitch"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Waddress-of-packed-member"
#pragma clang diagnostic ignored "-Wnull-dereference"
#pragma clang diagnostic ignored "-Winvalid-noreturn"

#elif defined(__GNUC__)

#define DLLEXPORT __attribute__ ((__visibility__ ("default")))
#define DLLIMPORT
#define THREADLOCAL __thread
#define STDCALL __attribute__((stdcall))
#define CDECL __attribute__((cdecl))
#define RESTRICT __restrict__
#define INLINE inline
#define FORCE_INLINE inline
#define FORCE_NOINLINE __attribute__((noinline))
#define NO_EXPECT noexpect
#define NO_RETURN __attribute__((noreturn))
#define PACK_BEGIN()
#define PACK_END() __attribute__((__packed__))
#define ALIGN_BEGIN(_align)
#define ALIGN_END(_align) __attribute__( (aligned(_align) ) )
#define OFFSET_OF(X, Y) __builtin_offsetof(X, Y)
#define DEPRECATED __attribute__((deprecated))

#elif defined(__INTEL_COMPILER)

#define DLLEXPORT ??
#define DLLIMPORT ??
#if _WIN32
#define THREADLOCAL __declspec(thread)
#else
#define THREADLOCAL __thread
#endif
#define STDCALL __stdcall
#define CDECL __cdecl
#define RESTRICT
#define INLINE inline
#define FORCE_INLINE inline
#define FORCE_NOINLINE
#define NO_EXPECT
#define NO_RETURN ??
#define PACK_BEGIN() ??
#define PACK_END() ??
#define ALIGN_BEGIN(_align) ??
#define ALIGN_END(_align) ??
#define OFFSET_OF(X, Y) ??
#define DEPRECATED ??
#define FUNCTION __FUNCTION__

#elif defined(_MSC_VER)

#if _MSC_VER < 1900
#error "Required Visual Studio 2015 or newer."
#endif

#ifndef COMPILER_DEFINES
#define COMPILER_DEFINES
#endif

#ifdef COMPILER_DEFINES
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#define THREADLOCAL __declspec(thread)
#define STDCALL __stdcall
#undef CDECL
#define CDECL __cdecl
#define RESTRICT __restrict
#define INLINE __inline
#define FORCE_INLINE __forceinline
#define FORCE_NOINLINE __declspec(noinline)
#define NO_EXPECT noexpect
#define NO_RETURN __declspec(noreturn)
#define PACK_BEGIN() __pragma(pack(push, 1))
#define PACK_END() ; __pragma(pack(pop))
#define ALIGN_BEGIN(_align) __declspec(align(_align))
#define ALIGN_END(_align)
#define OFFSET_OF(X, Y) offsetof(X, Y)
#define DEPRECATED __declspec(deprecated)
#define DEPRECATED_MSG(msg) [[deprecated(msg)]]
#define NO_DISCARD [[nodiscard]]
#define NO_VTABLE __declspec(novtable)
#define FUNCTION __FUNCSIG__
#endif

#pragma warning(disable: 4251)

#else

#pragma error "Unknown compiler."

#endif

#define PACK_STRUCT(TYPE) PACK_BEGIN() TYPE PACK_END()
