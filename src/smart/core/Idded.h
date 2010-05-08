#ifndef __SMART_IDDED_H__
#define __SMART_IDDED_H__

#include "common.h"

namespace smart {
// -------------------------------------------------------------------------- //
// Idded
// -------------------------------------------------------------------------- //
  class Idded {
  public:
    Idded(int id): mId(id) {}

    Idded(): mId(SMART_INVALID_ID) {}

    int getId() const {
      return mId;
    }

  protected:
    void setId(int id) {
      assert(mId == SMART_INVALID_ID && id != SMART_INVALID_ID);
      mId = id;
    }

  private:
    friend class SmartCore;
    friend class RenderManager;
    friend class ShaderManager;

    int mId;
  };

} // namespace smart

#endif // __SMART_IDDED_H__
