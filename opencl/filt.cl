
kernel void fast_filter(
    read_only  image2d_t   scores,
    global     int2      * corners,
    global     int2      * filtered,
    global     int       * icorner
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as 1D offset into corners.
    int  const ic  = get_global_id(0);
    int2 const xy  = corners[ic];

    // Read the candidate score.
    int  const p00 = read_imagei(scores, sampler, xy).x;

    // Read other scores in a tight square around the candidate score.
    int  const p01 = read_imagei(scores, sampler, xy + (int2)(-1, -1)).x;
    int  const p02 = read_imagei(scores, sampler, xy + (int2)( 0, -1)).x;
    int  const p03 = read_imagei(scores, sampler, xy + (int2)( 1, -1)).x;
    int  const p04 = read_imagei(scores, sampler, xy + (int2)(-1,  0)).x;
    int  const p05 = read_imagei(scores, sampler, xy + (int2)( 0,  0)).x;
    int  const p06 = read_imagei(scores, sampler, xy + (int2)( 1,  0)).x;
    int  const p07 = read_imagei(scores, sampler, xy + (int2)(-1,  1)).x;
    int  const p08 = read_imagei(scores, sampler, xy + (int2)( 0,  1)).x;
    int  const p09 = read_imagei(scores, sampler, xy + (int2)( 1,  1)).x;

    // Select the maximum score.
    int        sco = p00;
               sco = max(sco, p01);
               sco = max(sco, p02);
               sco = max(sco, p03);
               sco = max(sco, p04);
               sco = max(sco, p05);
               sco = max(sco, p06);
               sco = max(sco, p07);
               sco = max(sco, p08);
               sco = max(sco, p09);

    // Keep this score if it is as good as the maximum.
    if (p00 >= sco) {
        // Atomically append to filtered corner buffer.
        filtered[atom_inc(icorner)] = xy;
    }
}

