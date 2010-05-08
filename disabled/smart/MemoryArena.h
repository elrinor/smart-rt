#ifndef __SMART_MEMORYARENA__
#define __SMART_MEMORYARENA__

#include "common.h"
#include <algorithm>
#include <memory>
#include <arx/Utility.h>

namespace smart {
// -------------------------------------------------------------------------- //
// MemoryArena
// -------------------------------------------------------------------------- //
  /** Class used for allocation of raw memory from memory arena. Doesn't meet
   * the requirements of allocator! */
  template<class Type, class Allocator = std::allocator<Type> >
  class MemoryArena: public arx::noncopyable {
  public:
    typedef Type value_type;
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::pointer pointer;

    MemoryArena() {
      initialize(std::max(1, 512 / sizeof(value_type) - blockOverhead));
    }

    MemoryArena(size_type defaultBlockCapacity, allocator_type alloc): mAllocator(alloc) {
      initialize(defaultBlockCapacity);
    }

    MemoryArena(size_type defaultBlockCapacity) {
      initialize(defaultBlockCapacity);
    }

    pointer allocate(size_type count) {
      assert(count > 0);
      if(mCurrentEndPtr - mCurrentPtr < count)
        newBlock(std::max(count, mDefaultBlockCapacity));
      mCurrentPtr += count;
      return mCurrentPtr - count;
    }

    void deallocate(pointer /* ptr */, size_type /* count */){
      return;
    }

    void setDefaultBlockCapacity(int defaultBlockCapacity) {
      assert(defaultBlockCapacity > 0);
      mDefaultBlockCapacity = defaultBlockCapacity;
    }

    ~MemoryArena() {
      ArenaBlock* block = mStartingBlock;
      while(block != NULL) {
        ArenaBlock* next = block->mNext;
        mAllocator.deallocate(block, block->mCapacity);
        block = next;
      }
    }

  private:
    struct ArenaBlock {
      ArenaBlock* mNext;
      size_type mCapacity;
    };

    enum {
      blockOverhead = (sizeof(ArenaBlock) + sizeof(value_type) - 1) / sizeof(value_type)
    };

    /** Initializer. Called from constructors. */
    void initialize(size_type defaultBlockCapacity) {
      mStartingBlock = NULL;
      mCurrentBlock = NULL;
      mCurrentEndPtr = NULL;
      mCurrentPtr = NULL;
      mDefaultBlockCapacity = defaultBlockCapacity;
    }

    /** Allocates a new block of the given capacity and updates all the necessary
     * data structures. */
    void newBlock(size_type capacity) {
      /* Allocate new block and initialize it. */
      ArenaBlock* newBlock = reinterpret_cast<ArenaBlock*>(mAllocator.allocate(capacity + blockOverhead));
      newBlock->mNext = NULL;
      newBlock->mCapacity = capacity;

      if(mCurrentBlock == NULL) {
        /* We're uninitialized - store new block as starting. */
        mStartingBlock = newBlock;
        mCurrentBlock = newBlock;
      } else {
        /* Update current block and store new one as current. */
        mCurrentBlock->mNext = newBlock;
        mCurrentBlock = newBlock;
      }

      /* Reset working members. */
      mCurrentPtr = reinterpret_cast<value_type*>(mCurrentBlock) + blockOverhead * sizeof(value_type);
      mCurrentEndPtr = mCurrentPtr + mCurrentBlock->mCapacity;
    }

    /** Allocator associated with this arena object. */
    allocator_type mAllocator;

    /** Pointer to the first memory block of this arena. */
    ArenaBlock* mStartingBlock;

    /** Pointer to the current memory block of this arena. */
    ArenaBlock* mCurrentBlock;

    /** Pointer to the end of the current block. */
    pointer mCurrentEndPtr;

    /** Pointer to the first free object in current block. */
    pointer mCurrentPtr;

    /** Default block size. */
    size_type mDefaultBlockCapacity;
  };
} // namespace smart

#endif // __SMART_MEMORYARENA__
