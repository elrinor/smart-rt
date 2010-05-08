#ifndef __SMART_TEXTUREMANAGER_H__
#define __SMART_TEXTUREMANAGER_H__

#include "common.h"
#include <arx/Utility.h>
#include "Texture.h"

namespace smart {
  /* TODO: passing TextureManager to ShadedModel on construction makes no sense...
   * think about another way to pass it to TraceContext. */
// -------------------------------------------------------------------------- //
// TextureManager
// -------------------------------------------------------------------------- //
  class TextureManager: public arx::noncopyable {
  public:
    Texture* newTexture(arx::Image3f image) {
      Texture* texture = new Texture(image);
      texture->setId(mTextures.put(texture));
      return texture;
    }

    bool hasTexture(int textureId) const {
      return mTextures.contains(textureId);
    }

    Texture* getTexture(int textureId) const {
      assert(hasTexture(textureId));
      return mTextures[textureId];
    }

    void releaseTexture(Texture* texture) {
      mTextures.remove(texture->getId());
      delete texture;
    }

  private:
    /** Array of Texture objects, indexed by id. */
    IdMap<Texture*> mTextures;
  };

} // namespace smart

#endif // __SMART_TEXTUREMANAGER_H__
