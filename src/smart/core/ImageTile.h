#ifndef __SMART_IMAGETILE_H__
#define __SMART_IMAGETILE_H__

#include "common.h"

namespace smart {
// -------------------------------------------------------------------------- //
// ImageTile
// -------------------------------------------------------------------------- //
  class ImageTile {
  public:
    ImageTile() {}

    ImageTile(int x, int y, int w, int h): mX(x), mY(y), mWidth(w), mHeight(h) {}

    int getX() const {
      return mX;
    }

    int getY() const {
      return mY;
    }

    int getHeight() const {
      return mHeight;
    }

    int getWidth() const {
      return mWidth;
    }

  private:
    int mX, mY, mWidth, mHeight;
  };

} // namespace smart

#endif // __SMART_IMAGETILE_H__
