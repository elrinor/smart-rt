#ifndef __SMART_TRACER_H__
#define __SMART_TRACER_H__

#include "common.h"
#include "TraceContext.h"
#include "IntersectionTests.h"

namespace smart {
// -------------------------------------------------------------------------- //
// Tracer
// -------------------------------------------------------------------------- //
  class Tracer {
  public:
    /** Traces the ray through the given triangle (given as TriAccel structure). 
     *
     * In case there was no intersection, returns false without modifying any 
     * of the parameters. 
     *
     * If there was an intersection, then returns true, clipping the segment and
     * storing barycentric coordinates. Note that triangle id is unknown at this
     * step, therefore it should be set by a callee. */
    static bool trace(TraceContext& ctx, const TriAccel& triAccel) {
      if(intersect(ctx.ray, ctx.segment, triAccel, ctx.barycentricCoord, SMART_TRACETRIACCEL_EPS)) {
        ctx.tAlongRay = ctx.segment.getMax(); // TODO remove.
        return true;
      } else
        return false;
    }

    struct StackElement {
      StackElement(const BspNode* node, float segmentEnd): 
        node(node), segmentEnd(segmentEnd) {}

      const BspNode* node;
      float segmentEnd;
    };

    template<class StaticStack>
    static FORCEINLINE void trace(TraceContext& ctx, StaticStack& nodeStack, const BspNode*& nextNode, const BspNode* frontChild, const BspNode* backChild, float d) {
      const float eps = SMART_TRACEBSPNODE_SEGMENTCONTAINS_EPS;
      if(d < ctx.segment.getMin() - eps - eps * d) {
        /* Case one, cull front side. */
        nextNode = backChild;
      } else if(ctx.segment.getMax() + eps + eps * d < d) {
        /* Case two, cull back side. */
        nextNode = frontChild;
      } else {
        /* Case three - traverse both sides in turn. 
         * Push the back one into the stack. */
        nodeStack.push_back(StackElement(backChild, ctx.segment.getMax()));

        /* Then issue traversal of the front one. */
        nextNode = frontChild;
        ctx.segment.setMax(d);
      }
    }


    /** Recursively traces the ray through the given BSP subtree. 
     * Clips in case of a valid intersection. */
    static bool trace(TraceContext& ctx, const ShadedModel* model, const BspNode* root) {
      arx::StaticFastArray<StackElement, SMART_MAX_BSPTREE_DEPTH> nodeStack;
     
      const BspNode* node = root;

      while(true) {
        /* First deal with leaf nodes. */
        if(node->isLeaf()) {
          NodeTriangleIdList list = node->getTriangleIndexList();
          bool success = false;
          for(int i = 0; i < list.size(); ++i) {
            if(trace(ctx, model->getTriAccel(list[i]))) {
              ctx.triangleId = list[i];
              success = true;
            }
          }
          if(success)
            return true;
          else if(nodeStack.size() == 0)
            return false;
          else {
            node = nodeStack.back().node;
            ctx.segment.setMin(ctx.segment.getMax());
            ctx.segment.setMax(nodeStack.back().segmentEnd);
            nodeStack.pop_back();
          }
        } else {
          /* TODO: this can be moved up, once for one call. */
          if(abs(ctx.ray.getDirection(node->getSplitDim())) > SMART_TRACEBSPNODE_DIRECTIONGEZERO_EPS) {
            /* Calculate distance along the ray to the splitting dimension. */
            const float d = (node->getSplitCoord() - ctx.ray.getOrigin(node->getSplitDim())) / 
              ctx.ray.getDirection(node->getSplitDim());

            /* Trace children in order. */
            if(ctx.ray.getDirection(node->getSplitDim()) > 0) {
              trace(ctx, nodeStack, node, node->getLeftChild(), node->getRightChild(), d);
            } else {
              trace(ctx, nodeStack, node, node->getRightChild(), node->getLeftChild(), d);
            }
          } else {
            /* Intersection impossible. */
            const float d = (ctx.ray.getOrigin(node->getSplitDim()) < node->getSplitCoord()) ?
              std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();
            trace(ctx, nodeStack, node, node->getLeftChild(), node->getRightChild(), d);
          }
        }
      }
    }

    /** Traces the the ray through the given ShadedModel. 
     * Clips in case of a valid intersection. */
    static bool trace(TraceContext& ctx, const ShadedModel* model) {
      Segment oldSegment = ctx.segment;
      clip(ctx.segment, ctx.ray, model->getBoundingBox());

      if(!ctx.segment.isEmpty<true, true>()) {
        if(trace(ctx, model, model->getBspTree().getRoot())) {
          ctx.model = model;
          ctx.segment.setMin(oldSegment.getMin());
          return true;
        }
      }

      ctx.segment = oldSegment;
      return false;
    }

    /** Traces the ray through the given CoreObject. 
     * Clips in case of a valid intersection. */
    static bool trace(TraceContext& ctx, const CoreObject* object) {
      Ray oldRay = ctx.ray;

      ctx.ray.setOrigin(transform(oldRay.getOrigin(), object->getWorldToLocalTransform()));
      ctx.ray.setDirection((transform(Vector3f(oldRay.getOrigin() + oldRay.getDirection()), object->getWorldToLocalTransform()) - ctx.ray.getOrigin()).normalized()); /* TODO: separate matrix? */
      bool traceResult = trace(ctx, object->getModel());
      ctx.ray = oldRay;

      if(traceResult)
        ctx.object = object;

      return traceResult;
    }

    static bool shadow(TraceContext& ctx) {
      ctx.segment.setMin(SMART_TRACEUPPER_SEGMENTSTART_EPS);

      for(int i = 0; i < ctx.scene->getObjectCount(); ++i)
        if(trace(ctx, ctx.scene->getObject(i)))
          return true;

      return false;
    }

    /** Top-level tracing routine. Traces the given ray through the given scene. */
    static void trace(TraceContext& ctx) {
      ctx.segment = Segment(SMART_TRACEUPPER_SEGMENTSTART_EPS, std::numeric_limits<float>::max());

      assert(abs(ctx.ray.getDirection().squaredNorm() - 1.0f) < 1.0e-5);

      //clip(ctx.segment, ctx.ray, ctx.scene->getBoundingBox());

      /*if(ctx.segment.isEmpty<true, true>()) {
        ctx.scene->getEnvShader()->envShade(ctx);
        return;
      }*/

      /* TODO - Upper Level BSP */
      bool intersectionFound = false;
      if(ctx.depth < 16) {
        for(int i = 0; i < ctx.scene->getObjectCount(); ++i)
          if(trace(ctx, ctx.scene->getObject(i)))
            intersectionFound = true;
      }

      if(!intersectionFound) {
        ctx.scene->getEnvShader()->envShade(ctx);
        return;
      }

      ctx.model->getTriangleShader(ctx.triangleId)->surfShade(ctx);
    }
   
  };

  inline Radiance TraceContext::trace(const Vector3f& position, const Vector3f& direction, float k) const {
    /* TODO check k & depth. */
    TraceContext ctx;
    ctx.setScene(scene);
    ctx.setDepth(depth + 1);

    ///*
    // ctx.ray.setOrigin(transform(position, object->getLocalToWorldTransform()));
    // ctx.ray.setDirection((transform(Vector3f(position + direction), object->getLocalToWorldTransform()) - ctx.ray.getOrigin()).normalized());
    //*/

    ctx.ray = Ray(position, direction);

    Tracer::trace(ctx);
    return ctx.getRadiance() * k;
  }

  inline Radiance TraceContext::illuminate(int lightIndex, const Vector3f& position, Vector3f& direction, float& distance) const {
    Radiance result;
    scene->getLightShaderByIndex(lightIndex)->illuminate(position, direction, distance, result);

    // /* TODO reimplement trace. */
    return result;
  }

  inline bool TraceContext::shadow(const Vector3f& position, const Vector3f& direction, float distance) const {
    TraceContext ctx;
    ctx.setScene(scene);
    ctx.setDepth(0);
    ctx.ray = Ray(position, direction);
    ctx.segment = Segment(0, distance);

    return Tracer::shadow(ctx);
  }



} // namespace smart

#endif // __SMART_TRACER_H__
