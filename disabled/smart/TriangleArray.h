#ifndef __SMART_TRIANGLEARRAY_H__
#define __SMART_TRIANGLEARRAY_H__

#include "common.h"
#include <arx/Collections.h>
#include "Triangle.h"

namespace smart {

  class TriangleArray;

  namespace detail {
// -------------------------------------------------------------------------- //
// TriangleInArray
// -------------------------------------------------------------------------- //
    /** TriangleInArray represents a single triangle stored in TriangleArray.
     * It's adapted class is returned by a call to TriangleArray::getTriangle(int), 
     * and this is the only way it's used. 
     * 
     * Note that only TriangleArray can be used as a template parameter, but 
     * since we want to declare all the methods inside the class definition,
     * we need to pass the actual array type as a template parameter.
     *
     * @param ArrayType type of array to use. */
    template<class ArrayType>
    class TriangleInArray {
    public:
      TriangleInArray(const ArrayType& array, int triangleIndex):
        mArray(array), mTriangleIndex(triangleIndex) {}

      const Vector3f& getCoord(int vertexIndex) const {
        return mArray.mCoords[vertexIndex];
      }

      const Vector3f& getNormal(int vertexIndex) const {
        return mArray.mNormals[vertexIndex];
      }

      const Vector3f& getTexCoord(int vertexIndex) const {
        return mArray.mTexCoords[vertexIndex];
      }

    private:
      const ArrayType& mArray;
      int mTriangleIndex;
    };

  } // namespace detail


// -------------------------------------------------------------------------- //
// TriangleArray
// -------------------------------------------------------------------------- //
  /** TriangleArray class is a storage for geometric triangle data. */
  class TriangleArray: public arx::noncopyable {
  public:
    typedef detail::TriangleAdapter<detail::TriangleInArray<TriangleArray>, true> TriangleType;

    TriangleArray() {}

    TriangleArray(int triangleCapacity, int vertexCapacity) {
      reserve(triangleCapacity, vertexCapacity);
    }

    const TriangleType getTriangle(int index) const {
      return TriangleType(detail::TriangleInArray<TriangleArray>(*this, index));
    }

    int size() const {
      return mIndexedTriangles.size();
    }

    void reserve(int triangleCapacity, int vertexCapacity) {
      mIndexedTriangles.reserve(triangleCapacity);
      mCoords.reserve(vertexCapacity);
      mNormals.reserve(vertexCapacity);
      mTexCoords.reserve(vertexCapacity);
    }

    /** Adds a new vertex to vertex buffer. It is guaranteed that succeeding 
     * calls to addVertex will return succeeding integer identifiers.
     *
     * @param c vertex coordinates.
     * @param n normalized vertex normal.
     * @param t texture coordinates.
     * @returns index of a newly added vertex. */
    int addVertex(const Vector3f& c, const Vector3f& n, const Vector3f& t) {
      assert(abs(n.squaredNorm() - 1) < SMART_EPS);

      /* We don't want our arrays to perform three checks on push_back...
       * Therefore we're using unchecked FastArray and check manually. */
      int capacity = mCoords.capacity();
      if(capacity == mCoords.size()) {
        int newCapacity;
        if(capacity > 48)
          newCapacity = (capacity * 4) / 3; /* TODO: rethink. */
        else
          newCapacity = capacity + 16;
        mCoords.reserve(newCapacity);
        mNormals.reserve(newCapacity);
        mTexCoords.reserve(newCapacity);
      }

      mCoords.push_back(c);
      mNormals.push_back(n);
      mTexCoords.push_back(t);
      return mCoords.size() - 1;
    }

    /** Adds a new triangle to triangle buffer. It is guaranteed that 
     * succeeding calls to addTriangle will return succeeding integer 
     * identifiers.
     * 
     * @param vertexIndex0 index of the first vertex.
     * @param vertexIndex1 index of the secong vertex.
     * @param vertexIndex2 index of the third vertex.
     * @returns index of a newly created triangle. */
    int addTriangle(int vertexIndex0, int vertexIndex1, int vertexIndex2) {
      mIndexedTriangles.push_back(IndexedTriangle(vertexIndex0, vertexIndex1, vertexIndex2));
      return mIndexedTriangles.size() - 1;
    }


  private:
    friend class detail::TriangleInArray<TriangleArray>;

    /** A triangle described by the indices into vertex tables for its three 
     * vertices. */
    class IndexedTriangle {
    public:
      IndexedTriangle(int vertexIndex0, int vertexIndex1, int vertexIndex2) {
        mVertexIndices[0] = vertexIndex0;
        mVertexIndices[1] = vertexIndex1;
        mVertexIndices[2] = vertexIndex2;
      }

      template<int index>
      int getVertexIndex() const { return mVertexIndices[index]; }

      int getVertexIndex(int index) const { return mVertexIndices[index]; }

    private:
      int mVertexIndices[3];
      int shaderId;
    };

    arx::CheckedFastArray<IndexedTriangle> mIndexedTriangles;
    arx::FastArray<Vector3f> mCoords;

    /* TODO move this to model and store in one array */
    arx::FastArray<Vector3f> mNormals;
    arx::FastArray<Vector3f> mTexCoords;
  };

} // namespace smart

#endif // __SMART_TRIANGLEARRAY_H__
