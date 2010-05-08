#ifndef __SMART_RENDERTILER_H__
#define __SMART_RENDERTILER_H__

#include "common.h"
#include "ImageTile.h"

namespace smart {
// -------------------------------------------------------------------------- //
// RenderTiler
// -------------------------------------------------------------------------- //
  class RenderTiler {
  public:
    void nextTask(ShadedScene* scene, arx::Image3f& image, int renderersCount);

    bool nextTile(int rendererId, ImageTile& tile);
  };


// -------------------------------------------------------------------------- //
// LinearTiler
// -------------------------------------------------------------------------- //
  class LinearTiler {
  public:
    LinearTiler(int tileSize): mTileSize(tileSize) {}

    void nextTask(ShadedScene* scene, arx::Image3f& image, int renderersCount) {
      mImageWidth = image.getWidth();
      mImageHeight = image.getHeight();
      mCols = (mImageHeight + mTileSize - 1) / mTileSize;
      mRows = (mImageWidth + mTileSize - 1) / mTileSize;
      mCol = 0;
      mRow = 0;
    }

    bool nextTile(int rendererId, ImageTile& tile) {
      if(mRow >= mRows) {
        return false;
      } else {
        int x = mCol * mTileSize;
        int y = mRow * mTileSize;
        int w = std::min(x + mTileSize, mImageWidth) - x;
        int h = std::min(y + mTileSize, mImageHeight) - y;
        tile = ImageTile(x, y, w, h);

        mCol++;
        if(mCol > mCols) {
          mCol = 0;
          mRow++;
        }

        return true;
      }
    }

  private:
    int mTileSize;
    int mRows;
    int mCols;
    int mImageWidth;
    int mImageHeight;
    int mRow;
    int mCol;
  };


// -------------------------------------------------------------------------- //
// AbstractRenderTiler
// -------------------------------------------------------------------------- //
  class AbstractRenderTiler: public arx::noncopyable {
  public:
    virtual void nextTask(ShadedScene* scene, arx::Image3f& image, int renderersCount) = 0;
    virtual bool nextTile(int rendererId, ImageTile& tile) = 0;
  };


// -------------------------------------------------------------------------- //
// RenderTilerAdapter
// -------------------------------------------------------------------------- //
  template<class Tiler>
  class RenderTilerAdapter: public AbstractRenderTiler {
  public:
    RenderTilerAdapter(Tiler tiler): mTiler(tiler) {}
    
    virtual void nextTask(ShadedScene* scene, arx::Image3f& image, int renderersCount) {
      mTiler.nextTask(scene, image, renderersCount);
    }

    virtual bool nextTile(int rendererId, ImageTile& tile) {
      return mTiler.nextTile(rendererId, tile);
    }

  private:
    Tiler mTiler;
  };

} // namespace smart

#endif // __SMART_RENDERTILER_H__
