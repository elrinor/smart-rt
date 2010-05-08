#ifndef __SMART_COREMODEL_H__
#define __SMART_COREMODEL_H__

#include "common.h"
#include <cmath>
#include <assert.h>
#include <arx/Collections.h>
#include <arx/Utility.h>
#include <arx/static_assert.h>
#include "ExplicitlyCounted.h"
#include "BspTree.h"
#include "TriAccel.h"
#include "ShaderClass.h"
#include "MemoryArena.h"
#include "Utility.h"
#include "Clipping.h"
#include "ShaderManager.h"
#include "Idded.h"

namespace smart {
// -------------------------------------------------------------------------- //
// CoreModel
// -------------------------------------------------------------------------- //
  /** CoreModel represents a compiled 3d model - including vertex information,
   * triangle acceleration structures, and BSP tree. 
   *
   * Note that it is recommended to distinguish static and dynamic models.
   * Static models do not change from frame to frame. Such models are 
   * normally constructed before the start of rendering cycle, therefore 
   * it's profitable to use sophisticated algorithms for compilation to 
   * speed up rendering.
   * Dynamic models change from frame to frame, and therefore they need 
   * to be recompiled quite often, so that only fast compilation algorithms 
   * can be used.
   */
  class CoreModel: private arx::noncopyable, private ExplicitlyCounted<CoreModel>, public Idded {
  private:
    class TriangleAdapter;

  public:
    typedef TriangleAdapter TriangleType;

    /** @returns BSP tree built for this model. */
    const BspTree& getBspTree() const {
      assert(mCompiled);
      return mBspTree;
    }

    /** @returns bounding box of this model. */
    const BoundingBox& getBoundingBox() const {
      assert(mCompiled);
      return mBspTree.getBoundingBox();
    }

    /** @return the number of triangles in this model. */
    const int getTriangleCount() const {
      return mTriangleData.size();
    }

    /** @returns TriAccel structure for the triangle with the given id. */
    const TriAccel& getTriAccel(int id) const {
      assert(mCompiled);
      return mTriAccels[id];
    }

    /** @returns the triangle with the given id. */
    const TriangleType getTriangle(int id) const {
      return TriangleType(*this, id);
    }

    bool hasVertex(int id) const {
      return 0 <= id && id < mVertexData.size();
    }

    bool hasTriangle(int id) const {
      return 0 <= id && id <= mTriangleData.size();
    }

    const Vector3f& getCoord(int vertexId) const {
      return mVertexData[vertexId].mCoord;
    }

    const Vector3f& getNormal(int vertexId) const {
      return mVertexData[vertexId].mNormal;
    }

    const Vector3f& getTexCoord(int vertexId) const {
      return mVertexData[vertexId].mTexCoord;
    }

    const Vector3f& getCoord(int triangleId, int n) const {
      return getCoord(mTriangleData[triangleId].mVertexId[n]);
    }

    const Vector3f& getNormal(int triangleId, int n) const {
      return getNormal(mTriangleData[triangleId].mVertexId[n]);
    }

    const Vector3f& getTexCoord(int triangleId, int n) const {
      return getTexCoord(mTriangleData[triangleId].mVertexId[n]);
    }

    /** Creates a new storage for Triangle shader parameters for the shader with
     * the given identifier. */
    void* newShaderTriangleParam(SurfaceShaderClass* shaderClass) {
      return static_cast<void*>(
        mShadingParamArena.allocate(shaderClass->getTriangleParamSize(), shaderClass->getTriangleParamAlign())
      );
    }

    /** Creates a new storage for Attribute shader parameters for the shader 
     * with the given identifier. */
    void* newShaderAttribParam(SurfaceShaderClass* shaderClass) {
      return static_cast<void*>(
        mShadingParamArena.allocate(shaderClass->getAttribParamSize(), shaderClass->getAttribParamAlign())
      );
    }

    /** Adds a new triangle to triangle buffer. It is guaranteed that 
     * succeeding calls to newTriangle will return succeeding integer 
     * identifiers.
     * 
     * @param vertexId0 identifier of the first vertex.
     * @param vertexId1 identifier of the second vertex.
     * @param vertexId2 identifier of the third vertex.
     * @param shaderId identifier of a shader to use.
     * @param shadingParam pointer to a location of Triangle shading parameter. 
     *        Use NULL for none.
     * @returns identifier of a newly created triangle. */
    int newTriangle(int vertexId0, int vertexId1, int vertexId2, void* shadingParam) {
      assert(!mCompiled);
      assert(vertexId0 != vertexId1 && vertexId1 != vertexId2 && vertexId2 != vertexId0);
      assert(hasVertex(vertexId0) && hasVertex(vertexId1) && hasVertex(vertexId2));
      assert((getCoord(vertexId0) - getCoord(vertexId1)).squaredNorm() >= SMART_NOT_A_TRIANGLE_EPS &&
             (getCoord(vertexId1) - getCoord(vertexId2)).squaredNorm() >= SMART_NOT_A_TRIANGLE_EPS &&
             (getCoord(vertexId2) - getCoord(vertexId0)).squaredNorm() >= SMART_NOT_A_TRIANGLE_EPS);

      TriangleData triangle;
      triangle.mVertexId[0] = vertexId0;
      triangle.mVertexId[1] = vertexId1;
      triangle.mVertexId[2] = vertexId2;
      triangle.mShadingParam = shadingParam;

      if(mTriangleData.capacity() == mTriangleData.size())
        mTriangleData.reserve(mTriangleData.capacity() * 2 + 16);
      mTriangleData.push_back(triangle);

      return mTriangleData.size() - 1;
    }

    /** Adds a new vertex to vertex buffer. It is guaranteed that succeeding 
     * calls to newVertex will return succeeding integer identifiers.
     *
     * @param coord vertex coordinates.
     * @param normal normalized vertex normal.
     * @param texCoord texture coordinates.
     * @param shadingParam pointer to a location of Attrib shading parameter. 
     *        Use NULL for none.
     * @returns identifier of a newly added vertex. */
    int newVertex(const Vector3f& coord, const Vector3f& normal, const Vector3f& texCoord, void* shadingParam) {
      assert(!mCompiled);

      VertexData vertex;
      vertex.mCoord = coord;
      vertex.mNormal = normal;
      vertex.mTexCoord = texCoord;
      vertex.mShadingParam = shadingParam;
      
      if(mVertexData.capacity() == mVertexData.size())
        mVertexData.reserve(mVertexData.capacity() * 2 + 16);
      mVertexData.push_back(vertex);

      return mVertexData.size() - 1;
    }

    /** Compiles a model - builds ray-triangle intersection test acceleration
     * structures and a BSP tree. */
    void compile() {
      if(mCompiled)
        return;
      
      /* Create TriAccel structures first. */
      mTriAccels.reserve(getTriangleCount());
      for(int i = 0; i < getTriangleCount(); ++i)
        mTriAccels.push_back(TriAccel(getTriangle(i)));

      /* Build BspTree. */
      mBspTree.compile(FakeArray<int>(getTriangleCount()), TriangleClipper(*this));

      /* We're done. */
      mCompiled = true;
    }

    bool isCompiled() const {
      return mCompiled;
    }

  private:
    class TriangleClipper {
    public:
      TriangleClipper(const CoreModel& coreModel): mCoreModel(&coreModel) {}

      BoundingBox operator()(int i, const BoundingBox& boundingBox) const {
        BoundingBox result;
        clip(mCoreModel->getTriangle(i), mCoreModel->mTriAccels[i], boundingBox, result);
        return result;
      }

      BoundingBox operator()(int i) const {
        return triangleBoundingBox(mCoreModel->getTriangle(i));
      }
    private:
      const CoreModel* mCoreModel;
    };

    /** TriangleData structure stores data associated with a triangle. */
    struct TriangleData {
      int mVertexId[3];
      void* mShadingParam;
    };

    /** VertexData structure stores data associated with a vertex. */
    struct VertexData {
      Vector3f mCoord;
      Vector3f mNormal;
      Vector3f mTexCoord;
      void* mShadingParam;
    };

    /** TriangleAdapter class represents a reference to one of the triangles in a model. */
    class TriangleAdapter {
    public:
      TriangleAdapter(const CoreModel& model, int index):
        mModel(model), mIndex(index) {}

      const Vector3f& operator[](int vertexIndex) const {
        assert(vertexIndex >= 0 && vertexIndex < 3);
        return mModel.mVertexData[mModel.mTriangleData[mIndex].mVertexId[vertexIndex]].mCoord;
      }

    private:
      const CoreModel& mModel;
      int mIndex;
    };

    friend class SmartCore;
    friend class ShadedModel;
    friend class TriangleClipper;

    /** Constructor. 
     *
     * @param id identifier of this model. */
    CoreModel() {
      initialize(16, 48);
    }

    /** Constructor. Allocates triangle and vertex buffers.
     * 
     * @param id identifier of this model.
     * @param triangleCapacity initial triangle capacity of this model.
     * @param vertexCapacity initial vertex capacity of this model. */
    CoreModel(int triangleCapacity, int vertexCapacity) {
      initialize(triangleCapacity, vertexCapacity);
    }

    /** Initializer. Called from constructors. */
    void initialize(int triangleCapacity, int vertexCapacity) {
      ExplicitlyCounted::initialize(this);
      mTriangleData.reserve(triangleCapacity);
      mVertexData.reserve(vertexCapacity);
      mCompiled = false;
      mShadingParamArena.setNextBlockCapacity(1024);
    }

    /** Array of TriAccel structures - one structure instance per triangle. 
     * Created on compilation. Note that this is the only data accessed
     * during triangle intersection tests. */
    arx::FastArray<TriAccel> mTriAccels;

    /** Array of TriangleData structures, describing triangles. 
     * Stays intact after compilation. */
    arx::FastArray<TriangleData> mTriangleData;

    /** Array of VertexData structures, describing vertices.
     * Stays intact after compilation. */
    arx::FastArray<VertexData> mVertexData;

    /** Storage for shading parameters, which are set per triangle and per vertex. */
    MemoryArena<> mShadingParamArena;

    /** Binary Space Subdivision tree for triangle data. 
     * Created on compilation. */
    BspTree mBspTree;

    /** Is this CoreModel compiled? */
    bool mCompiled;
  };


} // namespace smart

#endif // __SMART_COREMODEL_H__
