#ifndef PTHREAD_WRAPPER_H_
#define PTHREAD_WRAPPER_H_

#ifndef FOLLY_NO_CONFIG
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef __APPLE__
#include <TargetConditionals.h> // @manual
#endif

#if !defined(FOLLY_MOBILE)
#if defined(__ANDROID__) || \
			(defined(__APPLE__) &&  \
			 (TARGET_IPHONE_SIMULATOR || TARGET_OS_SIMULATOR || TARGET_OS_IPHONE))
#define FOLLY_MOBILE 1
#else
#define FOLLY_MOBILE 0
#endif
#endif // FOLLY_MOBILE

/* #undef FOLLY_HAVE_PTHREAD */
/* #undef FOLLY_HAVE_PTHREAD_ATFORK */

#define FOLLY_HAVE_LIBGFLAGS 1
/* #undef FOLLY_UNUSUAL_GFLAGS_NAMESPACE */
#define FOLLY_GFLAGS_NAMESPACE gflags

#define FOLLY_HAVE_LIBGLOG 1

/* #undef FOLLY_USE_JEMALLOC */

#if __has_include(<features.h>)
#include <features.h>
#endif

/* #undef FOLLY_HAVE_ACCEPT4 */
#define FOLLY_HAVE_GETRANDOM 0
/* #undef FOLLY_HAVE_PREADV */
/* #undef FOLLY_HAVE_PWRITEV */
/* #undef FOLLY_HAVE_CLOCK_GETTIME */
/* #undef FOLLY_HAVE_PIPE2 */
/* #undef FOLLY_HAVE_SENDMMSG */
/* #undef FOLLY_HAVE_RECVMMSG */
#define FOLLY_HAVE_OPENSSL_ASN1_TIME_DIFF 1

/* #undef FOLLY_HAVE_IFUNC */
#define FOLLY_HAVE_STD__IS_TRIVIALLY_COPYABLE 1
#define FOLLY_HAVE_UNALIGNED_ACCESS 1
/* #undef FOLLY_HAVE_VLA */
#define FOLLY_HAVE_WEAK_SYMBOLS 0
/* #undef FOLLY_HAVE_LINUX_VDSO */
/* #undef FOLLY_HAVE_MALLOC_USABLE_SIZE */
/* #undef FOLLY_HAVE_INT128_T */
#define FOLLY_HAVE_WCHAR_SUPPORT 1
/* #undef FOLLY_HAVE_EXTRANDOM_SFMT19937 */
/* #undef HAVE_VSNPRINTF_ERRORS */

/* #undef FOLLY_HAVE_LIBUNWIND */
/* #undef FOLLY_HAVE_DWARF */
/* #undef FOLLY_HAVE_ELF */
/* #undef FOLLY_HAVE_SWAPCONTEXT */
/* #undef FOLLY_HAVE_BACKTRACE */
/* #undef FOLLY_USE_SYMBOLIZER */
#define FOLLY_DEMANGLE_MAX_SYMBOL_SIZE 1024

/* #undef FOLLY_HAVE_SHADOW_LOCAL_WARNINGS */

/* #undef FOLLY_HAVE_LIBLZ4 */
/* #undef FOLLY_HAVE_LIBLZMA */
/* #undef FOLLY_HAVE_LIBSNAPPY */
#define FOLLY_HAVE_LIBZ 1
/* #undef FOLLY_HAVE_LIBZSTD */
/* #undef FOLLY_HAVE_LIBBZ2 */

#define FOLLY_LIBRARY_SANITIZE_ADDRESS 0

/* #undef FOLLY_SUPPORT_SHARED_LIBRARY */

#define FOLLY_HAVE_LIBRT 0

#endif

#if __has_include(<features.h>)
#include <features.h> // @manual
#endif

#if __has_include(<bits/c++config.h>)
#include <bits/c++config.h> // @manual
#endif

#if __has_include(<__config>)
#include <__config> // @manual
#endif

#ifdef __ANDROID__
#include <android/api-level.h> // @manual
#endif

#ifdef __APPLE__
#include <Availability.h> // @manual
#include <AvailabilityMacros.h> // @manual
#include <TargetConditionals.h> // @manual
#endif

/**
 * Portable version check.
 */
#ifndef __GNUC_PREREQ
#if defined __GNUC__ && defined __GNUC_MINOR__
 /* nolint */
#define __GNUC_PREREQ(maj, min) \
		  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
/* nolint */
#define __GNUC_PREREQ(maj, min) 0
#endif
#endif

// portable version check for clang
#ifndef __CLANG_PREREQ
#if defined __clang__ && defined __clang_major__ && defined __clang_minor__
	/* nolint */
#define __CLANG_PREREQ(maj, min) \
		  ((__clang_major__ << 16) + __clang_minor__ >= ((maj) << 16) + (min))
#else
/* nolint */
#define __CLANG_PREREQ(maj, min) 0
#endif
#endif

#if defined(__has_builtin)
#define FOLLY_HAS_BUILTIN(...) __has_builtin(__VA_ARGS__)
#else
#define FOLLY_HAS_BUILTIN(...) 0
#endif

#if defined(__has_feature)
#define FOLLY_HAS_FEATURE(...) __has_feature(__VA_ARGS__)
#else
#define FOLLY_HAS_FEATURE(...) 0
#endif

/* FOLLY_SANITIZE_ADDRESS is defined to 1 if the current compilation unit
 * is being compiled with ASAN enabled.
 *
 * Beware when using this macro in a header file: this macro may change values
 * across compilation units if some libraries are built with ASAN enabled
 * and some built with ASAN disabled.  For instance, this may occur, if folly
 * itself was compiled without ASAN but a downstream project that uses folly is
 * compiling with ASAN enabled.
 *
 * Use FOLLY_LIBRARY_SANITIZE_ADDRESS (defined in folly-config.h) to check if
 * folly itself was compiled with ASAN enabled.
 */
#ifndef FOLLY_SANITIZE_ADDRESS
#if FOLLY_HAS_FEATURE(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define FOLLY_SANITIZE_ADDRESS 1
#endif
#endif

/* Define attribute wrapper for function attribute used to disable
 * address sanitizer instrumentation. Unfortunately, this attribute
 * has issues when inlining is used, so disable that as well. */
#ifdef FOLLY_SANITIZE_ADDRESS
#if defined(__clang__)
#if __has_attribute(__no_sanitize__)
#define FOLLY_DISABLE_ADDRESS_SANITIZER \
  __attribute__((__no_sanitize__("address"), __noinline__))
#elif __has_attribute(__no_address_safety_analysis__)
#define FOLLY_DISABLE_ADDRESS_SANITIZER \
  __attribute__((__no_address_safety_analysis__, __noinline__))
#elif __has_attribute(__no_sanitize_address__)
#define FOLLY_DISABLE_ADDRESS_SANITIZER \
  __attribute__((__no_sanitize_address__, __noinline__))
#endif
#elif defined(__GNUC__)
#define FOLLY_DISABLE_ADDRESS_SANITIZER \
  __attribute__((__no_address_safety_analysis__, __noinline__))
#endif
#endif
#ifndef FOLLY_DISABLE_ADDRESS_SANITIZER
#define FOLLY_DISABLE_ADDRESS_SANITIZER
#endif

/* Define a convenience macro to test when thread sanitizer is being used
 * across the different compilers (e.g. clang, gcc) */
#ifndef FOLLY_SANITIZE_THREAD
#if FOLLY_HAS_FEATURE(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#define FOLLY_SANITIZE_THREAD 1
#endif
#endif

#ifdef FOLLY_SANITIZE_THREAD
#define FOLLY_DISABLE_THREAD_SANITIZER \
  __attribute__((no_sanitize_thread, noinline))
#else
#define FOLLY_DISABLE_THREAD_SANITIZER
#endif

/**
 * Define a convenience macro to test when memory sanitizer is being used
 * across the different compilers (e.g. clang, gcc)
 */
#ifndef FOLLY_SANITIZE_MEMORY
#if FOLLY_HAS_FEATURE(memory_sanitizer) || defined(__SANITIZE_MEMORY__)
#define FOLLY_SANITIZE_MEMORY 1
#endif
#endif

#ifdef FOLLY_SANITIZE_MEMORY
#define FOLLY_DISABLE_MEMORY_SANITIZER \
  __attribute__((no_sanitize_memory, noinline))
#else
#define FOLLY_DISABLE_MEMORY_SANITIZER
#endif

/**
 * Define a convenience macro to test when dataflow sanitizer is being used
 * across the different compilers (e.g. clang, gcc)
 */
#ifndef FOLLY_SANITIZE_DATAFLOW
#if FOLLY_HAS_FEATURE(dataflow_sanitizer) || defined(__SANITIZE_DATAFLOW__)
#define FOLLY_SANITIZE_DATAFLOW 1
#endif
#endif

#ifdef FOLLY_SANITIZE_DATAFLOW
#define FOLLY_DISABLE_DATAFLOW_SANITIZER \
  __attribute__((no_sanitize_dataflow, noinline))
#else
#define FOLLY_DISABLE_DATAFLOW_SANITIZER
#endif

/**
 * Define a convenience macro to test when ASAN, UBSAN, TSAN or MSAN sanitizer
 * are being used
 */
#ifndef FOLLY_SANITIZE
#if defined(FOLLY_SANITIZE_ADDRESS) || defined(FOLLY_SANITIZE_THREAD) || \
    defined(FOLLY_SANITIZE_MEMORY) || defined(FOLLY_SANITIZE_DATAFLOW)
#define FOLLY_SANITIZE 1
#endif
#endif

#ifdef FOLLY_SANITIZE
#define FOLLY_DISABLE_UNDEFINED_BEHAVIOR_SANITIZER(...) \
  __attribute__((no_sanitize(__VA_ARGS__)))
#else
#define FOLLY_DISABLE_UNDEFINED_BEHAVIOR_SANITIZER(...)
#endif // FOLLY_SANITIZE

#define FOLLY_DISABLE_SANITIZERS  \
  FOLLY_DISABLE_ADDRESS_SANITIZER \
  FOLLY_DISABLE_THREAD_SANITIZER  \
  FOLLY_DISABLE_MEMORY_SANITIZER  \
  FOLLY_DISABLE_UNDEFINED_BEHAVIOR_SANITIZER("undefined")

/**
 * Macro for marking functions as having public visibility.
 */
#if defined(__GNUC__)
#define FOLLY_EXPORT __attribute__((__visibility__("default")))
#else
#define FOLLY_EXPORT
#endif

// noinline
#ifdef _MSC_VER
#define FOLLY_NOINLINE __declspec(noinline)
#elif defined(__HIP_PLATFORM_HCC__)
// HIP software stack defines its own __noinline__ macro.
#define FOLLY_NOINLINE __attribute__((noinline))
#elif defined(__GNUC__)
#define FOLLY_NOINLINE __attribute__((__noinline__))
#else
#define FOLLY_NOINLINE
#endif

// always inline
#ifdef _MSC_VER
#define FOLLY_ALWAYS_INLINE __forceinline
#elif defined(__GNUC__)
#define FOLLY_ALWAYS_INLINE inline __attribute__((__always_inline__))
#else
#define FOLLY_ALWAYS_INLINE inline
#endif

// attribute hidden
#if defined(_MSC_VER)
#define FOLLY_ATTR_VISIBILITY_HIDDEN
#elif defined(__GNUC__)
#define FOLLY_ATTR_VISIBILITY_HIDDEN __attribute__((__visibility__("hidden")))
#else
#define FOLLY_ATTR_VISIBILITY_HIDDEN
#endif

// An attribute for marking symbols as weak, if supported
#if FOLLY_HAVE_WEAK_SYMBOLS
#define FOLLY_ATTR_WEAK __attribute__((__weak__))
#else
#define FOLLY_ATTR_WEAK
#endif

// Microsoft ABI version (can be overridden manually if necessary)
#ifndef FOLLY_MICROSOFT_ABI_VER
#ifdef _MSC_VER
#define FOLLY_MICROSOFT_ABI_VER _MSC_VER
#endif
#endif

//  FOLLY_ERASE
//
//  A conceptual attribute/syntax combo for erasing a function from the build
//  artifacts and forcing all call-sites to inline the callee, at least as far
//  as each compiler supports.
//
//  Semantically includes the inline specifier.
#define FOLLY_ERASE FOLLY_ALWAYS_INLINE FOLLY_ATTR_VISIBILITY_HIDDEN

//  FOLLY_ERASE_HACK_GCC
//
//  Equivalent to FOLLY_ERASE, but without hiding under gcc. Useful when applied
//  to a function which may sometimes be hidden separately, for example by being
//  declared in an anonymous namespace, since in such cases with -Wattributes
//  enabled, gcc would emit: 'visibility' attribute ignored.
//
//  Semantically includes the inline specifier.
#if defined(__GNUC__) && !defined(__clang__)
#define FOLLY_ERASE_HACK_GCC FOLLY_ALWAYS_INLINE
#else
#define FOLLY_ERASE_HACK_GCC FOLLY_ERASE
#endif

//  FOLLY_ERASE_TRYCATCH
//
//  Equivalent to FOLLY_ERASE, but for code which might contain explicit
//  exception handling. Has the effect of FOLLY_ERASE, except under MSVC which
//  warns about __forceinline when functions contain exception handling.
//
//  Semantically includes the inline specifier.
#ifdef _MSC_VER
#define FOLLY_ERASE_TRYCATCH inline
#else
#define FOLLY_ERASE_TRYCATCH FOLLY_ERASE
#endif

// Generalize warning push/pop.
#if defined(__GNUC__) || defined(__clang__)
// Clang & GCC
#define FOLLY_PUSH_WARNING _Pragma("GCC diagnostic push")
#define FOLLY_POP_WARNING _Pragma("GCC diagnostic pop")
#define FOLLY_GNU_DISABLE_WARNING_INTERNAL2(warningName) #warningName
#define FOLLY_GNU_DISABLE_WARNING(warningName) \
  _Pragma(                                     \
      FOLLY_GNU_DISABLE_WARNING_INTERNAL2(GCC diagnostic ignored warningName))
#ifdef __clang__
#define FOLLY_CLANG_DISABLE_WARNING(warningName) \
  FOLLY_GNU_DISABLE_WARNING(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName)
#else
#define FOLLY_CLANG_DISABLE_WARNING(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName) \
  FOLLY_GNU_DISABLE_WARNING(warningName)
#endif
#define FOLLY_MSVC_DISABLE_WARNING(warningNumber)
#elif defined(_MSC_VER)
#define FOLLY_PUSH_WARNING __pragma(warning(push))
#define FOLLY_POP_WARNING __pragma(warning(pop))
// Disable the GCC warnings.
#define FOLLY_GNU_DISABLE_WARNING(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName)
#define FOLLY_CLANG_DISABLE_WARNING(warningName)
#define FOLLY_MSVC_DISABLE_WARNING(warningNumber) \
  __pragma(warning(disable : warningNumber))
#else
#define FOLLY_PUSH_WARNING
#define FOLLY_POP_WARNING
#define FOLLY_GNU_DISABLE_WARNING(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName)
#define FOLLY_CLANG_DISABLE_WARNING(warningName)
#define FOLLY_MSVC_DISABLE_WARNING(warningNumber)
#endif

#ifdef FOLLY_HAVE_SHADOW_LOCAL_WARNINGS
#define FOLLY_GCC_DISABLE_NEW_SHADOW_WARNINGS            \
  FOLLY_GNU_DISABLE_WARNING("-Wshadow-compatible-local") \
  FOLLY_GNU_DISABLE_WARNING("-Wshadow-local")            \
  FOLLY_GNU_DISABLE_WARNING("-Wshadow")
#else
#define FOLLY_GCC_DISABLE_NEW_SHADOW_WARNINGS /* empty */
#endif

#if defined(_MSC_VER)
#define FOLLY_MSVC_DECLSPEC(...) __declspec(__VA_ARGS__)
#else
#define FOLLY_MSVC_DECLSPEC(...)
#endif

#include <cstddef>

#if defined(_MSC_VER)
#define FOLLY_CPLUSPLUS _MSVC_LANG
#else
#define FOLLY_CPLUSPLUS __cplusplus
#endif

static_assert(FOLLY_CPLUSPLUS >= 201402L, "__cplusplus >= 201402L");

#if defined(__GNUC__) && !defined(__clang__)
static_assert(__GNUC__ >= 7, "__GNUC__ >= 7");
#endif

// Unaligned loads and stores
#if defined(FOLLY_HAVE_UNALIGNED_ACCESS) && FOLLY_HAVE_UNALIGNED_ACCESS
constexpr bool kHasUnalignedAccess = true;
#else
constexpr bool kHasUnalignedAccess = false;
#endif

// compiler specific attribute translation
// msvc should come first, so if clang is in msvc mode it gets the right defines

// NOTE: this will only do checking in msvc with versions that support /analyze
#ifdef _MSC_VER
#ifdef _USE_ATTRIBUTES_FOR_SAL
#undef _USE_ATTRIBUTES_FOR_SAL
#endif
/* nolint */
#define _USE_ATTRIBUTES_FOR_SAL 1
#include <sal.h> // @manual
#define FOLLY_PRINTF_FORMAT _Printf_format_string_
#define FOLLY_PRINTF_FORMAT_ATTR(format_param, dots_param) /**/
#else
#define FOLLY_PRINTF_FORMAT /**/
#define FOLLY_PRINTF_FORMAT_ATTR(format_param, dots_param) \
  __attribute__((__format__(__printf__, format_param, dots_param)))
#endif

// warn unused result
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define FOLLY_NODISCARD [[nodiscard]]
#endif
#endif
#if !defined FOLLY_NODISCARD
#if defined(_MSC_VER) && (_MSC_VER >= 1700)
#define FOLLY_NODISCARD _Check_return_
#elif defined(__GNUC__)
#define FOLLY_NODISCARD __attribute__((__warn_unused_result__))
#else
#define FOLLY_NODISCARD
#endif
#endif

// target
#ifdef _MSC_VER
#define FOLLY_TARGET_ATTRIBUTE(target)
#else
#define FOLLY_TARGET_ATTRIBUTE(target) __attribute__((__target__(target)))
#endif

// detection for 64 bit
#if defined(__x86_64__) || defined(_M_X64)
#define FOLLY_X64 1
#else
#define FOLLY_X64 0
#endif

#if defined(__arm__)
#define FOLLY_ARM 1
#else
#define FOLLY_ARM 0
#endif

#if defined(__aarch64__)
#define FOLLY_AARCH64 1
#else
#define FOLLY_AARCH64 0
#endif

#if defined(__powerpc64__)
#define FOLLY_PPC64 1
#else
#define FOLLY_PPC64 0
#endif

#if defined(__s390x__)
#define FOLLY_S390X 1
#else
#define FOLLY_S390X 0
#endif

constexpr bool kIsArchArm = FOLLY_ARM == 1;
constexpr bool kIsArchAmd64 = FOLLY_X64 == 1;
constexpr bool kIsArchAArch64 = FOLLY_AARCH64 == 1;
constexpr bool kIsArchPPC64 = FOLLY_PPC64 == 1;
constexpr bool kIsArchS390X = FOLLY_S390X == 1;


/**
 * folly::kIsLibrarySanitizeAddress reports if folly was compiled with ASAN
 * enabled.  Note that for compilation units outside of folly that include
 * folly/Portability.h, the value of kIsLibrarySanitizeAddress may be different
 * from whether or not the current compilation unit is being compiled with ASAN.
 */
#if FOLLY_LIBRARY_SANITIZE_ADDRESS
constexpr bool kIsLibrarySanitizeAddress = true;
#else
constexpr bool kIsLibrarySanitizeAddress = false;
#endif

#ifdef FOLLY_SANITIZE_ADDRESS
constexpr bool kIsSanitizeAddress = true;
#else
constexpr bool kIsSanitizeAddress = false;
#endif

#ifdef FOLLY_SANITIZE_THREAD
constexpr bool kIsSanitizeThread = true;
#else
constexpr bool kIsSanitizeThread = false;
#endif

#ifdef FOLLY_SANITIZE_DATAFLOW
constexpr bool kIsSanitizeDataflow = true;
#else
constexpr bool kIsSanitizeDataflow = false;
#endif

#ifdef FOLLY_SANITIZE
constexpr bool kIsSanitize = true;
#else
constexpr bool kIsSanitize = false;
#endif

// packing is very ugly in msvc
#ifdef _MSC_VER
#define FOLLY_PACK_ATTR /**/
#define FOLLY_PACK_PUSH __pragma(pack(push, 1))
#define FOLLY_PACK_POP __pragma(pack(pop))
#elif defined(__GNUC__)
#define FOLLY_PACK_ATTR __attribute__((__packed__))
#define FOLLY_PACK_PUSH /**/
#define FOLLY_PACK_POP /**/
#else
#define FOLLY_PACK_ATTR /**/
#define FOLLY_PACK_PUSH /**/
#define FOLLY_PACK_POP /**/
#endif

// It turns out that GNU libstdc++ and LLVM libc++ differ on how they implement
// the 'std' namespace; the latter uses inline namespaces. Wrap this decision
// up in a macro to make forward-declarations easier.
#if defined(_LIBCPP_VERSION)
#define FOLLY_NAMESPACE_STD_BEGIN _LIBCPP_BEGIN_NAMESPACE_STD
#define FOLLY_NAMESPACE_STD_END _LIBCPP_END_NAMESPACE_STD
#else
#define FOLLY_NAMESPACE_STD_BEGIN namespace std {
#define FOLLY_NAMESPACE_STD_END }
#endif

// If the new c++ ABI is used, __cxx11 inline namespace needs to be added to
// some types, e.g. std::list.
#if defined(_GLIBCXX_USE_CXX11_ABI) && _GLIBCXX_USE_CXX11_ABI
#define FOLLY_GLIBCXX_NAMESPACE_CXX11_BEGIN \
  inline _GLIBCXX_BEGIN_NAMESPACE_CXX11
#define FOLLY_GLIBCXX_NAMESPACE_CXX11_END _GLIBCXX_END_NAMESPACE_CXX11
#else
#define FOLLY_GLIBCXX_NAMESPACE_CXX11_BEGIN
#define FOLLY_GLIBCXX_NAMESPACE_CXX11_END
#endif

// MSVC specific defines
// mainly for posix compat
#ifdef _MSC_VER
#include <sys/types.h>

#ifdef _WIN32
#include <basetsd.h> // @manual

// This is a massive pain to have be an `int` due to the pthread implementation
// we support, but it's far more compatible with the rest of the windows world
// as an `int` than it would be as a `void*`
using pid_t = int;

using uid_t = int;
using gid_t = int;

// This isn't actually supposed to be defined here, but it's the most
// appropriate place without defining a portability header for stdint.h
// with just this single typedef.
using ssize_t = SSIZE_T;

#ifndef HAVE_MODE_T
#define HAVE_MODE_T 1
// The Windows headers don't define this anywhere, nor do any of the libs
// that Folly depends on, so define it here.
using mode_t = unsigned int;
#endif

#endif

// We have compiler support for the newest of the new, but
// MSVC doesn't tell us that.
//
// Clang pretends to be MSVC on Windows, but it refuses to compile
// SSE4.2 intrinsics unless -march argument is specified.
// So cannot unconditionally define __SSE4_2__ in clang.
#ifndef __clang__
#if !defined(_M_ARM) && !defined(_M_ARM64)
#define __SSE4_2__ 1
#endif // !defined(_M_ARM) && !defined(_M_ARM64)

// Hide a GCC specific thing that breaks MSVC if left alone.
#define __extension__

// compiler specific to compiler specific
// nolint
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#endif

// Define FOLLY_HAS_EXCEPTIONS
#if (defined(__cpp_exceptions) && __cpp_exceptions >= 199711) || \
FOLLY_HAS_FEATURE(cxx_exceptions)
#define FOLLY_HAS_EXCEPTIONS 1
#elif __GNUC__
#if defined(__EXCEPTIONS) && __EXCEPTIONS
#define FOLLY_HAS_EXCEPTIONS 1
#else // __EXCEPTIONS
#define FOLLY_HAS_EXCEPTIONS 0
#endif // __EXCEPTIONS
#elif FOLLY_MICROSOFT_ABI_VER
#if _CPPUNWIND
#define FOLLY_HAS_EXCEPTIONS 1
#else // _CPPUNWIND
#define FOLLY_HAS_EXCEPTIONS 0
#endif // _CPPUNWIND
#else
#define FOLLY_HAS_EXCEPTIONS 1 // default assumption for unknown platforms
#endif

// Debug
#ifdef NDEBUG
constexpr auto kIsDebug = false;
#else
constexpr auto kIsDebug = true;
#endif

// Exceptions
#if FOLLY_HAS_EXCEPTIONS
constexpr auto kHasExceptions = true;
#else
constexpr auto kHasExceptions = false;
#endif

// Endianness
#ifdef _MSC_VER
// It's MSVC, so we just have to guess ... and allow an override
#ifdef FOLLY_ENDIAN_BE
constexpr auto kIsLittleEndian = false;
#else
constexpr auto kIsLittleEndian = true;
#endif
#else
constexpr auto kIsLittleEndian = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#endif
constexpr auto kIsBigEndian = !kIsLittleEndian;

// Weak
#if FOLLY_HAVE_WEAK_SYMBOLS
constexpr auto kHasWeakSymbols = true;
#else
constexpr auto kHasWeakSymbols = false;
#endif

#ifndef FOLLY_SSE
#if defined(__SSE4_2__)
#define FOLLY_SSE 4
#define FOLLY_SSE_MINOR 2
#elif defined(__SSE4_1__)
#define FOLLY_SSE 4
#define FOLLY_SSE_MINOR 1
#elif defined(__SSE4__)
#define FOLLY_SSE 4
#define FOLLY_SSE_MINOR 0
#elif defined(__SSE3__)
#define FOLLY_SSE 3
#define FOLLY_SSE_MINOR 0
#elif defined(__SSE2__)
#define FOLLY_SSE 2
#define FOLLY_SSE_MINOR 0
#elif defined(__SSE__)
#define FOLLY_SSE 1
#define FOLLY_SSE_MINOR 0
#else
#define FOLLY_SSE 0
#define FOLLY_SSE_MINOR 0
#endif
#endif

#ifndef FOLLY_SSSE
#if defined(__SSSE3__)
#define FOLLY_SSSE 3
#else
#define FOLLY_SSSE 0
#endif
#endif

#define FOLLY_SSE_PREREQ(major, minor) \
  (FOLLY_SSE > (major) || FOLLY_SSE == (major) && FOLLY_SSE_MINOR >= (minor))

#ifndef FOLLY_NEON
#if (defined(__ARM_NEON) || defined(__ARM_NEON__)) && !defined(__CUDACC__)
#define FOLLY_NEON 1
#else
#define FOLLY_NEON 0
#endif
#endif

#ifndef FOLLY_ARM_FEATURE_CRC32
#ifdef __ARM_FEATURE_CRC32
#define FOLLY_ARM_FEATURE_CRC32 1
#else
#define FOLLY_ARM_FEATURE_CRC32 0
#endif
#endif

// RTTI may not be enabled for this compilation unit.
#if defined(__GXX_RTTI) || defined(__cpp_rtti) || \
    (defined(_MSC_VER) && defined(_CPPRTTI))
#define FOLLY_HAS_RTTI 1
#else
#define FOLLY_HAS_RTTI 0
#endif

constexpr const bool kHasRtti = FOLLY_HAS_RTTI;

#if defined(__APPLE__) || defined(_MSC_VER)
#define FOLLY_STATIC_CTOR_PRIORITY_MAX
#else
// 101 is the highest priority allowed by the init_priority attribute.
// This priority is already used by JEMalloc and other memory allocators so
// we will take the next one.
#define FOLLY_STATIC_CTOR_PRIORITY_MAX __attribute__((__init_priority__(102)))
#endif

#if defined(__APPLE__) && TARGET_OS_IOS
#define FOLLY_APPLE_IOS 1
#else
#define FOLLY_APPLE_IOS 0
#endif

#if defined(__APPLE__) && TARGET_OS_OSX
#define FOLLY_APPLE_MACOS 1
#else
#define FOLLY_APPLE_MACOS 0
#endif

#if defined(__APPLE__) && TARGET_OS_TV
#define FOLLY_APPLE_TVOS 1
#else
#define FOLLY_APPLE_TVOS 0
#endif

#if defined(__APPLE__) && TARGET_OS_WATCH
#define FOLLY_APPLE_WATCHOS 1
#else
#define FOLLY_APPLE_WATCHOS 0
#endif


#ifdef __OBJC__
constexpr auto kIsObjC = true;
#else
constexpr auto kIsObjC = false;
#endif

#if FOLLY_MOBILE
constexpr auto kIsMobile = true;
#else
constexpr auto kIsMobile = false;
#endif

#if defined(__linux__) && !FOLLY_MOBILE
constexpr auto kIsLinux = true;
#else
constexpr auto kIsLinux = false;
#endif

#if defined(_WIN32)
constexpr auto kIsWindows = true;
#else
constexpr auto kIsWindows = false;
#endif

#if defined(__APPLE__)
constexpr auto kIsApple = true;
#else
constexpr auto kIsApple = false;
#endif

constexpr bool kIsAppleIOS = FOLLY_APPLE_IOS == 1;
constexpr bool kIsAppleMacOS = FOLLY_APPLE_MACOS == 1;
constexpr bool kIsAppleTVOS = FOLLY_APPLE_TVOS == 1;
constexpr bool kIsAppleWatchOS = FOLLY_APPLE_WATCHOS == 1;

#if defined(__GLIBCXX__)
constexpr auto kIsGlibcxx = true;
#else
constexpr auto kIsGlibcxx = false;
#endif

#if defined(__GLIBCXX__) && _GLIBCXX_RELEASE // major version, 7+
constexpr auto kGlibcxxVer = _GLIBCXX_RELEASE;
#else
constexpr auto kGlibcxxVer = 0;
#endif

#if defined(__GLIBCXX__) && defined(_GLIBCXX_ASSERTIONS)
constexpr auto kGlibcxxAssertions = true;
#else
constexpr auto kGlibcxxAssertions = false;
#endif

#ifdef _LIBCPP_VERSION
constexpr auto kIsLibcpp = true;
#else
constexpr auto kIsLibcpp = false;
#endif

#if defined(__GLIBCXX__)
constexpr auto kIsLibstdcpp = true;
#else
constexpr auto kIsLibstdcpp = false;
#endif

#ifdef _MSC_VER
constexpr auto kMscVer = _MSC_VER;
#else
constexpr auto kMscVer = 0;
#endif

#if defined(__GNUC__) && __GNUC__
constexpr auto kGnuc = __GNUC__;
#else
constexpr auto kGnuc = 0;
#endif

#if __clang__
constexpr auto kIsClang = true;
constexpr auto kClangVerMajor = __clang_major__;
#else
constexpr auto kIsClang = false;
constexpr auto kClangVerMajor = 0;
#endif

#ifdef FOLLY_MICROSOFT_ABI_VER
constexpr auto kMicrosoftAbiVer = FOLLY_MICROSOFT_ABI_VER;
#else
constexpr auto kMicrosoftAbiVer = 0;
#endif

// cpplib is an implementation of the standard library, and is the one typically
// used with the msvc compiler
#ifdef _CPPLIB_VER
constexpr auto kCpplibVer = _CPPLIB_VER;
#else
constexpr auto kCpplibVer = 0;
#endif

//  MSVC does not permit:
//
//    extern int const num;
//    constexpr int const num = 3;
//
//  Instead:
//
//    extern int const num;
//    FOLLY_STORAGE_CONSTEXPR int const num = 3;
//
//  True as of MSVC 2017.
#ifdef _MSC_VER
#define FOLLY_STORAGE_CONSTEXPR
#else
#define FOLLY_STORAGE_CONSTEXPR constexpr
#endif

//  FOLLY_CXX17_CONSTEXPR
//
//  C++17 permits more cases to be marked constexpr, including lambda bodies and
//  the `if` keyword.
#if FOLLY_CPLUSPLUS >= 201703L
#define FOLLY_CXX17_CONSTEXPR constexpr
#else
#define FOLLY_CXX17_CONSTEXPR
#endif

//  FOLLY_CXX20_CONSTEXPR
//
//  C++20 permits more cases to be marked constexpr, including constructors that
//  leave members uninitialized and virtual functions.
#if FOLLY_CPLUSPLUS >= 202002L
#define FOLLY_CXX20_CONSTEXPR constexpr
#else
#define FOLLY_CXX20_CONSTEXPR
#endif

// C++20 constinit
#if defined(__cpp_constinit) && __cpp_constinit >= 201907L
#define FOLLY_CONSTINIT constinit
#else
#define FOLLY_CONSTINIT
#endif

#if defined(FOLLY_CFG_NO_COROUTINES)
#define FOLLY_HAS_COROUTINES 0
#else
#if FOLLY_CPLUSPLUS >= 201703L
// folly::coro requires C++17 support
#if defined(__NVCC__)
// For now, NVCC matches other compilers but does not offer coroutines.
#define FOLLY_HAS_COROUTINES 0
#elif defined(_WIN32) && defined(__clang__) && !defined(LLVM_COROUTINES)
// LLVM and MSVC coroutines are ABI incompatible, so for the MSVC implementation
// of <experimental/coroutine> on Windows we *don't* have coroutines.
//
// LLVM_COROUTINES indicates that LLVM compatible header is added to include
// path and can be used.
//
// Worse, if we define FOLLY_HAS_COROUTINES 1 we will include
// <experimental/coroutine> which will conflict with anyone who wants to load
// the LLVM implementation of coroutines on Windows.
#define FOLLY_HAS_COROUTINES 0
#elif defined(_MSC_VER) && _MSC_VER && defined(_RESUMABLE_FUNCTIONS_SUPPORTED)
// NOTE: MSVC 2017 does not currently support the full Coroutines TS since it
// does not yet support symmetric-transfer.
#define FOLLY_HAS_COROUTINES 0
#elif (                                                                    \
    (defined(__cpp_coroutines) && __cpp_coroutines >= 201703L) ||          \
    (defined(__cpp_impl_coroutine) && __cpp_impl_coroutine >= 201902L)) && \
    (__has_include(<coroutine>) || __has_include(<experimental/coroutine>))
#define FOLLY_HAS_COROUTINES 1
// This is mainly to workaround bugs triggered by LTO, when stack allocated
// variables in await_suspend end up on a coroutine frame.
#define FOLLY_CORO_AWAIT_SUSPEND_NONTRIVIAL_ATTRIBUTES FOLLY_NOINLINE
#else
#define FOLLY_HAS_COROUTINES 0
#endif
#else
#define FOLLY_HAS_COROUTINES 0
#endif // FOLLY_CPLUSPLUS >= 201703L
#endif // FOLLY_CFG_NO_COROUTINES

// MSVC 2017.5 && C++17
#if __cpp_noexcept_function_type >= 201510 || \
    (_MSC_FULL_VER >= 191225816 && _MSVC_LANG > 201402)
#define FOLLY_HAVE_NOEXCEPT_FUNCTION_TYPE 1
#endif

#if __cpp_inline_variables >= 201606L || FOLLY_CPLUSPLUS >= 201703L
#define FOLLY_HAS_INLINE_VARIABLES 1
#define FOLLY_INLINE_VARIABLE inline
#else
#define FOLLY_HAS_INLINE_VARIABLES 0
#define FOLLY_INLINE_VARIABLE
#endif

// feature test __cpp_lib_string_view is defined in <string>, which is
// too heavy to include here.
#if __has_include(<string_view>) && FOLLY_CPLUSPLUS >= 201703L
#define FOLLY_HAS_STRING_VIEW 1
#else
#define FOLLY_HAS_STRING_VIEW 0
#endif

// C++20 consteval
#if FOLLY_CPLUSPLUS >= 202002L
#define FOLLY_CONSTEVAL consteval
#else
#define FOLLY_CONSTEVAL constexpr
#endif

// C++17 deduction guides
#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201703L
#define FOLLY_HAS_DEDUCTION_GUIDES 1
#else
#define FOLLY_HAS_DEDUCTION_GUIDES 0
#endif

#ifndef _WIN32
#include <sched.h>
#else
#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2


namespace tssched
{
    struct sched_param
    {
        int sched_priority;
    };

    int sched_yield();
    int sched_get_priority_min(int policy);
    int sched_get_priority_max(int policy);
} // namespace sched

FOLLY_PUSH_WARNING
FOLLY_CLANG_DISABLE_WARNING("-Wheader-hygiene")
/* using override */
using namespace tssched;
FOLLY_POP_WARNING
#endif


#if !defined(_WIN32)

#include <pthread.h>

#if defined(__FreeBSD__)
#include <sys/thr.h> // @manual
#endif

#else
#include <cstdint>
#include <memory>
#include <Windows.h>

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL

#define _POSIX_TIMEOUTS 200112L

namespace tSpthread
{
    struct pthread_attr_t
    {
        size_t stackSize;
        bool detached;
    };

    int pthread_attr_init(pthread_attr_t* attr);
    int pthread_attr_setdetachstate(pthread_attr_t* attr, int state);
    int pthread_attr_setstacksize(pthread_attr_t* attr, size_t kb);

    namespace pthread_detail
    {
        struct pthread_t
        {
            HANDLE handle{INVALID_HANDLE_VALUE};
            DWORD threadID{0};
            bool detached{false};

            ~pthread_t() noexcept;
        };
    } // namespace pthread_detail
    using pthread_t = std::shared_ptr<pthread_detail::pthread_t>;

    int pthread_equal(const pthread_t& threadA, const pthread_t& threadB);
    int pthread_create(
        pthread_t* thread,
        const pthread_attr_t* attr,
        void* (*start_routine)(void*),
        void* arg);
    pthread_t pthread_self();
    int pthread_join(const pthread_t& thread, void** exitCode);

    HANDLE pthread_getw32threadhandle_np(const pthread_t& thread);
    DWORD pthread_getw32threadid_np(const pthread_t& thread);

    int pthread_setschedparam(
        const pthread_t& thread, int policy, const sched_param* param);

    struct pthread_mutexattr_t
    {
        int type;
    };

    int pthread_mutexattr_init(pthread_mutexattr_t* attr);
    int pthread_mutexattr_destroy(const pthread_mutexattr_t* attr);
    int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type);

    using pthread_mutex_t = struct pthread_mutex_t_*;
    int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr);
    int pthread_mutex_destroy(pthread_mutex_t* mutex);
    int pthread_mutex_lock(const pthread_mutex_t* mutex);
    int pthread_mutex_trylock(const pthread_mutex_t* mutex);
    int pthread_mutex_unlock(const pthread_mutex_t* mutex);
    int pthread_mutex_timedlock(
        const pthread_mutex_t* mutex, const timespec* abs_timeout);

    using pthread_rwlock_t = struct pthread_rwlock_t_*;
    // Technically the second argument here is supposed to be a
    // const pthread_rwlockattr_t* but we don support it, so we
    // simply don't define pthread_rwlockattr_t at all to cause
    // a build-break if anyone tries to use it.
    int pthread_rwlock_init(pthread_rwlock_t* rwlock, const void* attr);
    int pthread_rwlock_destroy(pthread_rwlock_t* rwlock);
    int pthread_rwlock_rdlock(const pthread_rwlock_t* rwlock);
    int pthread_rwlock_tryrdlock(const pthread_rwlock_t* rwlock);
    int pthread_rwlock_timedrdlock(
        const pthread_rwlock_t* rwlock, const timespec* abs_timeout);
    int pthread_rwlock_wrlock(const pthread_rwlock_t* rwlock);
    int pthread_rwlock_trywrlock(const pthread_rwlock_t* rwlock);
    int pthread_rwlock_timedwrlock(
        const pthread_rwlock_t* rwlock, const timespec* abs_timeout);
    int pthread_rwlock_unlock(const pthread_rwlock_t* rwlock);

    using pthread_cond_t = struct pthread_cond_t_*;
    // Once again, technically the second argument should be a
    // pthread_condattr_t, but we don't implement it, so void*
    // it is.
    int pthread_cond_init(pthread_cond_t* cond, const void* attr);
    int pthread_cond_destroy(pthread_cond_t* cond);
    int pthread_cond_wait(const pthread_cond_t* cond, const pthread_mutex_t* mutex);
    int pthread_cond_timedwait(
        const pthread_cond_t* cond, const pthread_mutex_t* mutex, const timespec* abstime);
    int pthread_cond_signal(const pthread_cond_t* cond);
    int pthread_cond_broadcast(const pthread_cond_t* cond);

    // In reality, this is boost::thread_specific_ptr*, but we're attempting
    // to avoid introducing boost into a portability header.
    using pthread_key_t = void*;

    int pthread_key_create(pthread_key_t* key, void (*destructor)(void*));
    int pthread_key_delete(pthread_key_t key);
    void* pthread_getspecific(pthread_key_t key);
    int pthread_setspecific(pthread_key_t key, const void* value);
} // namespace pthread

FOLLY_PUSH_WARNING
FOLLY_CLANG_DISABLE_WARNING("-Wheader-hygiene")
/* using override */
using namespace tSpthread;
FOLLY_POP_WARNING

#endif

#ifndef _WIN32

#include <sys/time.h>

// win32 #defines timezone; this avoids collision
using folly_port_struct_timezone = struct timezone;

#else

// Someone decided this was a good place to define timeval.....
#include <folly/portability/Windows.h>

struct folly_port_struct_timezone_
{
    int tz_minuteswest;
    int tz_dsttime;
};

using folly_port_struct_timezone = struct folly_port_struct_timezone_;

extern "C" {
// We use folly_port_struct_timezone due to issues with #defines on Windows
// platforms.
// The python 3 headers `#define timezone _timezone` on Windows. `_timezone` is
// a global field that contains information on the current timezone.
// As such "timezone" is not a good name to use inside of C/C++ code on
// Windows.  Instead users should use folly_port_struct_timezone instead.
int gettimeofday(timeval* tv, folly_port_struct_timezone*);
void timeradd(const timeval* a, const timeval* b, timeval* res);
void timersub(const timeval* a, const timeval* b, timeval* res);
}

tm* localtime_r(const time_t* t, tm* o);

#endif

#endif
