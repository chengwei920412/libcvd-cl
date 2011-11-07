#include <CL/cl.h>

#include <stdlib.h>

static inline cl_ulong bitcount(cl_ulong v){
  v = v - ((v >> 1) & (cl_ulong)~(cl_ulong)0/3);
  v = (v & (cl_ulong)~(cl_ulong)0/15*3) + ((v >> 2) & (cl_ulong)~(cl_ulong)0/15*3);
  v = (v + (v >> 4)) & (cl_ulong)~(cl_ulong)0/255*15;
  return (cl_ulong)(v * ((cl_ulong)~(cl_ulong)0/255)) >> (sizeof(cl_ulong) - 1) * 8;
}

static inline cl_ulong bitcount4(cl_ulong4 const * restrict const t, cl_ulong4 const * restrict const r) {
    cl_ulong total = 0;
    for (cl_uint i = 0; i < 4; i++)
        total += bitcount(t->s[i] & ~(r->s[i]));
    return total;
}

static inline void rotate4(cl_ulong4 * restrict const r, cl_ulong4 const * restrict const t, cl_ulong const lshift) {
    cl_ulong const rshift = (((cl_ulong) 64) - lshift);
    for (cl_uint i = 0; i < 4; i++)
        r->s[i] = ((t->s[i] << lshift) | (t->s[i] >> rshift));
}

cl_uint HipsTreeFindStep_findByQueue(
    cl_int2         * restrict const pairs,
    cl_ulong4 const * restrict const tree,
    cl_ulong4 const * restrict const tests,
    cl_ushort const * restrict const maps,
    cl_uint                    const nKeepNodes,
    cl_uint                    const iTreeLeaf0,
    cl_uint                    const ntests,
    cl_uint                    const nrot,
    cl_uint                    const maxerr
) {

    // Prepare integer for atomic input counter.
    cl_uint atom_itest = 0;

    // Prepare integer for atomic output counter.
    cl_uint atom_ipair = 0;

    #pragma omp parallel default(shared)
    {
        cl_uint * const queue = (cl_uint *) malloc(nKeepNodes * sizeof(cl_uint));
        int itail = 0;
        int ihead = 0;

        while (1) {
            // Atomically retrieve test descriptor index.
            cl_uint const itest = __sync_fetch_and_add(&atom_itest, 1);
            if (itest >= ntests)
                break;

            // Refer to test descriptor.
            cl_ulong4 const * restrict const test0 = (tests + itest);

            // Try all rotations.
            for (cl_uint irot = 0; irot < nrot; irot++) {
                // Rotate descriptor.
                cl_ulong4 test;
                rotate4(&test, test0, irot * 4);

                // Seed queue with single tree root.
                queue[0] = 0;
                ihead = 0;
                itail = 1;

                // Perform selective breadth-first search of the tree.
                while (ihead < itail) {
                    // Dequeue first element.
                    cl_uint const inode = queue[ihead++];

                    // Refer to descriptor node.
                    cl_ulong4 const * restrict const node = (tree + inode);

                    // Calculate descriptor pair error.
                    cl_uint const error = bitcount4(&test, node);

                    // Check error against threshold.
                    if (error <= maxerr) {
                        if (inode >= iTreeLeaf0) {
                            // This is a leaf, record the match.
                            cl_uint const ileaf = maps[inode - iTreeLeaf0];
                            cl_int2 const pair = {{ileaf, itest}};

                            cl_uint const ipair = __sync_fetch_and_add(&atom_ipair, 1);
                            pairs[ipair] = pair;
                        } else {
                            // This is an internal node, add its children to the queue.
                            cl_uint const inext0 = (inode  * 2);
                            cl_uint const inext1 = (inext0 + 1);
                            cl_uint const inext2 = (inext0 + 2);
                            queue[itail++] = inext2;
                            queue[itail++] = inext1;
                        }
                    }
                }
            }
        }

        free(queue);
    }

    return atom_ipair;
}
