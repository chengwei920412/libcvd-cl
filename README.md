libcvd-cl
=========

libcvd-cl implements several computer vision algorithms in a simple
and extensible framework of standard OpenCL and C++.

It is related to
<a href="http://savannah.nongnu.org/projects/libcvd">libcvd</a>
by the <i>concepts</i> of its algorithms, although the algorithms in libcvd-cl are
completely reinvented for highly parallel architectures.

***

Included algorithms
-------------------

*   <h3>Simple image blur</h3>

    *   Blur grayscale images (<b>BlurGrayStep</b>)
    *   Blur colour images (<b>BlurRichStep</b>)

    Remarks:

    *   OpenCL code is pre-generated with a given convolution kernel.



*   <h3>Point cloud pre-processing</h3>

    *   Convert <i>(x,y)</i> to <i>(u,v)</i> or <i>(u,v,q</i>) for a known camera (<b>ToUvqUvStep</b>)
    *   Filter positions by depth (<b>ClipDepthStep</b>)
    *   Perform arbitrary pixel->value mappings (<b>FxyStep</b>)



*   <h3>FAST</h3>

    *   Find corners in grayscale images (<b>PreFastGrayStep</b> then <b>FastGrayStep</b>)
    *   Find corners in colour images (<b>PreFastRichStep</b> then <b>FastRichStep</b>)

    Remarks:

    *   Tests 16 pixels in a ring, with variable corner ring size and threshold.
    *   OpenCL on CPU is slow, but OpenCL on a reasonable GPU is faster than C++ on CPU.
    *   Code is almost entirely branch-free, intended for GPU and not CPU.



*   <h3>HIPS</h3>

    *   Build descriptors for grayscale images (<b>HipsGrayStep</b> or <b>HipsBlendGrayStep</b>)
    *   Build descriptors for colour images (<b>HipsRichStep</b> or <b>HipsBlendRichStep</b>)
    *   Remove descriptors with high bit count (<b>HipsClipStep</b>)
    *   Match descriptors by brute force (<b>HipsFindStep</b>)
    *   Build balanced tree/forest in C++ (<b>HipsMakeTreeStep</b>)
    *   Search balanced tree/forest in C++ (lossy, lossless) and OpenCL (lossy only) (<b>HipsTreeFindStep</b>)

    Remarks:

    *   Tests 64 pixels in concentric circles.
    *   Search kernels have optional naive rotational invariance using barrel shift.
    *   OpenCL on CPU is slow, but OpenCL on a reasonable GPU is faster than C++ on CPU.



*   <h3>3-point pose using RANSAC</h3>

    *   Assign matrix identity (<b>MatIdentStep</b>)
    *   Randomly select point triples (<b>RandomIntStep</b> then <b>MixUvqUvStep</b>)
    *   Generate Jacobian matrix (<b>PoseUvqWlsStep</b>)
    *   Perform Cholesky decomposition and back-substitution (<b>CholeskyStep</b>)
    *   Exponentiate SE3 matrix (<b>SE3ExpStep</b>)
    *   Use SE3 matrix for point transforms (<b>SE3Run1Step</b>)
    *   Evaluate SE3 matrix for inliers (<b>SE3ScoreStep</b>)

    Remarks:

    *   Basic iterative computation, but does <i>not</i> refine with all inliers.
    *   OpenCL on CPU is quite fast, OpenCL on GPU is extremely fast.



* <h3>2D point cloud "radar" matching</h3>

  * Compute point radar (<b>makePointRadar()</b>)
  * Match two point radars (<b>matchPointRadars()</b>)

  Remarks:

  * Highly experimental, not published in the literature.
  * Matches by spatial distribution of points in 2D.
  * Conceptually similar to "semi-local constraints".
  * Naturally invariant to 2D/3D scale, 2D translation and 2D rotation.
  * Intolerant of large shear, 3D translation, and 3D rotation.
  * Potentially useful for inter-frame tracking.



*   <h3>3D point cloud "galaxy" matching</h3>

    *   Compute point galaxy (<b>makePointGalaxy()</b>)
    *   Match two point galaxies (<b>matchPointGalaxies()</b>)

    Remarks:

    *   Very highly experimental, yet to show promise.
    *   Matches by spatial distribution of points in 3D.
    *   Conceptually similar to "semi-local constraints".
    *   Naturally invariant to 3D translation, 3D rotation and 3D scale by a single scalar.
    *   Requires consistent (x,y,z) scale.
    *   Potentially useful for inter-frame tracking.



***


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

    *   <b>Worker</b> class bundles an OpenCL device, its context, and its command queue.



*   <h3>C++ classes defining data <i>states</i></h3>

    *   Camera information per pixel (<b>CameraState</b>)
    *   Integer counter (<b>CountState</b>)
    *   HIPS balanced tree state (<b>HipsTreeState</b>)
    *   Image data of any format (<b>ImageState</b>)
    *   Variable-size list data of any type (<b>ListState</b>)
    *   Matrix/vector data of small fixed size (<b>MatrixState</b>)
    *   <i>(u,v)</i> point cloud (<b>UvState</b>)
    *   <i>(u,v,q)</i> point cloud (<b>UvqState</b>)
    *   <i>((u,v,q),(u,v))</i> point pair cloud (<b>UvqUvState</b>)

    Remarks:

    *   States may be created once and linked by multiple steps.
    *   Most states have methods to translate data to/from reasonable C++ types.
    *   Each state is bound to exactly one worker.



*   <h3>C++ classes defining processing <i>steps</i></h3>

    *   See "Included algorithms" above.

    Remarks:

    *   Steps may link multiple input and output states.
    *   Steps may be created once and run repeatedly as part of a pipeline.
    *   Steps may be timed individually.
    *   Each step is bound to exactly one worker, inferred from the worker of its states.



*   <h3>Test driver programs</h3>

    *   Messy; <i>not</i> intended for code reuse.
    *   May serve as working examples for library consumers.
