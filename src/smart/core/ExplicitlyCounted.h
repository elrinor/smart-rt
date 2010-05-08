#ifndef __SMART_COUNTED_H__
#define __SMART_COUNTED_H__

#include "common.h"

namespace smart {
  template<class T>
  class Destroyer {
  public:
    virtual void operator()(T* ptr) = 0;
  };

  template<class T> class ExplicitlyCounted {
  public:
    ExplicitlyCounted(T* ptr, Destroyer<T>* d): mPtr(ptr), mDestroyer(d), mUseCount(1) {}

    ExplicitlyCounted(T* ptr): mPtr(ptr), mDestroyer(NULL), mUseCount(1) {}

    ExplicitlyCounted(): mPtr(NULL), mDestroyer(NULL), mUseCount(0) {}

    void initialize(T* ptr) {
      assert(mUseCount == 0);
      mPtr = ptr;
      mUseCount = 1;
    }

    void setDestroyer(Destroyer<T>* d) {
      mDestroyer = d;
    }

    void claimOwnership() {
      assert(mUseCount > 0);
      mUseCount++;
    }

    void releaseOwnership() {
      assert(mUseCount > 0);
      mUseCount--;
      if(mUseCount == 0)
        (*mDestroyer)(mPtr);
    }

  private:
    T* mPtr;
    int mUseCount;
    Destroyer<T>* mDestroyer;
  };

}

#endif