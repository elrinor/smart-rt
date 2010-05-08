#ifndef __SMART_COREMODELBUILDER_H__
#define __SMART_COREMODELBUILDER_H__

#include "common.h"
#include "CoreModel.h"
#include <arx/smart_ptr.h>

namespace smart {

  class CoreModelBuilder: public arx::noncopyable {
  public:
    CoreModelBuilder(CoreModel* model): mModel(model) {
      assert(!mModel->mCompiled && !mModel->mBeingEdited);
      mModel->mBeingEdited = true;
    }

    ~CoreModelBuilder() {
      if(mModel)
        endEditing();
    }

    void endEditing() {
      assert(mModel && mModel->mBeingEdited);
      mModel->mBeingEdited = false;
      mModel = NULL;
    }

    Shader* newShader() {

    }

    /** Adds a new triangle to triangle buffer. It is guaranteed that 
     * succeeding calls to addTriangle will return succeeding integer 
     * identifiers.
     * 
     * @param vertexId0 identifier of the first vertex.
     * @param vertexId1 identifier of the second vertex.
     * @param vertexId2 identifier of the third vertex.
     * @param shaderId identifier of a shader to use.
     * @returns identifier of a newly created triangle. */
    int addTriangle(int vId0, int vId1, int vId2) {
      assert(mModel);
      return mModel->mTriangles.addTriangle(vId0, vId1, vId2);
    }

    /** Adds a new vertex to vertex buffer. It is guaranteed that succeeding 
     * calls to addVertex will return succeeding integer identifiers.
     *
     * @param v vertex coordinates.
     * @param n normalized vertex normal.
     * @param t texture coordinates.
     * @returns identifier of a newly added vertex. */
    int addVertex(const Vector4f& c, const Vector4f& n, const Vector4f& t) {
      assert(mModel);
      return mModel->mTriangles.addVertex(transformTo3d(c), transformTo3d(n), transformTo3d(t));
    }

  private:
    CoreModel* mModel;
  };

} // namespace smart

#endif // __SMART_COREMODELBUILDER_H__
