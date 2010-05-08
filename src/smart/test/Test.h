#ifndef __SMART_TEST_H__
#define __SMART_TEST_H__

#include "../Smart.h"
#include <cassert>

namespace smart {

  void test_BspNode_getSplitDimension() {
    for(int i = 0; i <= 2; i++) {
      BspNode n = BspNode(BspNode::INNER(), i, 0.0f, NULL);
      assert(n.getDim<0>() == i);
      assert(n.getDim<1>() == (i + 1) % 3);
      assert(n.getDim<2>() == (i + 2) % 3);
    }
  }

  void test_Engine_intersects_BoundingBox_Triangle() {
    typedef Vector3f V;

    assert(Engine::intersects(BoundingBox(V(0,0,0), V(1,1,1)), Triangle(V(2,0,0), V(0,2,0), V(0,0,2))));
    assert(Engine::intersects(BoundingBox(V(0,0,0), V(1,1,1)), Triangle(V(3,0,0), V(0,3,0), V(0,0,3))));

    assert(Engine::intersects(BoundingBox(V(0,0,0), V(1,1,1)), Triangle(V(3,0,0), V(0,3,0), V(0,0,2.9f))));
    assert(Engine::intersects(BoundingBox(V(0,0,0), V(1,1,1)), Triangle(V(3,0,0), V(0,2.9f,0), V(0,0,3))));
    assert(Engine::intersects(BoundingBox(V(0,0,0), V(1,1,1)), Triangle(V(2.9f,0,0), V(0,3,0), V(0,0,3))));
    assert(!Engine::intersects(BoundingBox(V(0,0,0), V(1,1,1)), Triangle(V(3,0,0), V(0,3,0), V(0,0,3.1f))));
    assert(!Engine::intersects(BoundingBox(V(0,0,0), V(1,1,1)), Triangle(V(3,0,0), V(0,3.1f,0), V(0,0,3))));
    assert(!Engine::intersects(BoundingBox(V(0,0,0), V(1,1,1)), Triangle(V(3.1f,0,0), V(0,3,0), V(0,0,3))));

    assert(Engine::intersects(BoundingBox(V(-1,-1,-1), V(0,0,0)), Triangle(V(-3,0,0), V(0,-3,0), V(0,0,-2.9f))));
    assert(Engine::intersects(BoundingBox(V(-1,-1,-1), V(0,0,0)), Triangle(V(-3,0,0), V(0,-2.9f,0), V(0,0,-3))));
    assert(Engine::intersects(BoundingBox(V(-1,-1,-1), V(0,0,0)), Triangle(V(-2.9f,0,0), V(0,-3,0), V(0,0,-3))));
    assert(!Engine::intersects(BoundingBox(V(-1,-1,-1), V(0,0,0)), Triangle(V(-3,0,0), V(0,-3,0), V(0,0,-3.1f))));
    assert(!Engine::intersects(BoundingBox(V(-1,-1,-1), V(0,0,0)), Triangle(V(-3,0,0), V(0,-3.1f,0), V(0,0,-3))));
    assert(!Engine::intersects(BoundingBox(V(-1,-1,-1), V(0,0,0)), Triangle(V(-3.1f,0,0), V(0,-3,0), V(0,0,-3))));

    assert(Engine::intersects(BoundingBox(V(-1,-1,-1), V(1,1,1)), Triangle(V(-100,-100,0), V(100,0,0), V(0,100,0))));
    assert(Engine::intersects(BoundingBox(V(-1,-1,-1), V(1,1,1)), Triangle(V(-100,-100,1), V(100,0,1), V(0,100,1))));
    assert(Engine::intersects(BoundingBox(V(-1,-1,-1), V(1,1,1)), Triangle(V(-100,-100,-1), V(100,0,-1), V(0,100,-1))));
    assert(!Engine::intersects(BoundingBox(V(-1,-1,-1), V(1,1,1)), Triangle(V(-100,-100,2), V(100,0,2), V(0,100,2))));

    assert(Engine::intersects(BoundingBox(V(-1,-1,-1), V(1,1,1)), Triangle(V(1,1,1), V(1,1,2), V(1,2,1))));
  }

  void testSmart() {
    test_BspNode_getSplitDimension();
    test_Engine_intersects_BoundingBox_Triangle();
  }

} // namespace smart

#endif // __SMART_TEST_H__
