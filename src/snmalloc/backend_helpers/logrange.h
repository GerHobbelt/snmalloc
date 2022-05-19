#pragma once

#include "empty_range.h"

namespace snmalloc
{
  /**
   * RangeName is an integer to specify which range is being logged. Strings can
   * be used as template parameters.
   *
   * ParentRange is what the range is logging calls to.
   */
  template<size_t RangeName, typename ParentRange = EmptyRange>
  class LogRange
  {
    ParentRange parent{};

  public:
    /**
     * We use a nested Apply type to enable a Pipe operation.
     */
    template<typename ParentRange2>
    using Apply = LogRange<RangeName, ParentRange2>;

    static constexpr bool Aligned = ParentRange::Aligned;

    static constexpr bool ConcurrencySafe = ParentRange::ConcurrencySafe;

    constexpr LogRange() = default;

    capptr::Chunk<void> alloc_range(size_t size)
    {
#ifdef SNMALLOC_TRACING
      message<1024>("Call alloc_range({}) on {}", size, RangeName);
#endif
      auto range = parent.alloc_range(size);
#ifdef SNMALLOC_TRACING
      message<1024>(
        "{} = alloc_range({}) in {}", range.unsafe_ptr(), size, RangeName);
#endif
      return range;
    }

    void dealloc_range(capptr::Chunk<void> base, size_t size)
    {
#ifdef SNMALLOC_TRACING
      message<1024>(
        "dealloc_range({}, {}}) on {}", base.unsafe_ptr(), size, RangeName);
#endif
      parent.dealloc_range(base, size);
#ifdef SNMALLOC_TRACING
      message<1024>(
        "Done dealloc_range({}, {}})! on {}",
        base.unsafe_ptr(),
        size,
        RangeName);
#endif
    }
  };
} // namespace snmalloc