#include "libc.h"
#include "override.h"
// Must be included after snmalloc headers
#include "../mem/memcpy.h"

using namespace snmalloc;

#ifndef MALLOC_USABLE_SIZE_QUALIFIER
#  define MALLOC_USABLE_SIZE_QUALIFIER
#endif

namespace
{
  /**
   * Specialised memcpy that knows that it is copying at least the minimum
   * object size and that the start and end are strongly aligned.
   */
  SNMALLOC_FAST_PATH_INLINE
  void memcpy_for_realloc(void* dst, const void* src, size_t size)
  {
    // Tell the compiler some things that it can use to optimise the memcpy:
    // The size is at least the minimum alloc size (skip all of the small copy
    // things) and the size is a multiple of the smallest size.
    SNMALLOC_ASSUME(size >= MIN_ALLOC_SIZE);
    SNMALLOC_ASSUME((size % MIN_ALLOC_SIZE) == 0);
    // The start of both objects is strongly aligned
    SNMALLOC_ASSUME(
      (static_cast<size_t>(reinterpret_cast<uintptr_t>(src)) &
       (MIN_ALLOC_SIZE - 1)) == 0);
    SNMALLOC_ASSUME(
      (static_cast<size_t>(reinterpret_cast<uintptr_t>(dst)) &
       (MIN_ALLOC_SIZE - 1)) == 0);
    snmalloc::memcpy<DEBUG, false>(dst, src, size);
  }
} // namespace

extern "C"
{
  SNMALLOC_EXPORT void* SNMALLOC_NAME_MANGLE(__malloc_end_pointer)(void* ptr)
  {
    return snmalloc::libc::__malloc_end_pointer(ptr);
  }

  SNMALLOC_EXPORT void* SNMALLOC_NAME_MANGLE(malloc)(size_t size)
  {
    return snmalloc::libc::malloc(size);
  }

  SNMALLOC_EXPORT void SNMALLOC_NAME_MANGLE(free)(void* ptr)
  {
    snmalloc::libc::free(ptr);
  }

  SNMALLOC_EXPORT void SNMALLOC_NAME_MANGLE(cfree)(void* ptr)
  {
    snmalloc::libc::free(ptr);
  }

  SNMALLOC_EXPORT void* SNMALLOC_NAME_MANGLE(calloc)(size_t nmemb, size_t size)
  {
    return snmalloc::libc::calloc(nmemb, size);
  }

  SNMALLOC_EXPORT
  size_t SNMALLOC_NAME_MANGLE(malloc_usable_size)(
    MALLOC_USABLE_SIZE_QUALIFIER void* ptr)
  {
    return snmalloc::libc::malloc_usable_size(ptr);
  }

#ifdef _WIN32
  SNMALLOC_EXPORT
  size_t SNMALLOC_NAME_MANGLE(_msize)(MALLOC_USABLE_SIZE_QUALIFIER void* ptr)
  {
    return snmalloc::libc::malloc_usable_size(ptr);
  }
#endif

  SNMALLOC_EXPORT
  size_t SNMALLOC_NAME_MANGLE(malloc_good_size)(size_t size)
  {
    return round_size(size);
  }

  SNMALLOC_EXPORT void* SNMALLOC_NAME_MANGLE(realloc)(void* ptr, size_t size)
  {
    return snmalloc::libc::realloc(ptr, size);
  }

#if !defined(SNMALLOC_NO_REALLOCARRAY)
  SNMALLOC_EXPORT void*
  SNMALLOC_NAME_MANGLE(reallocarray)(void* ptr, size_t nmemb, size_t size)
  {
    return snmalloc::libc::reallocarray(ptr, nmemb, size);
  }
#endif

#if !defined(SNMALLOC_NO_REALLOCARR)
  SNMALLOC_EXPORT int
  SNMALLOC_NAME_MANGLE(reallocarr)(void* ptr, size_t nmemb, size_t size)
  {
    return snmalloc::libc::reallocarr(ptr, nmemb, size);
  }
#endif

  SNMALLOC_EXPORT void*
  SNMALLOC_NAME_MANGLE(memalign)(size_t alignment, size_t size)
  {
    return snmalloc::libc::memalign(alignment, size);
  }

  SNMALLOC_EXPORT void*
  SNMALLOC_NAME_MANGLE(aligned_alloc)(size_t alignment, size_t size)
  {
    return snmalloc::libc::memalign(alignment, size);
  }

  SNMALLOC_EXPORT int SNMALLOC_NAME_MANGLE(posix_memalign)(
    void** memptr, size_t alignment, size_t size)
  {
    return snmalloc::libc::posix_memalign(memptr, alignment, size);
  }

#if !defined(__FreeBSD__) && !defined(__OpenBSD__)
  SNMALLOC_EXPORT void* SNMALLOC_NAME_MANGLE(valloc)(size_t size)
  {
    return snmalloc::libc::memalign(OS_PAGE_SIZE, size);
  }
#endif

  SNMALLOC_EXPORT void* SNMALLOC_NAME_MANGLE(pvalloc)(size_t size)
  {
    return snmalloc::libc::memalign(
      OS_PAGE_SIZE, (size + OS_PAGE_SIZE - 1) & ~(OS_PAGE_SIZE - 1));
  }

#if __has_include(<features.h>)
#  include <features.h>
#endif
#if defined(__GLIBC__) && !defined(SNMALLOC_PASS_THROUGH)
  // glibc uses these hooks to replace malloc.
  // This is required when RTL_DEEPBIND is used and the library is
  // LD_PRELOADed.
  // See https://github.com/microsoft/snmalloc/issues/595
  SNMALLOC_EXPORT void (*SNMALLOC_NAME_MANGLE(__free_hook))(void* ptr) =
    &SNMALLOC_NAME_MANGLE(free);
  SNMALLOC_EXPORT void* (*SNMALLOC_NAME_MANGLE(__malloc_hook))(size_t size) =
    &SNMALLOC_NAME_MANGLE(malloc);
  SNMALLOC_EXPORT void* (*SNMALLOC_NAME_MANGLE(__realloc_hook))(
    void* ptr, size_t size) = &SNMALLOC_NAME_MANGLE(realloc);
  SNMALLOC_EXPORT void* (*SNMALLOC_NAME_MANGLE(__memalign_hook))(
    size_t alignment, size_t size) = &SNMALLOC_NAME_MANGLE(memalign);
#endif
}
