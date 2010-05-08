#ifndef __SMART_CORE_ARCH_MACHINE_H__
#define __SMART_CORE_ARCH_MACHINE_H__

#include "../common.h"

namespace smart {

  /** Class for access to platform-specific features. */
  class Machine {
  public:
    /** @returns the number of processors in this machine. */
    static int getProcessorCount();
  };

} // namespace smart

#endif // __SMART_CORE_ARCH_MACHINE_H__
