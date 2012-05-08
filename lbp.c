/**
 * Implementation of LBP (ignore the border).
 *
 * @TODO:
 *  :0 int -> double conversion optimization
 *  :1 parallel processing and block-based extraction.
 * 
 * @blackball<bugway@gmail.com>
 */

#include "lbp2.h"
#include <string.h>
#include <math.h>

EXTERN_BEGIN

/* Uniform pattern value lookup */
static const unsigned char _uniform_patterns[ 256 ] = {
    0,1,2,3,4,58,6,7,8,58,58,58,12,58,14,15,
    16,58,58,58,58,58,58,58,24,58,58,58,28,58,30,31,
    32,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
    48,58,58,58,58,58,58,58,56,58,58,58,60,58,62,63,
    64,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
    58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
    96,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
    112,58,58,58,58,58,58,58,120,58,58,58,124,58,126,127,
    128,129,58,131,58,58,58,135,58,58,58,58,58,58,58,143,
    58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,159,
    58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
    58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,191,
    192,193,58,195,58,58,58,199,58,58,58,58,58,58,58,207,
    58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,223,
    224,225,58,227,58,58,58,231,58,58,58,58,58,58,58,239,
    240,241,58,243,58,58,58,247,248,249,58,251,252,253,254,255
};

/* LBP histogram index lookup */
static const unsigned char _hist_index[ 256 ] = {
    0,1,2,3,4,58,5,6,7,58,58,58,8,58,9,10,11,58,58,58,
    58,58,58,58,12,58,58,58,13,58,14,15,16,58,58,58,58,
    58,58,58,58,58,58,58,58,58,58,58,17,58,58,58,58,58,
    58,58,18,58,58,58,19,58,20,21,22,58,58,58,58,58,58,
    58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
    58,58,58,58,58,58,58,58,23,58,58,58,58,58,58,58,58,
    58,58,58,58,58,58,58,24,58,58,58,58,58,58,58,25,58,
    58,58,26,58,27,28,29,30,58,31,58,58,58,32,58,58,58,
    58,58,58,58,33,58,58,58,58,58,58,58,58,58,58,58,58,
    58,58,58,34,58,58,58,58,58,58,58,58,58,58,58,58,58,
    58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
    58,35,36,37,58,38,58,58,58,39,58,58,58,58,58,58,58,
    40,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,41,
    42,43,58,44,58,58,58,45,58,58,58,58,58,58,58,46,47,
    48,58,49,58,58,58,50,51,52,58,53,54,55,56,57
};

void lbp_set(struct lbp_setting *setting,
             int win_w, int win_h,
             int step_x, int step_y) {
    
    setting->win_w = win_w;
    setting->win_h = win_h;  

    /* default configuration */
    setting->step_x = step_x; /* no overlap */
    setting->step_y = step_y; 
    setting->bin_num = 59;
    setting->radius = 1;
}

int lbp_length(const struct LbpSetting *setting,
               int w, int ws, int h) {
    if (setting->win_w > w ||
        setting->win_h > h)
        return 1;
  
    return
            (1 + (w - setting->win_w) / setting->step_x) *
            (1 + (h - setting->win_h) / setting->step_y) *
            setting->bin_num;
}

/**
 * Calculate LBP image.
 *
 * @buffer gray-scale image data
 * @w width of image
 * @ws width step or stride 
 * @h height of image
 * @lbpImage LBP image
 */
void lbp_process(const unsigned char *buffer,
                 int w, int ws, int height,
                 unsigned char *lbp_image) {
    int x,y,t;
    const unsigned char *pBuff = buffer;
    unsigned char *pLbp = lbp_image;
    /* ignore the borders */
    for (y = 1; y < h - 1; ++y) {
        t = y * ws;
        for (x = 1; x < width - 1; ++x) {
            unsigned char val;
            unsigned char orig = *(pBuff + t + x);
            unsigned char v[8] = {0};
#define _get_val(vi, px, pyw) v[ vi ] = *(pBuff + (pyw) + (px));
            /* 6 7 8
               5 0 1
               4 3 2 
               order:87654321 
            */
            _get_val(5, t-ws, x-1);
            _get_val(6, t-ws,   x);
            _get_val(7, t-ws, x+1);
            _get_val(4, t, x-1);
            _get_val(0, t, x+1);
            _get_val(3, t+ws, x-1);
            _get_val(2, t+ws, x);
            _get_val(1, t+ws, x+1);
#undef _get_val
            v[0] = v[0] > orig;
            v[1] = v[1] > orig;
            v[2] = v[2] > orig;
            v[3] = v[3] > orig;
            v[4] = v[4] > orig;
            v[5] = v[5] > orig;
            v[6] = v[6] > orig;
            v[7] = v[7] > orig;

            /* calcLBP */
            val = ((v[7] << 7) | (v[6] << 6) |
                   (v[5] << 5) | (v[4] << 4) |
                   (v[3] << 3) | (v[2] << 2) |
                   (v[1] << 1) | v[0]);
      
            *(pLbp + y * ws + x) = _uniform_patterns[ val & 0xFF ];   
        }
    }
}

void lbp_extract(const struct lbp_setting *setting,
                 const unsigned char *lbp_image,
                 int w, int ws, int h,
                 double *feat_vec,
                 int norm_type) {

    int
            win_w = setting->win_w,
            win_h = setting->win_h,
            step_x = setting->step_x,
            step_y = setting->step_y,
            win_num_x = 1 + (w - win_w) / step_x,
            win_num_y = 1 + (h - win_h) / step_y;
  
    const unsigned char *pLbp = lbp_image;

    double *pFeat = feat_vec;
    int feat_count = 0;
    double sqSum;
    int i,j,ix,iy,bx,by,t,sz,sum;
    int blockHist[ 59 ] = {0};

    for (iy = 0; iy < height - win_h + 1; iy += step_y) {
        for (ix = 0; ix < width - win_w + 1; ix += step_x) {
            memset(blockHist, 0, sizeof(int)*59);
      
            for (by = 0; by < win_h; ++by) {
                t = (iy + by) * ws;
                for (bx = 0; bx < win_w; ++bx) {
                    i = *(pLbp + t + ix+bx);
                    ++ blockHist[ _hist_index[ *(pLbp + t + ix+bx) & 0xFF ] ];
                }
            }
      
            /* normalize block */
            switch(norm_type) {
                
                case 0: /* do nothing */
                    for (i = 0; i < 59; ++i)
                        pFeat[ feat_count++ ] = blockHist[i];
                    break;
          
                case 1: /* L1 norm */
                    sz = win_w * win_h;
                    for (i = 0; i < 59; ++i)
                        pFeat[ feat_count++ ] = blockHist[i] * ( 1.0 / sz);
                    break;
          
                case 2: /* L2 norm */
                    /* It can't overflow here */
                    sum = 0;
                    sqSum = .0;
                    for (i = 0; i < 59; ++i)
                        sum += blockHist[ i ] * blockHist[ i ];
                    sqSum = 1.0 / sqrt((double)sum);
                    for (j = 0; j < 59; ++j)
                        pFeat[ feat_count++ ] = blockHist[j] * sqSum; 
                    break;
          
                default: /* unsupported, do nothing */
                    /**
                     * @TODO give a warning msg ?
                     */
                    for (i = 0; i < 59; ++i)
                        pFeat[ feat_count++ ] = blockHist[i];
            }
        }
    }
}

EXTERN_END
