#ifndef __SMART_BSPTREE_H__
#define __SMART_BSPTREE_H__

#include "common.h"
#include <cassert>
#include <algorithm>
#include <arx/Collections.h>
#include <arx/Utility.h>
#include <arx/static_assert.h>
#include <arx/Memory.h>
#include "BoundingBox.h"
#include "MemoryArena.h"

namespace smart {
// -------------------------------------------------------------------------- //
// NodeTriangleIdList
// -------------------------------------------------------------------------- //
  /** Wrapper class for list of triangle ids stored in a leaf node. */
  class NodeTriangleIdList {
  public:
    int operator[] (int index) const {
      assert(0 <= index && index < mSize);
      return mIndices[index];
    }

    int size() const {
      return mSize;
    }

  private:
    friend class BspNode;

    NodeTriangleIdList(int size, const int* indices):
      mSize(size), mIndices(indices) {}

    int mSize;
    const int* mIndices;
  };


// -------------------------------------------------------------------------- //
// BspNode
// -------------------------------------------------------------------------- //
  /** Single node of a BSP tree. 
   *
   * Always aligned on 8-byte boundaries. Aligned on 16-bytes boundaries when 
   * allocated with operator new (as required by structure used in BspTree).
   * Brother nodes are always placed near each other, with the left one located
   * on the 16-bytes boundary. */
  ALIGN(8) class BspNode {
  private:
    /* My bithacks rely on this. */
    STATIC_ASSERT((sizeof(intptr_t) == sizeof(int)));

  public:
    struct LEAF {};
    struct INNER {};

    BspNode() {}

    BspNode(const LEAF& /* type */, int triangleListSize, const int* triangleList) {
      assert((reinterpret_cast<intptr_t>(triangleList) & MASK_FLAG) == 0);
      mAxisFlagPtr = reinterpret_cast<intptr_t>(triangleList);
      mIndexListSize = triangleListSize;
    }

    BspNode(const INNER& /* type */, int splitAxis, float splitPosition, BspNode* leftChild) {
      assert((reinterpret_cast<intptr_t>(leftChild) & (MASK_FLAG | MASK_SPLIT_DIM)) == 0);
      assert(splitAxis >= 0 && splitAxis <= 2);
      mAxisFlagPtr = 
        reinterpret_cast<intptr_t>(leftChild) | (splitAxis << 1) | 1;
      mSplitPosition = splitPosition;
    }


    /** @returns true if this node is a leaf, false otherwise. */
    bool isLeaf() const {
      return (mAxisFlagPtr & MASK_FLAG) == 0;
    }

    /** @returns splitting dimension of this inner node plus shift modulo 3.
     * Note that splitting dimension is returned as an index of the axis,
     * perpendicular to the splitting plane, i.e. 0 for yz, 1 for xz, 2 for xy. */
    template<int shift>
    int getDim() const {
      STATIC_ASSERT((shift >= 0 && shift <= 2));
      assert(!isLeaf());

      /* A quick note on this black magic. We don't want to perform shifts here,
       * so instead we use shifted dimension indices to access our lookup table.
       * The only thing left is to build such a table, and think of how to 
       * access it.
       *
       * When accessing a table, since the original dimension index is stored
       * shifted, we get index 0 for original 0, 2 for original 1, and 4 for 
       * original 2. It gives the following layout:
       *
       * <code>{0,  , 1,  , 2,  ,  ,  }</code>
       *
       * We need to handle shift values from 0 to 2. On original shift of 0
       * we can take array shift as zero. Taking array shift of 2 for
       * original shift of 1 is also correct, if we add one element to our
       * array:
       *
       * <code>{0,  , 1,  , 2,  , 0,  }</code>
       *
       * Then the only one shift left to handle is the shift of 2. We can use
       * array shift of 1 for it, since we have free elements in our array
       * on that shift. It gives the following layout:
       *
       * <code>{0, 2, 1, 0, 2, 1, 0,  }</code>
       *
       * So, we have the following mapping for shift:
       *
       * <tt>
       * 0 -> 0
       * 1 -> 2
       * 2 -> 1
       * </tt>
       *
       * If we replace here <tt>0 -> 0</tt> with <tt>0 -> 3</tt>, then this
       * mapping could be expressed with simple expression 
       * <code>shift - 3</code>. Looking on the structure of our array we 
       * see, that we can do it. This gives the following layout:
       *
       * <code>{ , 2, 1, 0, 2, 1, 0, 2}</code>
       *
       * That's exactly the layout you see below. */
      ALIGN(SMART_CACHELINE) static const unsigned int sSplitLookup[8] = 
        {0xFFFFFFFF, 2, 1, 0, 2, 1, 0, 2};
      return sSplitLookup[(mAxisFlagPtr & MASK_SPLIT_DIM) + (3 - shift)];
    }

    /** @returns splitting dimension of this inner node. */
    int getSplitDim() const {
      return getDim<0>();
    }

    /** @returns a split plane coordinate of this inner node. */
    float getSplitCoord() const {
      assert(!isLeaf());
      return mSplitPosition;
    }

    /** @returns the left child of this inner node. */
    const BspNode* getLeftChild() const {
      assert(!isLeaf());
      return reinterpret_cast<const BspNode*>(mAxisFlagPtr & MASK_POINTER);
    }

    /** @returns the right child of this inner node. */
    const BspNode* getRightChild() const {
      return getLeftChild() + 1;
    }

    /** @returns a triangle indices list corresponding to this leaf node. */
    NodeTriangleIdList getTriangleIndexList() const {
      assert(isLeaf());
      return NodeTriangleIdList(mIndexListSize, reinterpret_cast<const int*>(mAxisFlagPtr));
    }

  private:
    enum {
      MASK_FLAG = 0x1,
      MASK_SPLIT_DIM = 0x6,
      MASK_POINTER = 0xFFFFFFF8
    };

    /** This field has different structure for leaf and inner nodes.
     *
     * For inner nodes:
     * bit 0      : 1;
     * bits 1..2  : splitting dimension;
     * bits 3..31 : pointer to first child.
     *
     * For leaf nodes:
     * bits 0..1  : 0;
     * bits 2..31 : pointer to triangle list.
     *
     * Bit 0 is obviously a flag, which determines the type of the node. 
     *
     * Now I guess I need to explain, why this magic works. 
     *
     * For inner nodes we store a pointer to another node, which is always 
     * 8-byte aligned, therefore three lowermost bits of such a pointer are 
     * always zeros. That's why we can use them for our purposes.
     *
     * For leaf nodes we store a pointer to triangle list, which is always
     * 4-byte aligned, i.e. its two lowermost bits are always zeros. Since 
     * we've picked zero for leaf node flag, we don't even need to apply a bit 
     * mask - what is stored in this field is exactly the pointer.
     *
     * We store splitting dimension by shifting it one bit. Therefore, we get
     * 0 for x, 2 for y, and 4 for z. We can transform them into normal 0-1-2 
     * indexes using a static array - we were going to use it anyway for fast 
     * modulo 3 division. The only problem is that such a trick adds one 
     * additional indirection for computing the actual splitting plane 
     * dimension. This needs more testing.
     *
     * Another thing - we probably want to call prefetch as soon as possible.
     * Even before we know the actual type of the node. The problem is that
     * bit mask for pointer extraction differs for different node types. But 
     * even if we use bitmask for inner node, we won't miss for more that 
     * 4 bytes, which is not a miss actually. Therefore, prefetch is not 
     * affected by such a bit layout.
     *
     * TODO: try using Wald's structure. */
    intptr_t mAxisFlagPtr;

    union {
      /** Coordinate of the splitting plane. Valid only for inner nodes. */
      float mSplitPosition;

      /** Number of triangles in the list corresponding to this leaf node. */
      int mIndexListSize;
    };

    /** Lookup table for splitting plane extraction */
    ALIGN(SMART_CACHELINE) static const unsigned int sSplitLookup[];
  };


// -------------------------------------------------------------------------- //
// BspTree
// -------------------------------------------------------------------------- //
  /** Binary Space Partitioning tree with axis-aligned splitting planes, 
   * optimized for fast tracing. */
  class BspTree: private arx::noncopyable {
  public:
#if 0
    class ClipperInterface {
    public:
      /* This one should return a bounding box of an intersection of the
       * given object with the given bounding box. */
      template<class Object>
      BoundingBox operator()(const Object&, const BoundingBox&);

      template<class Object>
      BoundingBox operator()(const Object&);
    };
#endif

    /** Default Constructor.
     * Constructs an uninitialized BSP Tree, which cannot be used. */
    BspTree() {
      mNodePairAllocator.setArena(mArena);
      mIntAllocator.setArena(mArena);
      mCompiled = false;
    }

    /** Compiles the BSP Tree. */
    template<class ObjectArray, class Clipper>
    void compile(const ObjectArray& objects, Clipper clipper) {
      compile(objects, clipper, FakeBbArray<ObjectArray, Clipper>(objects, clipper));
    }

    template<class ObjectArray, class Clipper, class BoundingBoxArray>
    void compile(const ObjectArray& objects, Clipper clipper, const BoundingBoxArray& boundingBoxes) {
      assert(objects.size() > 0);
      assert(objects.size() == boundingBoxes.size());

      /* Reserve memory for tree structure. */
      int minMemoryNeeded = sahEstimateMinMemoryNeeded(objects.size());
      int maxMemoryNeeded = sahEstimateMaxMemoryNeeded(objects.size());
      mArena.reserve(minMemoryNeeded);
      mArena.setNextBlockCapacity(
        std::min(1024 * 1024, (maxMemoryNeeded - minMemoryNeeded) / 4 + 1)
      );

      /* Build context. */
      SahContext<ObjectArray, Clipper> ctx(objects, clipper);
      ctx.classes.resize(objects.size());
      arx::FastArray<Event> events;
      events.reserve(sahEstimateEventListSize(objects.size()));

      /* Build event list and global bounding box. */
      mBoundingBox = BoundingBox::empty();
      for(int i = 0; i < objects.size(); i++) {
        BoundingBox boundingBox = boundingBoxes[i];
        mBoundingBox.extend(boundingBox);

        sahGenerateEvents(events, boundingBox, i);
      }
      std::sort(events.begin(), events.end(), EventSahComparer());

      /* Allocate root. */
      mRoot = &mNodePairAllocator.allocate(1)->child[CLASS_R];

      /* Recurse. Note that we cannot pass the bounding box which is stored in
       * a field of our class, since node construction routine modifies the
       * given one. */
      BoundingBox boundingBox = mBoundingBox;
      constructNode(
        ctx, mRoot, arx::ArrayTail<arx::FastArray<Event> >(events, 0), 
        objects.size(), boundingBox, 0
      );

      /* We're done. */
      mCompiled = true;
    }

    /** @returns root node of this BSP tree. */
    const BspNode* getRoot() const {
      return mRoot;
    }

    /** @returns root node of this BSP tree. */
    BspNode* getRoot() {
      return mRoot;
    }

    /** @returns bounding box of the triangle structure that this BSP tree was 
     *    built upon. */
    const BoundingBox& getBoundingBox() const {
      assert(mCompiled);
      return mBoundingBox;
    }

  private:
    /* TODO: check whether align(16) is really needed here... */
    ALIGN(16) struct NodePair {
      BspNode child[2]; /* Indexed by ObjectClass */
    };

    /** ObjectClass enumeration is used for object classification during SAH
     * BSP tree compilation. It represents the side relative to the split
     * plane, on which an object will be after the split is performed. */
    enum ObjectClass {
      CLASS_L = 0,
      CLASS_R = 1,
      CLASS_B,
    };

    /** Event structure represents a single event used in SAH BSP tree 
     * construction. An event is basically a potential split plane position,
     * with some additional information attached to it. This additional 
     * information include the index of an object from which the event was
     * generated, and the type of an event - whether it was generated at
     * the point of that object with the minimal coordinate (BEGIN event),
     * or with the maximal coordinate (END event). For flat objects only
     * one event of type FLAT is generated instead of BEGIN & END pair. */
    struct Event {
      /* Type of an event. */
      enum EventType {
        END,
        FLAT,
        BEGIN
      };

      Event() {}

      Event(int dim, EventType type, float pos, int index):
        dim(dim), type(type), pos(pos), index(index) {}

      int dim;        /**< Index of the potential split plane dimension. */
      float pos;      /**< Position of the potential split plane dimension. */
      EventType type; /**< Event type. */
      int index;      /**< Index of an object from which this event was generated. */
    };

    /** Comparer class used in sorting of event lists in SAH compilation
     * implementation. */
    struct EventSahComparer {
      bool operator()(const Event& a, const Event& b) {
        if(a.pos == b.pos) {
          if(a.dim == b.dim) {
            return a.type < b.type;
          } else
            return a.dim < b.dim;
        } else
          return a.pos < b.pos;
      }
    };

    /** FakeBbArray class provides an array-like interface to access bounding
     * boxes of the list of given objects, for which bounding boxes are
     * calculated dynamically.
     *
     * This class allows to use the same code for tree building in case the
     * list of bounding boxes is not provided to the compilation routine 
     * without allocation of any additional storage for these bounding boxes. */
    template<class ObjectArray, class Clipper> class FakeBbArray {
    public:
      typedef typename ObjectArray::size_type size_type;

      FakeBbArray(const ObjectArray& objects, const Clipper& clipper):
        mObjects(objects), mClipper(clipper) {}

      BoundingBox operator[] (int index) const {
        return mClipper(mObjects[index]);
      }

      size_type size() const {
        return mObjects.size();
      }

    private:
      const ObjectArray& mObjects;
      const Clipper mClipper;
    };


    /** Single structure holding all the temporaries used during SAH BSP tree
     * construction. */
    template<class ObjectArray, class Clipper>
    struct SahContext {
      SahContext(const ObjectArray& objects, Clipper clipper):
        objects(objects), clipper(clipper) {}

      const ObjectArray& objects;
      Clipper clipper;
      arx::FastArray<ObjectClass> classes;
      arx::FastArray<Event> oldEvents[2];
      arx::FastArray<Event> newEvents[2];
    };

    /**
     * @param nodeCount count of nodes in BSP tree.
     * @param nonEmptyLeafCount number of non-empty leaf nodes in BSP tree.
     * @param objectsPerNonEmptyLeaf average number of objects per non-empty leaf node.
     * @returns an estimated amount of memory needed to store a compiled BSP 
     *   tree with the given parameters. */
    int estimateMemoryNeeded(int nodeCount, int nonEmptyLeafCount, float objectsPerNonEmptyLeaf) {
      int indexListSize = static_cast<int>(nonEmptyLeafCount * objectsPerNonEmptyLeaf);
      return
        nodeCount * sizeof(BspNode) + 
        indexListSize * sizeof(int) + 
        nonEmptyLeafCount * arx::alignment_of<NodePair>::value / 2;
    }

    /** 
     * @param n number of objects.
     * @returns an estimated minimal amount of memory needed to store a
     *   SAH compiled BSP tree for given number of objects.*/
    int sahEstimateMinMemoryNeeded(int n) {
      return estimateMemoryNeeded(3 * n, n, 2);
    }

    /** 
     * @param n number of objects.
     * @returns an estimated maximal amount of memory needed to store a
     *   SAH compiled BSP tree for given number of objects.*/
    int sahEstimateMaxMemoryNeeded(int n) {
      return estimateMemoryNeeded(10 * n, 3 * n, 3);
    }

    /**
     * @param n number of objects.
     * @returns an estimated size of event list stack, used during SAH
     *   compilation of BSP tree. */
    int sahEstimateEventListSize(int n) {
      /* Every object spawns no more than six events. Then, event list 
       * is processed recursively, being split in two on every next 
       * recursion level. Our hope is that the longest list chain will be 
       * no longer than twice the size of the initial list. */
      return 6 * 2 * n;
    }
/*
    template<class A>
    void out(const A& events, int cnt) {
      std::cout << "size = " << events.size() << std::endl;
      for(int i = 0; i < events.size(); i++)
        std::cout << (events[i].dim == 0 ? "x" : (events[i].dim == 1 ? "y" : "z")) << " " <<
        (events[i].type == Event::BEGIN ? "<" : (events[i].type == Event::END ? ">" : "|")) << " " <<
        events[i].pos << " " << events[i].index << "-" << cnt << std::endl;
      std::cout << std::endl;
      std::cout << std::endl;
    }
*/
    template<class ObjectArray, class Clipper>
    void constructNode(SahContext<ObjectArray, Clipper>& ctx, BspNode* node, 
                       arx::ArrayTail<arx::FastArray<Event> > events, 
                       int objectCount, BoundingBox& boundingBox, int currentDepth) {
      /* Check depth first. */
      if(currentDepth >= SMART_MAX_BSPTREE_DEPTH) {
        constructLeaf(node, events, objectCount);
        return;
      }

      /* Precompute parameters for sah test. 
       * Note that there is no "2" term here, because the only way these values
       * are used is multiplication, and therefore these "2" terms would have
       * been eaten by each other anyway %). */
      Vector3f extent = boundingBox.getExtent();
      float invSurfaceArea = 
        1 / (extent[0] * extent[1] + extent[1] * extent[2] + extent[2] * extent[0]);
      Vector3f sideLengths = 
        Vector3f(extent[1] + extent[2], extent[2] + extent[0], extent[0] + extent[1]);

      /* Count of objects to the left and to the right of current plane 
       * respectively. One number for each dimension. Note that single object 
       * may contribute to both of these numbers. */
      Vector3i nL, nR;

      /* Initially all objects are to the right. */
      nL.setZero();
      nR.setConstant(objectCount);

#ifdef DEBUG
      int realCount = 0;
      for(int i = 0; i < events.size(); i++)
        if(events[i].dim == 0 && events[i].type != Event::END)
          realCount++;
      assert(realCount == objectCount);
#endif

      /* Are we dealing with "flat" node? */
      bool isFlatNode = 
        ((boundingBox.getMax() - boundingBox.getMin()).cwise() < SMART_BSPSAH_SAMEPOS_EPS).any();

      /* Best split parameters. */
      float bestPos;
      int bestDim;
      ObjectClass bestFlatClass;
      float bestCost = std::numeric_limits<float>::max();
      int bestN[2]; /* Indexed by class. */

      /* Find best split. */
      for(int i = 0; i < events.size();) {
        /* Current splitting plane position. */
        int dim = events[i].dim;
        float pos = events[i].pos;

        /* Count number of events of three different kinds on the current plane. */
        int pL = 0, pF = 0, pR = 0;
        while(i < events.size() && events[i].dim == dim && events[i].pos == pos && events[i].type == Event::END)
          { pL++; i++; }
        while(i < events.size() && events[i].dim == dim && events[i].pos == pos && events[i].type == Event::FLAT)
          { pF++; i++; }
        while(i < events.size() && events[i].dim == dim && events[i].pos == pos && events[i].type == Event::BEGIN)
          { pR++; i++; }

        /* Then update corresponding object counts - every object that ends
         * on the current plane, or lies on the current plane, doesn't lie
         * on the right anymore. */
        nR[dim] -= pF + pL;

        /* We don't need long thin noodles, as well as we don't want to slice
         * flat cells into the ones which are even flatter. */
        if(!isFlatNode || (pos - boundingBox.getMin(dim) > SMART_BSPSAH_SAMEPOS_EPS && boundingBox.getMax(dim) - pos > SMART_BSPSAH_SAMEPOS_EPS)) {
          /* Apply SAH. */
          ObjectClass flatClass;
          float cost = sahCost(boundingBox.getMin(dim), boundingBox.getMax(dim), pos, 
            nL[dim], pF, nR[dim], invSurfaceArea, sideLengths[dim], flatClass);

          /* Store best. */
          if(cost < bestCost) {
            bestCost = cost;
            bestDim = dim;
            bestPos = pos;
            bestFlatClass = flatClass;
            bestN[CLASS_L] = nL[dim];
            bestN[CLASS_R] = nR[dim];
            bestN[bestFlatClass] += pF;
          }
        }

        /* Update object counts for the next step - every object that lies
         * on the current plane or begins on the current plane, from now on 
         * also lies on the left. */
        nL[dim] += pR + pF;
      }

      /* We have best split, but maybe it's too expensive and it's better to
       * terminate instead? */
      if(sahCostLeaf(objectCount) < bestCost) {
        constructLeaf(node, events, objectCount);
        return;
      }

      /* Ok, we have to split more. Time to classify objects. */
      for(int i = 0;;) {
        for(; i < events.size() && events[i].pos < bestPos; i++) {
          if(events[i].dim == bestDim) {
            if(events[i].type == Event::BEGIN) {
              ctx.classes[events[i].index] = CLASS_B;
            } else {
              ctx.classes[events[i].index] = CLASS_L;
            }
          }
        }
        for(; i < events.size() && events[i].pos == bestPos; i++) {
          if(events[i].dim == bestDim) {
            if(events[i].type == Event::END) {
              ctx.classes[events[i].index] = CLASS_L;
            } else if(events[i].type == Event::FLAT) {
              ctx.classes[events[i].index] = bestFlatClass;
            } else { /* It's BEGIN event. */
              ctx.classes[events[i].index] = CLASS_R;
            }
          }
        }
        for(; i < events.size(); i++)
          if(events[i].dim == bestDim)
            if(events[i].type != Event::END) /* i.e. FLAT or BEGIN. */
              ctx.classes[events[i].index] = CLASS_R;
        break;
      }

      /* Build bounding boxes for child nodes - we'll use them soon. */
      BoundingBox& leftBoundingBox = boundingBox;
      BoundingBox rightBoundingBox = boundingBox;
      leftBoundingBox.setMax(bestDim, bestPos);
      rightBoundingBox.setMin(bestDim, bestPos);

      /* Then it's time to split events in left and right ones, splitting old
       * ones where it's needed. 
       *
       * Note that we multiply count by six in reserve calls here because each 
       * triangle may have at most six associated events. */
      int splittingObjectCount = bestN[CLASS_L] + bestN[CLASS_R] - objectCount;
      ctx.oldEvents[CLASS_L].reserve(6 * (bestN[CLASS_L] - splittingObjectCount));
      ctx.oldEvents[CLASS_R].reserve(6 * (bestN[CLASS_R] - splittingObjectCount));
      ctx.newEvents[CLASS_L].reserve(6 * splittingObjectCount);
      ctx.newEvents[CLASS_R].reserve(6 * splittingObjectCount);

      for(int i = 0; i < events.size(); i++) {
        if(ctx.classes[events[i].index] == CLASS_B) {
          if(events[i].dim == 0 && events[i].type != Event::END) {
            /* If object is on both sides, then it must be split.
             * Note that only BEGIN events can get here since FLAT events 
             * cannot be of CLASS_B */
            sahGenerateEvents(
              ctx.newEvents[CLASS_L], 
              ctx.clipper(ctx.objects[events[i].index], leftBoundingBox), 
              events[i].index
            );
            sahGenerateEvents(
              ctx.newEvents[CLASS_R], 
              ctx.clipper(ctx.objects[events[i].index], rightBoundingBox), 
              events[i].index
            );
          }
        } else {
          /* If object is on one of the sides - than we don't touch it. */
          ctx.oldEvents[ctx.classes[events[i].index]].push_back(events[i]);
        }
      }

      /* Sort lists of newly created events as they are unsorted now. */
      std::sort(ctx.newEvents[CLASS_L].begin(), ctx.newEvents[CLASS_L].end(), EventSahComparer());
      std::sort(ctx.newEvents[CLASS_R].begin(), ctx.newEvents[CLASS_R].end(), EventSahComparer());

      /* OK, now it's time for some magic. We're using a stack-like structure
       * of arrays of events, but instead of using a real stack, we operate
       * on one big array and access it via tail objects. 
       * Since we don't want our big array to grow really big, we merge
       * a bigger resulting array in it first, and a smaller one second,
       * so that we can run the handling of a smaller one first.
       * Smaller one will be handled faster, and when finished, it will free
       * some place for the bigger one. This branching tactics is optimal. */
      int eventCounts[2] = {
        ctx.oldEvents[CLASS_L].size() + ctx.newEvents[CLASS_L].size(),
        ctx.oldEvents[CLASS_R].size() + ctx.newEvents[CLASS_R].size()
      };
      events.reserve(eventCounts[CLASS_L] + eventCounts[CLASS_R]);
      NodePair* children = mNodePairAllocator.allocate(1);

      BoundingBox *childrenBoundingBoxes[2] = {&leftBoundingBox, &rightBoundingBox};
      ObjectClass childOrder[2];
      if(eventCounts[CLASS_L] < eventCounts[CLASS_R]) {
        childOrder[0] = CLASS_L;
        childOrder[1] = CLASS_R;
      } else {
        childOrder[0] = CLASS_R;
        childOrder[1] = CLASS_L;
      }

      /* Resize events to the needed size first. */
      events.resize(eventCounts[CLASS_L] + eventCounts[CLASS_R]);

      /* Merge events of second-to-process child first, and vice-versa. */
      for(int i = 1; i >= 0; i--) {
        std::merge(
          ctx.newEvents[childOrder[i]].begin(), 
          ctx.newEvents[childOrder[i]].end(),
          ctx.oldEvents[childOrder[i]].begin(), 
          ctx.oldEvents[childOrder[i]].end(),
          events.begin() + ((i == 0) ? eventCounts[childOrder[1]] : 0), 
          EventSahComparer()
        );
      }

      /* We don't need old & new event arrays anymore. */
      ctx.newEvents[CLASS_L].clear();
      ctx.oldEvents[CLASS_L].clear();
      ctx.newEvents[CLASS_R].clear();
      ctx.oldEvents[CLASS_R].clear();

      /* Construct node. */
      new (node) BspNode(BspNode::INNER(), bestDim, bestPos, &children->child[CLASS_L]);

      /* Recurse. */
      for(int i = 0; i <= 1; i++) {
        constructNode(
          ctx, 
          &children->child[childOrder[i]], 
          events.tail((i == 0) ? eventCounts[childOrder[1]] : 0), 
          bestN[childOrder[i]], 
          *childrenBoundingBoxes[childOrder[i]],
          currentDepth + 1
        );
      }
    }

    /** Constructs a leaf node.
     * 
     * @param node pointer to node to construct.
     * @param events array of associated events.
     * @param objectCount number of objects in this node. */
    void constructLeaf(BspNode* node, arx::ArrayTail<arx::FastArray<Event> > events, int objectCount) {
      if(objectCount != 0) {
        int* indexList = mIntAllocator.allocate(objectCount);
        int* indexListPtr = indexList;
        for(int i = 0; i < events.size(); i++)
          if(events[i].dim == 0 && events[i].type != Event::END)
            *indexListPtr++ = events[i].index;
        assert(indexListPtr - indexList == objectCount);
        events.clear();
        new (node) BspNode(BspNode::LEAF(), objectCount, indexList);
      } else {
        new (node) BspNode(BspNode::LEAF(), 0, NULL);
      }
    }

    /** For the given bounding box of an objects, generates several events
     * associated with it, outputting them into the given array. 
     *
     * @param dst array to output events to.
     * @param boundingBox bounding box of an object. 
     * @param objectIndex index of an object. */
    template<class ArrayOfEvent>
    void sahGenerateEvents(ArrayOfEvent& dst, const BoundingBox& boundingBox, int objectIndex) {
      assert(!boundingBox.isEmpty());

      for(int k = 0; k < 3; k++) {
        if(boundingBox.getMin(k) == boundingBox.getMax(k)) {
          dst.push_back(Event(k, Event::FLAT,  boundingBox.getMin(k), objectIndex));
        } else {
          dst.push_back(Event(k, Event::BEGIN, boundingBox.getMin(k), objectIndex));
          dst.push_back(Event(k, Event::END,   boundingBox.getMax(k), objectIndex));
        }
      }
    }

    /** Cost function for surface area heuristic. Estimates SAH cost for the
     * given split.
     *
     * @param min minimal coordinate of the current voxel.
     * @param max maximal coordinate of the current voxel.
     * @param pos coordinate of the split plane.
     * @param nL number of objects to the left of the split plane.
     * @param nF number of flat object lying on the split plane.
     * @param nR number of objects to the right of the split plane.
     * @param invSurfaceArea inverse of the surface area of the current voxel.
     * @param sideLength sum of sides of the current voxel, which are
     *   parallel to the splitting plane.
     * @param flatClass (out) ObjectClass for flat objects lying on the split
     *   plane which gives best SAH cost.
     *
     * @returns SAH cost. */
    FORCEINLINE float sahCost(float min, float max, float pos, int nL, int nF, int nR, 
                              float invSurfaceArea, float sideLength, ObjectClass& flatClass) {
      bool favorEmptySpaceCutOff = (pos != min && pos != max);
      float pL = 1 - sideLength * (max - pos) * invSurfaceArea;
      float pR = 1 - sideLength * (pos - min) * invSurfaceArea;
      float costL = sahCostInner(pL, pR, nL + nF, nR, favorEmptySpaceCutOff);
      float costR = sahCostInner(pL, pR, nL, nF + nR, favorEmptySpaceCutOff);

      if(costL < costR) {
        flatClass = CLASS_L;
        return costL;
      } else {
        flatClass = CLASS_R;
        return costR;
      }
    }

    enum {
      sahTraversalCost = 15,
      sahIntersetionCost = 20
    };

    /** Estimates SAH cost for inner, non-leaf node.
     * 
     * @param pL probability of a ray hitting the left child voxel provided
     *   that the enclosing parent voxel is hit.
     * @param pL probability of a ray hitting the right child voxel provided
     *   that the enclosing parent voxel is hit.
     * @param nL number of object in the left child voxel.
     * @param nR number of object in the right child voxel.
     *
     * @returns SAH cost. */
    FORCEINLINE float sahCostInner(float pL, float pR, int nL, int nR, bool favorEmptySpaceCutOff) {
      float result = sahTraversalCost + sahIntersetionCost * (pL * nL + pR * nR);

      /* Favor the splits which cut off empty space. */
      if(favorEmptySpaceCutOff && (nL == 0 || nR == 0))
        result *= 0.8f;

      return result;
    }

    /** Estimates SAH cost for leaf node.
     * 
     * @param n number of object in this node.
     * @returns SAH cost. */
    FORCEINLINE float sahCostLeaf(int n) {
      return sahIntersetionCost * static_cast<float>(n);
    }

    typedef MemoryArena<MulDivAdd<1, 1, 0> > ArenaType;

    ArenaType mArena;
    MemoryArenaAllocator<int, ArenaType> mIntAllocator;
    MemoryArenaAllocator<NodePair, ArenaType> mNodePairAllocator;

    BspNode* mRoot;

    BoundingBox mBoundingBox;

    bool mCompiled;
  };

} // namespace smart

#endif // __SMART_BSPTREE_H__
