libcvd-cl
=========

libcvd-cl implements several computer vision algorithms in a simple
and extensible framework of standard OpenCL and C++.

It is related to
<a href="http://savannah.nongnu.org/projects/libcvd">libcvd</a>
by the <i>concepts</i> of its algorithms, although the algorithms in libcvd-cl are
completely reinvented for highly parallel architectures.



Included algorithms
-------------------

*   <h3>Simple image blur</h3>

    *   Blur grayscale images (<code>BlurGrayStep</code>)
    *   Blur colour images (<code>BlurRichStep</code>)

*   Remarks:

    *   OpenCL code is pre-generated with a given convolution kernel.



*   <h3>Point cloud pre-processing</h3>

    *   Convert <i>(x,y)</i> to <i>(u,v)</i> or <i>(u,v,q</i>) for a known camera (<code>ToUvqUvStep</code>)
    *   Filter positions by depth (<code>ClipDepthStep</code>)
    *   Perform arbitrary pixel->value mappings (<code>FxyStep</code>)



*   <h3>FAST</h3>

    *   Find corners in grayscale images (<code>PreFastGrayStep</code> then <code>FastGrayStep</code>)
    *   Find corners in colour images (<code>PreFastRichStep</code> then <code>FastRichStep</code>)

*   Remarks:

    *   Tests 16 pixels in a ring, with variable corner ring size and threshold.
    *   OpenCL on CPU is slow, but OpenCL on a reasonable GPU is faster than C++ on CPU.
    *   Code is almost entirely branch-free, intended for GPU and not CPU.



*   <h3>HIPS</h3>

    *   Build descriptors for grayscale images (<code>HipsGrayStep</code> or <code>HipsBlendGrayStep</code>)
    *   Build descriptors for colour images (<code>HipsRichStep</code> or <code>HipsBlendRichStep</code>)
    *   Remove descriptors with high bit count (<code>HipsClipStep</code>)
    *   Match descriptors by brute force (<code>HipsFindStep</code>)
    *   Build balanced tree/forest in C++ (<code>HipsMakeTreeStep</code>)
    *   Search balanced tree/forest in C++ (lossy, lossless) and OpenCL (lossy only) (<code>HipsTreeFindStep</code>)

*   Remarks:

    *   Tests 64 pixels in concentric circles.
    *   Search kernels have optional naive rotational invariance using barrel shift.
    *   OpenCL on CPU is slow, but OpenCL on a reasonable GPU is faster than C++ on CPU.



*   <h3>3-point pose using RANSAC</h3>

    *   Assign matrix identity (<code>MatIdentStep</code>)
    *   Randomly select point triples (<code>RandomIntStep</code> then <code>MixUvqUvStep</code>)
    *   Generate Jacobian matrix (<code>PoseUvqWlsStep</code>)
    *   Perform Cholesky decomposition and back-substitution (<code>CholeskyStep</code>)
    *   Exponentiate SE3 matrix (<code>SE3ExpStep</code>)
    *   Use SE3 matrix for point transforms (<code>SE3Run1Step</code>)
    *   Evaluate SE3 matrix for inliers (<code>SE3ScoreStep</code>)

*   Remarks:

    *   Basic iterative computation, but does <i>not</i> refine with all inliers.
    *   OpenCL on CPU is quite fast, OpenCL on GPU is extremely fast.



*   <h3>2D point cloud "radar" matching</h3>

    *   Compute point radar (<code>makePointRadar()</code>)
    *   Match two point radars (<code>matchPointRadars()</code>)

* Remarks:

    *   Highly experimental, not published in the literature.
    *   Matches by spatial distribution of points in 2D.
    *   Conceptually similar to "semi-local constraints".
    *   Naturally invariant to 2D/3D scale, 2D translation and 2D rotation.
    *   Intolerant of large shear, 3D translation, and 3D rotation.
    *   Potentially useful for inter-frame tracking.



*   <h3>3D point cloud "galaxy" matching</h3>

    *   Compute point galaxy (<code>makePointGalaxy()</code>)
    *   Match two point galaxies (<code>matchPointGalaxies()</code>)

* Remarks:

    *   Very highly experimental, yet to show promise.
    *   Matches by spatial distribution of points in 3D.
    *   Conceptually similar to "semi-local constraints".
    *   Naturally invariant to 3D translation, 3D rotation and 3D scale by a single scalar.
    *   Requires consistent (x,y,z) scale.
    *   Potentially useful for inter-frame tracking.



Components
----------

*   <h3>Python scripts to generate OpenCL source</h3>

    In general, where there are <i>fixed-size</i> loops in the OpenCL code,
    the unrolled code is pre-generated instead.  This way, device registers can be used
    instead of (even implicit) global memory arrays and numeric expressions can be
    simplified and propagated, making code faster for only slightly higher developer effort.

    Not all kernels require code generation, but for consistency, even those that
    don't still have a Python script which is simply a large <code>print</code>.



*   <h3>C++ classes representing the OpenCL situation</h3>

    *   <code>Worker</code> class bundles an OpenCL device, its context, and its command queue.



*   <h3>C++ classes defining data <i>states</i></h3>

    *   Camera information per pixel (<code>CameraState</code>)
    *   Integer counter (<code>CountState</code>)
    *   HIPS balanced tree state (<code>HipsTreeState</code>)
    *   Image data of any format (<code>ImageState</code>)
    *   Variable-size list data of any type (<code>ListState</code>)
    *   Matrix/vector data of small fixed size (<code>MatrixState</code>)
    *   <i>(u,v)</i> point cloud (<code>UvState</code>)
    *   <i>(u,v,q)</i> point cloud (<code>UvqState</code>)
    *   <i>((u,v,q),(u,v))</i> point pair cloud (<code>UvqUvState</code>)

*   Remarks:

    *   States may be created once and linked by multiple steps.
    *   Most states have methods to translate data to/from reasonable C++ types.
    *   Each state is bound to exactly one worker.



*   <h3>C++ classes defining processing <i>steps</i></h3>

    *   See "Included algorithms" above.

*   Remarks:

    *   Steps may link multiple input and output states.
    *   Steps may be created once and run repeatedly as part of a pipeline.
    *   Steps may be timed individually.
    *   Each step is bound to exactly one worker, inferred from the worker of its states.



*   <h3>Test driver programs</h3>

    *   Messy; <i>not</i> intended for code reuse.
    *   May serve as working examples for library consumers.


Resources for OpenCL developers
------------------------------

*   <h3>Standards body</h3>

    *   [Khronos](http://www.khronos.org/opencl/)

*   <h3>Implementation vendors</h3>

    *   [AMD](http://developer.amd.com/sdks/AMDAPPSDK/documentation/Pages/default.aspx)
    *   [Apple](http://developer.apple.com/library/mac/documentation/Performance/Conceptual/OpenCL_MacProgGuide/)
    *   [IBM](http://www.alphaworks.ibm.com/tech/opencl/)
    *   [Intel](http://software.intel.com/en-us/articles/vcsource-tools-opencl-sdk/)
    *   [NVIDIA](http://developer.nvidia.com/opencl/)
