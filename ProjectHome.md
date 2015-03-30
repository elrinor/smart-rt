<p> SMART is a real-time ray tracing engine. Its name stands for SMART Minimalistic Arx Ray Tracer. </p>

<p> SMART was designed from the ground with performance and extensibility in mind. The main design rule followed during the development of SMART was never to sacrifice the flexibility of the engine unless it is sacrificed for performance. </p>

<p> The main features of the SMART engine are: </p>

<ul>
<li> Triangle geometry. SMART works with triangles only. This restriction increases the performance of ray tracer, while not imposing a major penalty on the flexibility &mdash; anyway, almost all of the real-world 3d models use polygonal representation. </li>
<li> Axis-aligned BSP tree as acceleration structure for geometry. BSP trees have shown to perform at least not worse than any other acceleration structures, while allowing for a very simple, and therefore really fast traversal implementation. Another benefit of BSP trees is that the implementation of ray packet traversal for them is rather straightforward, which is not generally true for other acceleration structures. </li>
<li> <i>O(N logN)</i> BSP tree construction, as described in [<a href='#ref1'>1</a>]. Fast BSP tree building algorithm based on Surface Area Heuristic is used, which produces high quality BSP trees while offering a complexity of <i>O(N logN)</i>, which is a theoretical lower bound for any BSP tree building algorithm. </li>
<li> Two-level acceleration structure. SMART operates on immutable geometry units &mdash; models. Each model consists of several triangles and cannot be changed once compiled. That means that for each model a BSP tree must be built only once &mdash; at the time of model construction and can be reused during rendering of several succeeding frames. To allow for dynamic scenes, compiled models are inserted into a top-level global BSP tree, which is rebuilt every frame. Models that are generated anew each frame could also be used with this approach, but they will require another BSP tree building algorithm that would work faster but possibly produce BSP trees of inferior quality.</li>
<li> Multithreaded rendering. SMART automatically determines the number of available cores and performs rendering in the corresponding number of threads. Load balancing is done automatically, so that when double buffering is used, rendering threads do not stall between frames, therefore utilizing all available computational power. </li>
<li> Carefully optimized triangle intersection and BSP tree traversal routines. Ray tracer spends most of the rendering time in these routines, therefore they must be extremely optimized. SMART uses a method described by Ingo Wald in his Ph.D. thesis [<a href='#ref2'>2</a>] with some slight modifications to pay attention to floating point issues.</li>
</ul>

<p> More information on SMART is available in this <a href='https://wings-of-wrath.googlecode.com/hg/articles/smart_report/smart.pdf' title='SMART Details'>report</a>. It is a bit messy and outdated, though. </p>

<p>
[<a>1</a>] I. Wald and V. Havran. On building fast kd-trees for ray tracing, and on doing that in <i>O(N logN)</i>. Proceedings of the 2006 IEEE Symposium on Interactive Ray Tracing, 2006. <a href='http://www.sci.utah.edu/~wald/Publications/2006///NlogN/download//kdtree.pdf'><a href='PDF.md'>PDF</a></a> <br />
[<a>2</a>] I. Wald. Realtime Ray Tracing and Interactive Global Illumination. PhD thesis, 2004. <a href='http://www.sci.utah.edu/~wald/Publications/2004///WaldPhD/download//phd.pdf'><a href='PDF.md'>PDF</a></a> <br />
</p>