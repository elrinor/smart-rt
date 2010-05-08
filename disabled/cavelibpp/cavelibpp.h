#ifndef __CAVELIBPP_H__
#define __CAVELIBPP_H__

#include <arx/smart_ptr.h>
#include <arx/LinearAlgebra.h>

/**
 * @def CAVELIBPP_NO_CAVE
 * Do not compile CAVELib support. This eliminates the runtime dependency on CAVELib shared library.
 */

/**
 * @def CAVELIBPP_NO_GLUT
 * Do not compile GLUT support. This eliminates the runtime dependency on GLUT shared library.
 */

/** CAVELib abstraction library. */
namespace cave {
  namespace detail {
    class RendererImpl;
  }

// -------------------------------------------------------------------------- //
// Camera
// -------------------------------------------------------------------------- //
  /** Structure representing camera in a 3-dimensional space. */
  struct Camera {
    /** Position of the focus point. */
    arx::Vector3f pos; 

    /** Direction of the camera, actually a vector pointing from focus point into the 
     * center of the "virtual screen" */
    arx::Vector3f dir; 

    /** Virtual screen up vector. Half of the virtual screen's height in lenght. */
    arx::Vector3f up;

    /** Virtual screen right vector. Half of the virtual screen's width in lenght. */
    arx::Vector3f right;
  };


// -------------------------------------------------------------------------- //
// Callback
// -------------------------------------------------------------------------- //
  /**
   * CAVELib callback. Subclass it and reimplement the needed functions.
   */
  class Callback {
  private:
    friend class Renderer;

    /** Pointer to parent RendererImpl object. Note that we don't use shared_ptr not to create pointer loop.
     * TODO: weak_ptr? */
    detail::RendererImpl* impl; 

  protected:
    /* Here you can find some functions to use when implementing frame and display handlers.
     * They actually just redirect the calls to RendererImpl object. */
    int getWindowWidth() const;
    int getWindowHeight() const;
    Camera computeCamera() const;
    float getFrameStartTime() const;
    float getTimeFromLastFrame() const;

  public:
    /** OpenGL init function. */
    virtual void init() = 0;

    /**
     * Frame function is called once per frame. Note that inside this function you mustn't:
     * <ul>
     * <li> Mess with modelview and projection matrixes - they were already set up for you.
     * <li> Call glutSwapBuffers() or similar funcitons.
     * </ul>
     */
    virtual void frame() = 0;

    /** Display function is called once per eye, i.e. each call to frame function is followed by one or 
     * more calls to display function. */
    virtual void display() = 0;

    /** Virtual destructor. */
    virtual ~Callback() {
      return;
    }
  };


// -------------------------------------------------------------------------- //
// Renderer
// -------------------------------------------------------------------------- //
  /** Renderer class. Note that only one renderer can be created (TODO: this is not checked). */
  class Renderer {
  private:
    arx::shared_ptr<detail::RendererImpl> impl;
    bool initialized;

  public:
    /** Rendering mode. */
    enum Mode {
#ifndef CAVELIBPP_NO_GLUT
      MODE_GLUT, /**< Render using GLUT library */
#endif
#ifndef CAVELIBPP_NO_CAVE
      MODE_CAVE  /**< Render using CAVELib */
#endif
    };

    /** 
     * Constructor for renderer object.
     *
     * @param mode rendering mode - either glut or cave.
     * @param callback callback object.
     */
    Renderer(Mode mode, arx::shared_ptr<Callback> callback);

    /** Parses specific command line options and initializes.
     * Modifies args and argv. Call before parsing the command line yourself. */
    void init(int* argc, char** argv);

    /** Initializes and starts rendering loop. Never returns. Must be called after init.
     * Terminates application if ESC pressed. */
    void run();
  };
}

#endif // __CAVELIBPP_H__
