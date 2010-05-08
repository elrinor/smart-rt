#ifndef __SMART_SMARTACCESSOR_H__
#define __SMART_SMARTACCESSOR_H__

#include "common.h"

namespace smart {
  namespace detail {
    /* Forward it to befriend it. */
    class EmptySmartAccessor;
  }

  class CoreModel;
  class CoreScene;
  class Model;
  class Scene;

  /** This is a helper class that aids in eliminating cyclic dependencies. */
  class SmartAccessor {
  public:
    const CoreScene& getCoreScene(int id) const;
    CoreScene& getCoreScene(int id);
    void freeModel(Model& model);

    const CoreModel& getCoreModel(int id) const;
    CoreModel& getCoreModel(int id);
    void freeScene(Scene& scene);
    
    void invalidate() {
      mSmart = NULL;
    }

    bool isValid() {
      return mSmart != NULL;
    }

  private:
    friend class Smart;
    friend class detail::EmptySmartAccessor;

    explicit SmartAccessor(Smart* smart): mSmart(smart) {}

    Smart* mSmart;
  };

  namespace detail {
    /** This class is used for empty SmartAccessor generation in error handling. */
    class EmptySmartAccessor {
    public:
      SmartAccessor operator()() {
        return SmartAccessor(NULL);
      }
    };
  }

  // -------------------------------------------------------------------------- //
  // SmartAccessor methods
  // -------------------------------------------------------------------------- //
  /* These methods are here to resolve cyclic dependencies. */
  inline const CoreScene& SmartAccessor::getCoreScene(int id) const {
    return mSmart->mScenes[id];
  }

  inline CoreScene& SmartAccessor::getCoreScene(int id) {
    return mSmart->mScenes[id];
  }

  inline const CoreModel& SmartAccessor::getCoreModel(int id) const {
    return mSmart->mModels[id];
  }

  inline CoreModel& SmartAccessor::getCoreModel(int id) {
    return mSmart->mModels[id];
  }

  inline void SmartAccessor::freeModel(Model& model) {
    mSmart->freeModel(model);
  }

  inline void SmartAccessor::freeScene(Scene& scene) {
    mSmart->freeScene(scene);
  }

} // namespace smart

#endif // __SMART_SMARTACCESSOR_H__
