/**
 * A example usage demo for LBP, based on OpenCV.
 *
 * !note: un-tested
 */

#include "lbp.h"
#include <cv.h>
#include <highgui.h>
#include <stdio.h>

static void _test() {

    IplImage *img = NULL;
    int sz = 0;
    double *feat_vec = NULL;
    unsigned char *lbp_img = NULL;

    if (!img) return ;
  
    struct lbp_setting s;
    
    img = = cvLoadImage("foo.jpg", 0);
    if (img == NULL) return ; 

    lbp_init(&s, 16, 16, 16, 16);
  
    sz = lbp_length(&s, img->width, img->height);
    
    feat_vec = (double*)malloc(sizeof(double) * sz);

    assert( feat_vec != NULL );
  
    lbp_img = (unsigned char *)malloc(sizeof(unsigned char) *
                                      img->widthStep *
                                      img->height);
    assert( lbp_img != NULL );
    
    /* perform LBP and store result image into lbp_img */
    lbp_process(img->imageData, img->width,
                img->widthStep, img->height,
                lbp_img);
    
    /* extratc dense lbp feature, normalizaed using L2 */
    lbp_extract(&s, lbp_img, img->width, img->widthStep,
                img->height, feat_vec, LBP_L2NORM);
  
    /* do something with the feat vec */
}

int main(int argc, char *argv[]) {
    _test();
    return 0;
}
