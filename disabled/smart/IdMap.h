#ifndef __SMART_IDMAP_H__
#define __SMART_IDMAP_H__

#include "common.h"
#include <arx/Collections.h>
#include <arx/Memory.h>

namespace smart {

  /**
   * IdMap class is a lightweight container for storage of objects that are 
   * accessed by their identifiers. 
   */
  template<class Type, class Allocator = arx::classnew_allocator<Type> >
  class IdMap {
  public:
    typedef Type value_type;
    typedef Allocator allocator_type;
    typedef int size_type;

    IdMap() {}

    IdMap(const IdMap& other): mData(other.mData), mFreeStack(other.mFreeStack) {
#ifdef DEBUG
      mDebugOccupied = other.mDebugOccupied;
#endif
    }

    explicit IdMap(const allocator_type& allocator): 
      mData(allocator) {}

    explicit IdMap(size_type capacity): 
      mData(capacity) {}

    IdMap(size_type capacity, const allocator_type& allocator): 
      mData(capacity, allocator) {}

    const Type& operator[] (int id) const {
      assert(mDebugOccupied[id]);
      return mData[id];
    }

    Type& operator[] (int id) {
      assert(mDebugOccupied[id]);
      return mData[id];
    }

    /** @returns the first free id in this IdMap. */
    int firstFreeId() const {
      if(mFreeStack.size() == 0) {
        return mData.size();
      } else {
        return mFreeStack.back();
      }
    }

    /** Adds a new value to this IdMap with the given id. The given id must be 
     * free. You can obtain free id by calling firstFreeId() method. */
    void put(int id, const Type& value) {
      assert(!mDebugOccupied[id]);
#ifdef DEBUG
      mDebugOccupied[id] = true;
#endif

      if(id == mData.size()) {
        mData.push_back(value);
      } else if(id == mFreeStack.back()) {
        Type* ptr = mData.data() + id;
        mData.get_allocator().destroy(ptr);
        mData.get_allocator().construct(ptr, value);
        mFreeStack.pop_back();
      } else if(id > mData.size()) {
        mData.resize(id);
        mData.push_back(value);
      } else {
        int tmpId = mFreeStack.back();
        mFreeStack.pop_back();
        for(int i = mFreeStack.size() - 1; true; --i) {
          std::swap(tmpId, mFreeStack[i]);
          if(tmpId == id) {
            Type* ptr = mData.data() + id;
            mData.get_allocator().destroy(ptr);
            mData.get_allocator().construct(ptr, value);
            return;
          }
        }
      }
    }

    /** Removes the value associated with the given id from this IdMap.
     * Note that the constructor for this value is not called immediately.
     * Instead, it will be called when a new value will be assigned for this id. */
    void remove(int id) {
      assert(mDebugOccupied[id]);
      mFreeStack.push_back(id);
#ifdef DEBUG
      mDebugOccupied[id] = false;
#endif
    }

  private:
    arx::CheckedFastArray<Type> mData;
    arx::CheckedFastArray<int> mFreeStack;

#ifdef DEBUG
    arx::Map<int, bool> mDebugOccupied;
#endif
  };

}

#endif // __SMART_IDMAP_H__
