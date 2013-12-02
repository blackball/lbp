/**
 * Here implemented the original LBP(8, 1), and with tools to
 * create its variants.
 *
 * @blackball
 */
#include "lbp-new.h"

typedef unsigned char uchar;
typedef unsigned int  uint;

/* This is the basic LBP operator.
 * 6 7 8
 * 5 c 1   D(c) = sign{8-c, 7-c, 6-c, ..., 1-c}; 
 * 4 3 2
 * Perform LBP operator on data, and store the result in buffer.
 *
 * @note the borders are ignored. 
 */
#define at(i, j) *((pdata) + (i) * ws + (j))
void
lbp_operator(const uchar *data, int w, int ws, int h, uint *buffer) {
        int i, j;
        const uchar *pdata = data;
        for (i = 1; i < h-1; ++i) {
                pdata = i * ws;
                for (j = 1; j < w-1; ++j) {
                        
                        const uchar block[9] = {
                                at(0,   0), at(0,  1), at(1,  1),
                                at(1,   0), at(1, -1), at(0, -1),
                                at(-1, -1), at(-1, 0), at(-1, 1)
                        };
                        
                        buffer[i * w + j] =
                                (block[1] > block[0]) << 0 |
                                (block[2] > block[0]) << 1 |
                                (block[3] > block[0]) << 2 |
                                (block[4] > block[0]) << 3 |
                                (block[5] > block[0]) << 4 |
                                (block[6] > block[0]) << 5 |
                                (block[7] > block[0]) << 6 |
                                (block[8] > block[0]) << 7;
                }
        }
}
#undef at

/* After the 'operator', the typical processing is to make it *uniform*.
 *
 * The *uniform* means the bit transition times is less or equal 2. Below is a 
 * function to find all uniform value in [0, 255], you'll see, it's 58 uniform values,
 * and for all other values, assign then to another value, then we get 59 values. This
 * is typically the uniform patterns.
 */

static uchar uniform_patterns[256] = {0}; /* a uniform mapping for all pixel values */

#define bit_at(c, i) (((c) >> (i)) & 0x1)
static __inline uint
lbp_uniform(const uint c) {
        return
                (bit_at(c, 7) ^ bit_at(c, 0)) +
                (bit_at(c, 0) ^ bit_at(c, 1)) +
                (bit_at(c, 1) ^ bit_at(c, 2)) +
                (bit_at(c, 2) ^ bit_at(c, 3)) +
                (bit_at(c, 3) ^ bit_at(c, 4)) +
                (bit_at(c, 4) ^ bit_at(c, 5)) +
                (bit_at(c, 5) ^ bit_at(c, 6)) +
                (bit_at(c, 6) ^ bit_at(c, 7));
}
#undef bit_at

static int
lbp_init_uniform(void) {
        uint i = 0;
        uint counter = 0;
        for (; i < 256; ++i) {
                uint u = lbp_uniform(i);
                if (u <= 2) {
                        uniform_patterns[i] = i;
                        ++ counter;
                }
        }

        ++counter;
        for (i = 1; i < 256; ++i) {
                if (uniform_patterns[i] == 0) {
                        uniform_patterns[i] = counter;
                }
        }
        return counter + 1; /* counter == 58, return 59 */
}

/* There're lots of mapping techniques, like rotation invariant, etc
 * take a look at: 
 * "Multiresolution Gray-Scale and Rotation Invariant Texture Classification with Local Binary Patterns"
 *
 * In practice, it's hard to say which one is better, you need benchmark to prove.
 */

/*
 * The next step is to make a histogram.
 *
 * Here we consider the 59 dimension uniform patterns.
 * The size of hist here is 59.
 *
 * @note, If we get the histogram of full image, we may need ignore the
 * borders. If we just do this in a sub area, it's no need to ignore
 * the *borders*.
 */

static void
lbp_histogram(const int *buffer, int w, int h, double *hist) {
        int i, j;
        for (i = 1; i < h-1; ++i) {
                for (j = 1; j < w-1; ++j){
                        hist[ buffer[i * w + j] ] += 1.0;
                }
        }
}

/* The last important thing is normalization.
 *
 * There're many several ways to do this: mean, min-max, max-inf, or sqrt-L2, 
 *
 * You still need to benchmark to get the best one for your purpose. Here implement
 * the sqrt-L2 one.
 */

static void
lbp_root_L2(double *hist, int n) {
        int i = 0;
        double sum = 0.0;

        for (; i < n; ++i) {
                sum += hist[i] * hist[i];
        }

        sum = 1.0 / (sum + (1E-15));

        for (i = 0; i < n; ++i) {
                hist[i] = sqrt(hist[i]) * sum;
        }
}


