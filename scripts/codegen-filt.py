OFFSETS = [
    (x, y)
    for y in [-1, 0, 1]
    for x in [-1, 0, 1]
]

print """
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

    // Read other scores in a tight square around the candidate score."""

for (shift, (x, y)) in enumerate(OFFSETS):
    print ("    int  const p%02d = read_imagei(scores, sampler, xy + (int2)(%2d, %2d)).x;" % (shift + 1, x, y))
print

print "    // Select the maximum score."
print "    int        sco = p00;"
for (shift, _) in enumerate(OFFSETS):
    print "               sco = max(sco, p%02d);" % (shift + 1)

print """
    // Keep this score if it is as good as the maximum.
    if (p00 >= sco) {
        // Atomically append to filtered corner buffer.
        filtered[atom_inc(icorner)] = xy;
    }
}
"""
