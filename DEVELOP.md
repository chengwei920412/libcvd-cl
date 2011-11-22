libcvd-cl developer HOWTO
=========================

Requisite skills
----------------

*   ISO C++ 2003
*   OpenCL 1.0
*   Python 2.x
*   Boost C++

Recommended reading for OpenCL
------------------------------

Of the listed skills, OpenCL is the rarest, but increasingly important in
modern scientific computation, computer graphics and computer vision.
This entire section is dedicated to listing educational resources for developers.

For some introductory material, see the following presentations and documents.

*   [OpenCL - Parallel computing for CPUs and GPUs (AMD)](http://developer.amd.com/zones/OpenCLZone/courses/Documents/AMD_OpenCL_Tutorial_SAAHPC2010.pdf)
*   [Image Convolution Using OpenCL - A Step-by-Step Tutorial (AMD)](http://developer.amd.com/sdks/amdappsdk/documentation/ImageConvolutionOpenCL/Pages/ImageConvolutionUsingOpenCL.aspx)
*   [OpenCL on the GPU (NVIDIA)](http://www.nvidia.com/content/GTC/documents/1409_GTC09.pdf)
*   [OpenCL Overview (Intel)](http://www.haifux.org/lectures/267/OpenCL_for_Halifux_new.pdf)

More detailed documents are available that elaborate on many details and practical decisions.

*   [AMD Accelerated Parallel Processing OpenCL Programming Guide (AMD)](http://developer.amd.com/sdks/AMDAPPSDK/documentation/Pages/default.aspx)
*   [OpenCL Programming Guide <i>and</i> OpenCL Best Practices Guide (NVIDIA)](http://developer.nvidia.com/opencl/)

Many online tutorials are available, some are selected below.

*   [Introductory Tutorial to OpenCL (AMD)](http://developer.amd.com/SDKS/AMDAPPSDK/documentation/pages/TutorialOpenCL.aspx)
*   [Adventures in OpenCL](http://enja.org/category/tutorial/advcl/)

The official reference card is very useful, especially to get a quick overview of types and functions.
Try to avoid using 1.1 and 1.2 features, as not all implementations support these newer standards.

*   [OpenCL 1.0 Quick Reference](http://www.khronos.org/files/opencl-quick-reference-card.pdf)
*   [OpenCL 1.1 Quick Reference](http://www.khronos.org/files/opencl-1-1-quick-reference-card.pdf)
*   [OpenCL 1.2 Quick Reference](http://www.khronos.org/files/opencl-1-2-quick-reference-card.pdf)

There are also a few books that may be worth reading.

*   [Heterogeneous Computing with OpenCL](http://www.amazon.com/dp/0123877660/)
*   [OpenCL Programming Guide](http://www.amazon.com/dp/0321749642/)

Because OpenCL is in some ways similar to CUDA, and the NVIDIA CUDA
software contains a powerful implementation of OpenCL, it is helpful to read
CUDA documentation to learn details useful to OpenCL development and optimisation.

*   [NVIDIA GPU Programming Guide](http://developer.nvidia.com/nvidia-gpu-programming-guide)

There is a huge amount of information to consider when learning OpenCL, but where
libcvd-cl is concerned, pay attention to the following topics:

*   Differences in GPU execution architecture (compute units, work groups, warps)
*   Differences in GPU memory architecture (global memory, local/shared memory, constant memory, image/texture memory, implicit caches)
*   The C API, e.g. [<code>clEnqueueNDRangeKernel()</code>](http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clEnqueueNDRangeKernel.html)
*   [Vector types](http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/vectorDataTypes.html), operators and functions in OpenCL C, e.g. <code>cl_int4</code>
*   Image objects, their [host API](http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clCreateImage2D.html) and their [OpenCL C API](http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/imageFunctions.html).
*   [Atomic functions](http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/cl_khr_global_int32_base_atomics.html)

OpenCL workflow
---------------

*   Identify a computation step with the following traits:

    *   The step consumes a considerable amount of your total runtime.
    *   The total runtime is currently unacceptable (e.g. not frame-rate).
    *   The computation appears to be embarrassingly parallel.
    *   The computation does not require much memory for input or output.
    *   The computation does not require non-math API calls, such as IO or malloc.

*   Attempt to optimise the computation within its current language.

    Before introducing risk of bugs through parallel programming,
    you should first attempt to resolve performance issues within the
    algorithm itself.

    Even if it still isn't fast enough, you now have a faster algorithm
    to attempt to make parallel, and your overall improvement may be greater.

*   Attempt to use OpenMP or Cilk directives.

    As a general rule, if it isn't much faster using OpenMP, it won't be much
    faster using OpenCL either, as OpenCL places far more constaints on the
    algorithm than OpenMP does.

    There is no shortcut to understanding parallel processing.
    If you can't do it with OpenMP, you can't do it with OpenCL.
    If you can do it with OpenMP, you <i>may</i> be able to do it with OpenCL.

    If your algorithm is now <i>N-1</i> or even <i>N</i> times faster on your
    <i>N</i>-core CPU, then your algorithm is likely embarrassingly parallel
    and you can attempt to improve it further with OpenCL.

*   Attempt to use OpenCL.

    See the resources above and the tips in this very document.

    Some computations may end up hundreds of times faster, some will stay
    about the same, and some may end up slower.
    There are many examples of each, even in libcvd-cl.

OpenCL general "gotchas"
------------------------

*   Test on as many implementations as possible. It is possible to have 4+ implementations
    on the one system and run each of them from one program. You may find that
    OpenCL implementations have bugs, or that you are triggering openly defined
    behaviour.

    When you have several implementations, you can use Occam's Razor as a reasoning aid.

    *   If all consistently give the correct result, your code is very likely <i>correct</i>.
    *   If all but one consistently give the correct result, your code is still
        very likely <i>correct</i>, and an implementation is buggy. Try rewriting
        computations and tweaking execution parameters. Try on smaller data sets,
        and with smaller work group sizes, and try simplifying the algorithm.
    *   If one consistently gives the correct result and the others don't, your code is almost
        certainly <i>incorrect</i>, and it relies on a forgiving or quirky implementation.
    *   If all consistently give incorrect results, or inconsistently give correct/incorrect
        results, your code is almost certainly <i>incorrect</i>.

*   You can share type definitions like structs between C/C++ host code and OpenCL C
    device code, but be absolutely sure the compilers interpret the definitions the same
    way. Don't assume the byte order is the same. Don't keep pointers as data elements,
    only use pointers to refer to buffers of primitives or structures of primitives.

*   The OpenCL API is not required to be thread-safe in 1.0, so avoid accessing OpenCL
    APIs concurrently. libcvd-cl is assumed to be called from single-threaded contexts.

*   Due to compiler bugs and inadequacies, your code can silently fail or give incorrect
    results. If registers cannot be allocated to your code correctly, the result may
    be unexpectedly slow, or entirely incorrect. If you access more constant memory
    than is available, the kernel can silently abort but appear to have completed.
    Many, many unusual things can happen, so test thoroughly, and when something goes
    wrong, check the basics like memory bounds first.

*   Don't assume double precision floating point is available. This is currently
    optional, and not all devices physically support it.

*   Vector types make code on some devices faster, but not others.
    Use vector types where possible, because they rarely if ever make code slower.
    When you use the vector types, use the vector operators and functions too,
    don't repeat per-element code as you would in C.

*   Kernel thread barriers only apply within a work group, not across work groups within
    the same kernel execution. If this is a problem, split the kernel execution into
    multiple executions, or try to perform communication using atomic functions.


OpenCL on GPU "gotchas"
-----------------------

OpenCL is a portable language and standard, and will execute on CPU and GPU devices,
as well as other devices like the Cell SPU. Although programs may execute correctly
on a variety of devices, performance may diverge wildly from what is expected,
and different implementations may be vulnerable to different bugs, so
be aware of the following very general ideas.

*   NVIDIA CUDA OpenCL is one of the better implementations, but it
    has many interesting bugs of its own.

    *   It caches compiled programs, but the cache key is not complete enough,
        so a cached program can be unsuitable based on incidental changes.
        This will happen often, so just clear your <code>~/.nv/</code> directory entirely
        if your program is buggy or crashes.
        Mount this directory using tmpfs so that the cache is faster and is cleared on reboot.

    *   It can sometimes inexplicably fail to compile vector arithmetic.
         Try restating the expression, e.g. by breaking it across multiple lines.

    *   It is sometimes forgiving of misuse of mapped/pinned memory buffers,
        so be very careful to validate against other implementations and the
        OpenCL specification, or your program may be incorrect under slightly
        different circumstances. Try to avoid mapped/pinned memory in general
        unless memory traffic really is your bottleneck.

*   Avoid execution branching. If you must branch, try to ensure that all threads
    in a warp (or even all threads in a work group) will branch the same way.
    If all threads in a kernel will branch a certain way, consider using a preprocessor
    macro instead.

*   Prefer to use memory in this order of preference: constant, image/texture, local, global.

    Constant is tiny but very fast, and read-only.

    Image memory is technically writable (sometimes emulated), but only fast for reading.
    You can get bonus wrapping/clipping and interpolation with image memory.
    Sometimes image memory can be faster than constant, and is generally much larger, but
    much less convenient to program, and not all implementations support images.
    Even when supported, not all implementations support all image formats.
    In fact, 1-byte-per-pixel images aren't required by the standard.

    Local memory must first be fed by something else like global memory, but is then faster
    than global memory. It is quite limited, and only visible within a work group, but can
    be very useful as a communication block or a write-once read-many cache.

*   Be especially careful with memory bound errors. While a memory error in a C program
    confuses or crashes only the program, a memory bound error on a GPU can crash the
    host computer, because they have privileged access to actual host memory.
    Try to test new kernels with a CPU OpenCL first before trusting them on a GPU OpenCL.
    You can use gdb and valgrind for CPU OpenCL programs, but watch out for false positive
    error reports.

*   Avoid long-running kernels on GPU, as this may pause your computer's
    graphical session, especially if you use a desktop environment based on OpenGL.
    Many implementations have watchdog timers to kill long-running kernels,
    which may save you while debugging, but don't assume these are available
    when releasing code.

*   Avoid floating point errors on the device, as they may also fail silently or
    return confusing values. Check the defined domain of functions and operators
    you use.

*   Trigonometric and transcendental functions are standard, but not all devices
    have many hardware units available. Performance can be surprisingly slower
    than straightforward arithmetic, more than the difference on CPU.

