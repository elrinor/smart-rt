#include "Machine.h"
#include <arx/config.h>

#if defined(ARX_WIN32)
#  include <Windows.h>
#else
#  error "Platform not supported"
#endif

namespace smart {
  
  int Machine::getProcessorCount() {
#if defined(ARX_WIN32)
    SYSTEM_INFO sInfo;
    GetSystemInfo(&sInfo);
    return sInfo.dwNumberOfProcessors;
#else
    return 1;
#endif
  }

} // namespace smart
