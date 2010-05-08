#ifndef __SMART_BUILDER_H__
#define __SMART_BUILDER_H__

#include "common.h"

namespace smart {

// -------------------------------------------------------------------------- //
// PrimitiveType
// -------------------------------------------------------------------------- //
  /** Triangle construction modes used in Builder class. */
  enum PrimitiveType {
    SMART_TRIANGLES,
    SMART_TRIANGLE_STRIP,
    SMART_TRIANGLE_FAN,
    SMART_QUADS,
    SMART_QUAD_STRIP,
    SMART_POLYGON,
    SMART_DYNAMIC,
    SMART_NO_BUILDER = -1
  };

#define SMART_FOREACH_PRIMITIVE_TYPE(MACRO)                                      \
  MACRO(SMART_TRIANGLES)                                                        \
  MACRO(SMART_TRIANGLE_STRIP)                                                   \
  MACRO(SMART_TRIANGLE_FAN)                                                     \
  MACRO(SMART_QUADS)                                                            \
  MACRO(SMART_QUAD_STRIP)                                                       \
  MACRO(SMART_POLYGON)


// -------------------------------------------------------------------------- //
// BuilderData
// -------------------------------------------------------------------------- //
  namespace detail {
    /** Data class shared between all builders. */
    struct BuilderData {
      Vector4f mNormal;
      Vector4f mTexCoord;
      int mN;
      int mIds[3];
      int mType;
    };
  }


// -------------------------------------------------------------------------- //
// BuilderBase
// -------------------------------------------------------------------------- //
  template<class Derived>
  class BuilderBase {
  public:
    BuilderBase(CoreModel* model, int type) {
      /* Check whether it's legal type first. */
#define SMART_IS_TYPE(TYPE) type == TYPE ||
      assert(SMART_FOREACH_PRIMITIVE_TYPE(SMART_IS_TYPE) false);
#undef SMART_IS_TYPE

      /* Then initialize. */
      mModel = model;
      mData.reset(new detail::BuilderData());
      mData->mN = 0;
      mData->mType = type;
    }

    void vertex(const Vector4f& v) {
      *static_cast<Derived*>(this).vertexAdded(mModel->newVertex(v, mData->mNormal, mData->mTexCoord, NULL));
    }

    void normal(const Vector4f& v) {
      mData->mNormal = v;
    }

    void texCoord(const Vector4f& v) {
      mData->mTexCoord = v;
    }

#define SMART_BUILDERBASE_OVERLOAD(METHOD)                                      \
    void METHOD(float x, float y)  { METHOD(Vector4f(x, y, 0.0f, 1.0f)); }      \
    void METHOD(const Vector2f& v) { METHOD(Vector4f(v[0], v[1], 0.0f, 1.0f)); } \
    void METHOD(float x, float y, float z) { METHOD(Vector4f(x, y, z, 1.0f)); } \
    void METHOD(const Vector3f& v) { METHOD(Vector4f(v[0], v[1], v[2], 1.0f)); } \
    void METHOD(float x, float y, float z, float w) { METHOD(Vector4f(x, y, z, w)); }

    SMART_BUILDERBASE_OVERLOAD(vector)
    SMART_BUILDERBASE_OVERLOAD(normal)
    SMART_BUILDERBASE_OVERLOAD(texCoord)
#undef SMART_BUILDERBASE_OVERLOAD

  protected:
    typedef BuilderBase base_type;

    void addTriangle(int vId0, int vId1, int vId2) {
      mModel->newTriangle(vId0, vId1, vId2, NULL); /* TODO */
    }

    arx::shared_ptr<detail::BuilderData> mData;

  private:
    CoreModel* mModel;
  };


#define SMART_BUILDER_COMMON_CONSTRUCTOR(SOURCE_TYPE)                           \
    SMART_BUILDER_COMMON()                                                      \
  private:                                                                      \
    Builder(CoreModel* model): base_type(model, SOURCE_TYPE) {}

#define SMART_BUILDER_COMMON()                                                  \
    friend class base_type;                                                     \
    friend class Model;                                                         \
    template<int> friend class Builder;


// -------------------------------------------------------------------------- //
// Builder<int>
// -------------------------------------------------------------------------- //
  /** This one is empty, all logic is hidden in specializations. */
  template<int type = SMART_DYNAMIC>
  class Builder {};


// -------------------------------------------------------------------------- //
// Builder<SMART_TRIANGLES>
// -------------------------------------------------------------------------- //
  template<>
  class Builder<SMART_TRIANGLES>: public BuilderBase<Builder<SMART_TRIANGLES> > {
    SMART_BUILDER_COMMON_CONSTRUCTOR(SMART_TRIANGLES)
  private:
    void vertexAdded(int id) {
      if(mData->mN != 2) {
        mData->mIds[mData->mN] = id;
        mData->mN++;
      } else  {
        addTriangle(mData->mIds[0], mData->mIds[1], id);
        mData->mN = 0;
      }
    }
  };


// -------------------------------------------------------------------------- //
// Builder<SMART_TRIANGLE_STRIP>
// -------------------------------------------------------------------------- //
  template<>
  class Builder<SMART_TRIANGLE_STRIP>: public BuilderBase<Builder<SMART_TRIANGLE_STRIP> > {
    SMART_BUILDER_COMMON_CONSTRUCTOR(SMART_TRIANGLE_STRIP)
  private:
    void vertexAdded(int id) {
      if(mData->mN == 2) {
        addTriangle(mData->mIds[0], mData->mIds[1], id);
        mData->mIds[0] = mData->mIds[1];
        mData->mIds[1] = id;
      } else {
        mData->mIds[mData->mN] = id;
        mData->mN++;
      }
    }
  };

  
// -------------------------------------------------------------------------- //
// Builder<SMART_TRIANGLE_FAN>
// -------------------------------------------------------------------------- //
  template<>
  class Builder<SMART_TRIANGLE_FAN>: public BuilderBase<Builder<SMART_TRIANGLE_FAN> > {
    SMART_BUILDER_COMMON_CONSTRUCTOR(SMART_TRIANGLE_FAN)
  private:
    void vertexAdded(int id) {
      if(mData->mN == 2) {
        addTriangle(mData->mIds[0], mData->mIds[1], id);
        mData->mN = 2;
        mData->mIds[1] = id;
      } else {
        mData->mIds[mData->mN] = id;
        mData->mN++;
      }
    }
  };


// -------------------------------------------------------------------------- //
// Builder<SMART_QUADS>
// -------------------------------------------------------------------------- //
  template<>
  class Builder<SMART_QUADS>: public BuilderBase<Builder<SMART_QUADS> > {
    SMART_BUILDER_COMMON_CONSTRUCTOR(SMART_QUADS)
  private:
    void vertexAdded(int id) {
      if(mData->mN != 3) {
        mData->mIds[mData->mN] = id;
        mData->mN++;
      } else  {
        addTriangle(mData->mIds[0], mData->mIds[1], mData->mIds[2]);
        addTriangle(mData->mIds[2], mData->mIds[1], id);
        mData->mN = 0;
      }
    }
  };


// -------------------------------------------------------------------------- //
// Builder<SMART_QUAD_STRIP>
// -------------------------------------------------------------------------- //
  template<>
  class Builder<SMART_QUAD_STRIP>: public BuilderBase<Builder<SMART_QUAD_STRIP> > {
    SMART_BUILDER_COMMON_CONSTRUCTOR(SMART_QUAD_STRIP)
  private:
    void vertexAdded(int id) {
      if(mData->mN == 3) {
        addTriangle(mData->mIds[0], mData->mIds[1], mData->mIds[2]);
        addTriangle(mData->mIds[2], mData->mIds[1], id);
        mData->mIds[0] = mData->mIds[2];
        mData->mIds[1] = id;
        mData->mN = 2;
      } else {
        mData->mIds[mData->mN] = id;
        mData->mN++;
      }
    }
  };


// -------------------------------------------------------------------------- //
// Builder<SMART_POLYGON>
// -------------------------------------------------------------------------- //
  template<>
  class Builder<SMART_POLYGON>: public Builder<SMART_TRIANGLE_FAN> {
    SMART_BUILDER_COMMON()
  private:
    Builder(CoreModel* model):
      Builder<SMART_TRIANGLE_FAN>(model) {}
  };


// -------------------------------------------------------------------------- //
// Builder<SMART_DYNAMIC>
// -------------------------------------------------------------------------- //
  template<>
  class Builder<SMART_DYNAMIC>: public BuilderBase<Builder<SMART_DYNAMIC> > {
    SMART_BUILDER_COMMON()
  private:
    Builder(CoreModel* model, int type):
      base_type(model, type) {}

    void vertexAdded(int id) {
      switch(mData->mType) {
#define SMART_CASE_REDIRECT_BUILDER_CALL(MODE) case MODE: cast<MODE>().vertexAdded(id);
      SMART_FOREACH_PRIMITIVE_TYPE(SMART_CASE_REDIRECT_BUILDER_CALL)
#undef SMART_CASE_REDIRECT_BUILDER_CALL
      default:
        break; /* TODO: error handling, probably impossible. */
      }
    }

    template<int type>
    Builder<type>& cast() {
      return reinterpret_cast<Builder<type>&>(*this);
    }
  };


  /* Clean up. */
#undef SMART_FOREACH_PRIMITIVE_TYPE

} // namespace smart

#endif // __SMART_BUILDER_H__
