#ifndef __SMART_TRIANGLE_H__
#define __SMART_TRIANGLE_H__

#include "common.h"
#include "BoundingBox.h"

namespace smart {

  namespace detail {
// -------------------------------------------------------------------------- //
// TriangleStorage
// -------------------------------------------------------------------------- //
    class TriangleStorage {
    public:
      TriangleStorage(const Vector3f& coord0, const Vector3f& coord1, const Vector3f& coord2) {
        mCoords[0] = coord0;
        mCoords[1] = coord1;
        mCoords[2] = coord2;
      }

      TriangleStorage(const Vector3f& coord0, const Vector3f& coord1, const Vector3f& coord2,
                      const Vector3f& normal0, const Vector3f& normal1, const Vector3f& normal2,
                      const Vector3f& texCoord0, const Vector3f& texCoord1, const Vector3f& texCoord2) {
        mCoords[0] = coord0;
        mCoords[1] = coord1;
        mCoords[2] = coord2;
        mNormals[0] = normal0;
        mNormals[1] = normal1;
        mNormals[2] = normal2;
        mTexCoords[0] = texCoord0;
        mTexCoords[1] = texCoord1;
        mTexCoords[2] = texCoord2;
      }

      const Vector3f& getCoord(int vertexIndex) const {
        return mCoords[vertexIndex];
      }

      const Vector3f& getNormal(int vertexIndex) const {
        return mNormals[vertexIndex];
      }

      const Vector3f& getTexCoord(int vertexIndex) const {
        return mTexCoords[vertexIndex];
      }

    private:
      Vector3f mCoords[3];
      Vector3f mNormals[3];
      Vector3f mTexCoords[3];
    };


// -------------------------------------------------------------------------- //
// DynamicTriangleVertex
// -------------------------------------------------------------------------- //
    template<class TriangleType>
    class DynamicTriangleVertex {
    public:
      DynamicTriangleVertex(const TriangleType& triangle, int vertexIndex):
        mTriangle(triangle), mVertexIndex(vertexIndex) {}

      const Vector3f& getCoord() const {
        return mTriangle.getCoord(mVertexIndex);
      }

      const Vector3f& getNormal() const {
        return mTriangle.getNormal(mVertexIndex);
      }

      const Vector3f& getTexCoord() const {
        return mTriangle.getTexCoord(mVertexIndex);
      }

    private:
      const TriangleType& mTriangle;
      int mVertexIndex;
    };


// -------------------------------------------------------------------------- //
// StaticTriangleVertex
// -------------------------------------------------------------------------- //
    template<class TriangleType, int vertexIndex>
    class StaticTriangleVertex {
    public:
      StaticTriangleVertex(const TriangleType& triangle):
        mTriangle(triangle) {}

      const Vector3f& getCoord() const {
        return mTriangle.getCoord<vertexIndex>();
      }

      const Vector3f& getNormal() const {
        return mTriangle.getNormal<vertexIndex>();
      }

      const Vector3f& getTexCoord() const {
        return mTriangle.getTexCoord<vertexIndex>();
      }

    private:
      STATIC_ASSERT((vertexIndex >= 0 && vertexIndex <= 2));

      const TriangleType& mTriangle;
    };


// -------------------------------------------------------------------------- //
// TriangleAdapter
// -------------------------------------------------------------------------- //
    template<class TriangleType, bool storeByValue = true>
    class TriangleAdapter {
    public:
      TriangleAdapter(const TriangleType& triagle): mTriangle(triagle) {}

      const Vector3f& getCoord(int vertexIndex) const {
        return mTriangle.getCoord(vertexIndex);
      }

      const Vector3f& getNormal(int vertexIndex) const {
        return mTriangle.getNormal(vertexIndex);
      }

      const Vector3f& getTexCoord(int vertexIndex) const {
        return mTriangle.getTexCoord(vertexIndex);
      }

      const detail::DynamicTriangleVertex<TriangleAdapter> getVertex(int vertexIndex) const {
        return detail::DynamicTriangleVertex<TriangleAdapter>(*this, vertexIndex);
      }

      template<int vertexIndex>
      const Vector3f& getCoord() const {
        return mTriangle.getCoord(vertexIndex);
      }

      template<int vertexIndex>
      const Vector3f& getNormal() const {
        return mTriangle.getNormal(vertexIndex);
      }

      template<int vertexIndex>
      const Vector3f& getTexCoord() const {
        return mTriangle.getTexCoord(vertexIndex);
      }

      template<int vertexIndex>
      const detail::StaticTriangleVertex<TriangleAdapter, vertexIndex> getVertex() const {
        return detail::StaticTriangleVertex<TriangleAdapter, vertexIndex>(*this);
      }

      BoundingBox computeBoundingBox() const {
        return BoundingBox(
          getCoord<0>().cwise().min(getCoord<1>().cwise().min(getCoord<2>())),
          getCoord<0>().cwise().max(getCoord<1>().cwise().max(getCoord<2>()))
        );
      }

    private:
      typename arx::if_c<storeByValue, TriangleType, const TriangleType&>::type mTriangle;
    };

  } // namespace detail


// -------------------------------------------------------------------------- //
// Triangle
// -------------------------------------------------------------------------- //
  class Triangle: public detail::TriangleAdapter<detail::TriangleStorage, true> {
  private:
    typedef detail::TriangleAdapter<detail::TriangleStorage, true> base_type;
  public:
    Triangle(const Vector3f& coord0, const Vector3f& coord1, const Vector3f& coord2):
      base_type(detail::TriangleStorage(coord0, coord1, coord2)) {}

    Triangle(const Vector3f& coord0, const Vector3f& coord1, const Vector3f& coord2,
             const Vector3f& normal0, const Vector3f& normal1, const Vector3f& normal2,
             const Vector3f& texCoord0, const Vector3f& texCoord1, const Vector3f& texCoord2):
      base_type(detail::TriangleStorage(coord0, coord1, coord2, 
                                        normal0, normal1, normal2,
                                        texCoord0, texCoord1, texCoord2)) {}
  };

} // namespace smart

#endif // __SMART_TRIANGLE_H__
