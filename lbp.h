/**
 * Dense LBP feature, a clean implementation.
 *
 * @author blackball (bugway@gmail.com)
 */

#ifndef LBP_H
#define LBP_H

#ifdef __cplusplus
#define EXTERN_BEGIN extern "C" {
#define EXTERN_END   }
#else
#define EXTERN_BEGIN
#define EXTERN_END
#endif

EXTERN_BEGIN

#define LBP_NONORM 0 /* no normalization */
#define LBP_L1NORM 1
#define LBP_L2NORM 2

struct lbp_setting {
    int win_w;    /* sliding window width */
    int win_h;    /* sliding window height */
    int step_x;   /* step in x-direction, default = win_w */
    int step_y;   /* step in y-direction, default = win_h */
  
    int bin_num;  /* how much bin number in the histogram, default = 59 */
    int radius;   /* radius of lbp pattern, default = 1 */
};

void lbp_init(struct lbp_setting *setting,int win_w,
              int win_h, int step_x, int step_y);

int lbp_length(const struct lbp_setting *setting,
               int w, int h);

void lbp_process(const unsigned char *image_data,
                 int w, int ws, int h,
                 unsigned char *lbp_image);

void lbp_extract(const struct lbp_setting *setting,
                 const unsigned char *lbp_image,
                 int w, int ws, int h, double *feat_vec,
                 int norm_type);

EXTERN_END

#endif
