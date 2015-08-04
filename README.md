# About
SMART is a real-time ray tracing engine. Its name stands for SMART Minimalistic Arx Ray Tracer.

SMART was designed from the ground with performance and extensibility in mind. The main design rule followed during the development of SMART was never to sacrifice the flexibility of the engine unless it is sacrificed for performance.

The main features of the SMART engine are:

* Triangle geometry. SMART works with triangles only. This restriction increases the performance of ray tracer, while not imposing a major penalty on the flexibility &mdash; anyway, almost all of the real-world 3d models use polygonal representation.
* Axis-aligned BSP tree as acceleration structure for geometry. BSP trees have shown to perform at least not worse than any other acceleration structures, while allowing for a very simple, and therefore really fast traversal implementation. Another benefit of BSP trees is that the implementation of ray packet traversal for them is rather straightforward, which is not generally true for other acceleration structures.
* `O(N logN)` BSP tree construction, as described in `[1]`. Fast BSP tree building algorithm based on Surface Area Heuristic is used, which produces high quality BSP trees while offering a complexity of <i>O(N logN)</i>, which is a theoretical lower bound for any BSP tree building algorithm. </li>
* Two-level acceleration structure. SMART operates on immutable geometry units &mdash; models. Each model consists of several triangles and cannot be changed once compiled. That means that for each model a BSP tree must be built only once &mdash; at the time of model construction and can be reused during rendering of several succeeding frames. To allow for dynamic scenes, compiled models are inserted into a top-level global BSP tree, which is rebuilt every frame. Models that are generated anew each frame could also be used with this approach, but they will require another BSP tree building algorithm that would work faster but possibly produce BSP trees of inferior quality.
* Multithreaded rendering. SMART automatically determines the number of available cores and performs rendering in the corresponding number of threads. Load balancing is done automatically, so that when double buffering is used, rendering threads do not stall between frames, therefore utilizing all available computational power. 
* Carefully optimized triangle intersection and BSP tree traversal routines. Ray tracer spends most of the rendering time in these routines, therefore they must be extremely optimized. SMART uses a method described by Ingo Wald in his Ph.D. thesis `[2]` with some slight modifications to pay attention to floating point issues.


More information on SMART is available in this [report](https://wings-of-wrath.googlecode.com/hg/articles/smart_report/smart.pdf). It is a bit messy and outdated, though.

`[1]` I. Wald and V. Havran. On building fast kd-trees for ray tracing, and on doing that in <i>O(N logN)</i>. Proceedings of the 2006 IEEE Symposium on Interactive Ray Tracing, 2006. [PDF](http://www.sci.utah.edu/~wald/Publications/2006///NlogN/download//kdtree.pdf)

`[2]` I. Wald. Realtime Ray Tracing and Interactive Global Illumination. PhD thesis, 2004. [PDF](http://www.sci.utah.edu/~wald/Publications/2004///WaldPhD/download//phd.pdf)
