#ifndef __SMART_MEMORYARENA_V2_H__
#define __SMART_MEMORYARENA_V2_H__

#include "common.h"
#include <cassert>
#include <functional>
#include <memory>
#include <arx/Utility.h>
#include <arx/TypeTraits.h>
#include <arx/Collections.h>

namespace smart {
  template<int mul, int div, int add>
  class MulDivAdd: public std::unary_function<int, int> {
    STATIC_ASSERT((mul >= 0 && div > 0 && add >= 0));
  public:
    int operator()(int value) const {
      return value * mul / div + add;
    }
  };

  /** MemoryArena class provides an allocator-like interface for allocations,
   * which follow the "allocate-everything-then-destroy-everything" pattern. 
   * It can be adapted to allocator-compatible interface via 
   * MemoryArenaAllocator class.
   *
   * MemoryArena allocates memory in chunks, initial chunk size can be set as
   * a parameter to MemoryArena constructor. Size of the next chunk to allocate
   * is computed by GrowthFunc basing on the size of the previous chunk.
   *
   * All the memory allocated by MemoryArena is freed on a call to destructor. */
  template<class GrowthFunc = MulDivAdd<2, 1, 0>, class Allocator = std::allocator<unsigned char> >
  class MemoryArena: public arx::noncopyable {
  public:
    typedef GrowthFunc growth_function;
    typedef typename Allocator::template rebind<unsigned char>::other allocator_type;
    typedef typename allocator_type::value_type      value_type;
    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::const_pointer   const_pointer;
    typedef typename allocator_type::reference       reference;
    typedef typename allocator_type::const_reference const_reference;

    enum {
      defaultInitialSize = 64
    };

    MemoryArena() {
      initialize(defaultInitialSize, mGrowthFunc(defaultInitialSize));
    }

    MemoryArena(allocator_type alloc): mAllocator(alloc) {
      initialize(defaultInitialSize, mGrowthFunc(defaultInitialSize));
    }

    MemoryArena(growth_function growthFunc): mGrowthFunc(growthFunc) {
      initialize(defaultInitialSize, mGrowthFunc(defaultInitialSize));
    }

    MemoryArena(growth_function growthFunc, allocator_type alloc): 
      mGrowthFunc(growthFunc), mAllocator(alloc) {
      initialize(defaultInitialSize, mGrowthFunc(defaultInitialSize));
    }

    MemoryArena(int initialCapacity, int nextBlockCapacity) {
      initialize(initialCapacity, nextBlockCapacity);
    }

    MemoryArena(int initialCapacity, int nextBlockCapacity, allocator_type alloc): mAllocator(alloc) {
      initialize(initialCapacity, nextBlockCapacity);
    }

    MemoryArena(int initialCapacity, int nextBlockCapacity, growth_function growthFunc): mGrowthFunc(growthFunc) {
      initialize(initialCapacity, nextBlockCapacity);
    }

    MemoryArena(int initialCapacity, int nextBlockCapacity, growth_function growthFunc, allocator_type alloc): 
    mGrowthFunc(growthFunc), mAllocator(alloc) {
      initialize(initialCapacity, nextBlockCapacity);
    }

    pointer allocate(size_type count, size_type alignment) {
      assert(alignment >= 1);
      assert(count >= 0 && static_cast<ptrdiff_t>(count) >= 0);
      mCurrentPtr = alignForward(mCurrentPtr, alignment);
      /* Here static_cast is needed, since difference on the left side of "<" 
       * can be negative. We check validity of this conversion in assertion
       * above. */
      if(mEndPtr - mCurrentPtr < static_cast<ptrdiff_t>(count)) {
        nextBlock(count + alignment - 1);
        mCurrentPtr = alignForward(mCurrentPtr, alignment);
      }
      pointer result = mCurrentPtr;
      mCurrentPtr += count;
      return result;
    }

    template<class T>
    pointer allocate(size_type count) {
      return allocate(count * sizeof(T), arx::alignment_of<T>::value);
    }

    void deallocate(pointer /* ptr */, size_type /* count */){
      return;
    }

    pointer address(reference ref) const {
      return mAllocator.address(ref); 
    }

    const_pointer address(const_reference ref) const {
      return mAllocator.address(ref); 
    }

    void clear() {
      for(int i = 0; i < mBlocks.size(); i++)
        mBlocks[i].mSize = 0;
      mIndex = 0;
      mCurrentPtr = mBlocks[0].mPtr;
      mEndPtr = mCurrentPtr + mBlocks[0].mCapacity;
      std::sort(mBlocks.begin(), mBlocks.end(), MemoryBlockCapacityMore());
    }

    ~MemoryArena() {
      for(int i = 0; i < mBlocks.size(); i++)
        mAllocator.deallocate(mBlocks[i].mPtr, mBlocks[i].mCapacity);
    }

    void reserve(size_type requiredCapacity) {
      assert(requiredCapacity >= 0);
      size_type totalCapacity = mBlocks[mIndex].mCapacity - mBlocks[mIndex].mSize;
      for(int i = mIndex + 1; i < mBlocks.size(); i++)
        totalCapacity += mBlocks[i].mCapacity;
      if(totalCapacity < requiredCapacity)
        allocateBlock(requiredCapacity - totalCapacity);
    }

    allocator_type& allocator() {
      return mAllocator;
    }

    const allocator_type& allocator() const {
      return mAllocator;
    }

    growth_function& growthFunc() {
      return mGrowthFunc;
    }

    const growth_function& growthFunc() const {
      return mGrowthFunc;
    }

    size_type nextBlockCapacity() const {
      return mNextBlockCapacity
    }

    void setNextBlockCapacity(size_type capacity) {
      assert(capacity > 0);
      mNextBlockCapacity = capacity;
    }

  private:
    struct MemoryBlock {
      pointer mPtr;        /**< Pointer to the beginning of the block */
      size_type mCapacity; /**< Capacity of the block, in bytes. */
      size_type mSize;     /**< Size of the block, i.e. number of allocated bytes in it. */
    };

    struct MemoryBlockCapacityMore {
      bool operator()(const MemoryBlock& a, const MemoryBlock& b) const {
        return a.mCapacity > b.mCapacity;
      }
    };

    pointer alignForward(pointer ptr, size_type alignment) {
#ifdef ARX_MSVC
#  pragma warning(push)
#  pragma warning(disable: 4146) /* warning C4146: unary minus operator applied to unsigned type, result still unsigned */
#endif 
      return reinterpret_cast<pointer>((reinterpret_cast<intptr_t>(ptr) + alignment - 1) & -alignment);
#ifdef ARX_MSVC
#  pragma warning(pop)
#endif
    }

    void initialize(int initialCapacity, int nextBlockCapacity) {
      assert(initialCapacity >= 0 && nextBlockCapacity >= 1);

      mNextBlockCapacity = nextBlockCapacity;
      mIndex = 0;
      mBlocks.reserve(16);

      allocateBlock(initialCapacity);
      mCurrentPtr = mBlocks[0].mPtr;
      mEndPtr = mCurrentPtr + mBlocks[0].mCapacity;
    }

    void nextBlock(size_type neededCapacity) {
      /* Update current block first. */
      mBlocks[mIndex].mSize = mCurrentPtr - mBlocks[mIndex].mPtr;

      /* Then get a new one. 
       * Note that if the current block is already too small, we don't need to 
       * iterate through the rest ones since they are sorted in 
       * capacity-descending order. */
      mIndex++;
      if(mIndex >= mBlocks.size() || mBlocks[mIndex].mCapacity < neededCapacity) {
        /* No blocks for reuse - allocate new one. */
        mIndex = mBlocks.size();
        allocateBlock((neededCapacity > mNextBlockCapacity) ? neededCapacity : mNextBlockCapacity);
        mNextBlockCapacity = static_cast<size_type>(
          mGrowthFunc(static_cast<typename growth_function::argument_type>(mNextBlockCapacity))
        );
      }

      mCurrentPtr = mBlocks[mIndex].mPtr;
      mEndPtr = mCurrentPtr + mBlocks[mIndex].mCapacity;

      assert(mBlocks[mIndex].mSize == 0 && mBlocks[mIndex].mCapacity >= neededCapacity);
    }

    void allocateBlock(size_type capacity) {
      MemoryBlock newBlock;
      newBlock.mPtr = mAllocator.allocate(capacity);
      newBlock.mCapacity = capacity;
      newBlock.mSize = 0;
      mBlocks.push_back(newBlock);
    }

    allocator_type mAllocator;
    arx::CheckedArray<MemoryBlock> mBlocks;
    pointer mCurrentPtr;
    pointer mEndPtr;
    int mIndex;
    size_type mNextBlockCapacity;
    growth_function mGrowthFunc;
  };

  /** Allocator adapter to use with memory arena. 
   * 
   * Is alignment-aware, i.e. all the object allocated with this allocator
   * are properly aligned, as it is required by the given type. To determine
   * alignment, alignment_of metafunction is used. */
  template<class Type, class MemoryArena>
  class MemoryArenaAllocator {
  public:
    typedef Type value_type;
    typedef MemoryArena arena_type;
    typedef typename arena_type::allocator_type::template rebind<Type>::other allocator_type;
    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::const_pointer   const_pointer;
    typedef typename allocator_type::reference       reference;
    typedef typename allocator_type::const_reference const_reference;

    MemoryArenaAllocator(): mArena(NULL) {}

    MemoryArenaAllocator(arena_type& arena): mArena(&arena) {}

    MemoryArenaAllocator(const MemoryArenaAllocator<value_type, MemoryArena>& that) {
      mArena = that.mArena;
    }

    template<class OtherType>
    MemoryArenaAllocator(const MemoryArenaAllocator<OtherType, MemoryArena>& that) {
      mArena = that.mArena;
    }

    template<class OtherType>
    struct rebind {
      typedef MemoryArenaAllocator<OtherType, MemoryArena> other;
    };

    pointer address(reference ref) const {
      return reinterpret_cast<pointer>(
        mArena->address(reinterpret_cast<typename arena_type::reference>(ref))
      );
    }

    const_pointer address(const_reference _Val) const {
      return reinterpret_cast<const_pointer>(
        mArena->address(reinterpret_cast<typename arena_type::const_reference>(ref))
      );
    }

    void deallocate(pointer ptr, size_type count) {
      mArena->deallocate(reinterpret_cast<typename arena_type::pointer>(ptr), size);
    }

    pointer allocate(size_type count) {
      return reinterpret_cast<pointer>(mArena->allocate<value_type>(count));
    }

    pointer allocate(size_type count, const void* /* hint */) {
      return allocate(count);
    }

    void construct(pointer ptr, const value_type& refObj) {
      allocator_type(mArena->allocator()).construct(ptr, refObj);
    }

    void destroy(pointer ptr) {
      allocator_type(mArena->allocator()).destroy(ptr);
    }

    size_type max_size() const {
      return allocator_type(mArena->allocator()).max_size();
    }

    template<class OtherType> 
    bool operator==(const MemoryArenaAllocator<OtherType, MemoryArena>& that) {
      mArena == that.mArena;
    }

    template<class OtherType> 
    bool operator!=(const MemoryArenaAllocator<OtherType, MemoryArena>& that) {
      return !(*this == that);
    }

    arena_type& arena() {
      return *mArena;
    }

    const arena_type& arena() const {
      return *mArena;
    }

    void setArena(arena_type& arena) {
      mArena = &arena;
    }

  private:
    arena_type* mArena;
  };


} // namespace smart

#endif // __SMART_MEMORYARENA_V2_H__
