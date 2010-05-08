#ifndef __SMART_TEXTURE_H__
#define __SMART_TEXTURE_H__

#include "common.h"
#include <arx/Utility.h>
#include <arx/Image.h>
#include "Color.h"
#include "Idded.h"

namespace smart {

  /** Texture class represents an OpenGL-like texture. It's pixel values are 
   * indexed with float coordinates, <tt>(0, 0)</tt> being the lower-left 
   * corner of an image, and <tt>(1, 1)</tt> - the upper-right. It behaves
   * similarly to OpenGL textures in a sense that in case coordinates don't lie
   * in a <tt>[0, 1]</tt> segment, it just repeats the texture.
   *
   * Clamping and similar OpenGL modes are currently not supported. */
  class Texture: private arx::noncopyable, public Idded {
  public:
    Color getColor(float x, float y) const {
      float fx = x - floor(x);
      float fy = y - floor(y); /* TODO: this is wrong, you should provide borders. */
      arx::Color3f color = mData.getPixelInterpolated(fx * (mData.getWidth() - 1), fy * (mData.getHeight() - 1));
      return Color(color.r, color.g, color.b);
    }

  private:
    template<class ColorType, class Derived, bool materialized>
    Texture(const arx::GenericImageBase<ColorType, Derived, materialized>& image) {
      mData = arx::image_cast<arx::Image3f>(image);
    }

    friend class TextureManager;

    arx::Image3f mData;
  };

} // namespace smart

#endif // __SMART_TEXTURE_H__
