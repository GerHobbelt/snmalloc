#include "../ds/ds.h"
extern "C"
{
#include <string.h>
#include <unistd.h>
}

namespace snmalloc
{
  static void __attribute__((constructor)) snmalloc_announce_configuration(void)
  {
    const char* verdesc =
      "snmalloc2"
#define VERDESC_STR(x) #x
#define VERDESC_XSTR(x) VERDESC_STR(x)
#ifdef CHECK_CLIENT
      " check-client"
#endif
#ifndef NDEBUG
      " debug"
#endif
      "\n";

    write(2, verdesc, strlen(verdesc));
  }
};
