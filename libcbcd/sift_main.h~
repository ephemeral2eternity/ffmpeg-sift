#ifndef SIFTMAIN_H
#define SIFTMAIN_H

#include "highgui.h"
#include "cxcore.h"
#include "stdint.h"
#include <cv.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <mysql.h>

/******************************** Structures *********************************/

/** holds feature data relevant to detection */
struct detection_data
{
	int r;
	int c;
	int octv;
	int intvl;
	double subintvl;
	double scl_octv;
};

struct feature;


/******************************* Defs and macros *****************************/

/** default number of sampled intervals per octave */
/**可选值1，2，3，4，5，6，7，8.文章中3 */
#define SIFT_INTVLS 3

/** default sigma for initial gaussian smoothing */
/*可选值1.0, 1.2, 1.4, 1.6, 1.8, 2.0.文章中1.6*/
#define SIFT_SIGMA 1.6

/** default threshold on keypoint contrast |D(x)| */
/*可选0.02~0.04. 文章中0.03*/
#define SIFT_CONTR_THR 0.03

/** default threshold on keypoint ratio of principle curvatures */
/*文章中10*/
#define SIFT_CURV_THR 10

/** double image size before pyramid construction? */
#define SIFT_IMG_DBL 1

/** default width of descriptor histogram array */
#define SIFT_DESCR_WIDTH 4

/** default number of bins per histogram in descriptor array */
#define SIFT_DESCR_HIST_BINS 8

/* assumed gaussian blur for input image */
#define SIFT_INIT_SIGMA 0.5

/* width of border in which to ignore keypoints */
#define SIFT_IMG_BORDER 5

/* maximum steps of keypoint interpolation before failure */
#define SIFT_MAX_INTERP_STEPS 5

/* default number of bins in histogram for orientation assignment */
#define SIFT_ORI_HIST_BINS 36

/* determines gaussian sigma for orientation assignment */
#define SIFT_ORI_SIG_FCTR 1.5

/* determines the radius of the region used in orientation assignment */
#define SIFT_ORI_RADIUS 3.0 * SIFT_ORI_SIG_FCTR

/* number of passes of orientation histogram smoothing */
#define SIFT_ORI_SMOOTH_PASSES 2

/* orientation magnitude relative to max that results in new feature */
#define SIFT_ORI_PEAK_RATIO 0.8

/* determines the size of a single descriptor orientation histogram */
#define SIFT_DESCR_SCL_FCTR 3.0

/* threshold on magnitude of elements of descriptor vector */
#define SIFT_DESCR_MAG_THR 0.2

/* factor used to convert floating-point descriptor to unsigned char */
#define SIFT_INT_DESCR_FCTR 512.0

/* returns a feature's detection data */
#define feat_detection_data(f) ( (struct detection_data*)(f->feature_data) )

// Add by WCC, cbcd group, 2009-0619
#define def_host_name "localhost"
#define def_user_name "cbcd"
#define def_password  "cbcd2009"
#define def_db_name   "cbcd_feat_sift"

MYSQL *cnx_init;
MYSQL *cnx_db;
int sampleNum;

/*************************** Function Prototypes *****************************/

/**
Finds SIFT features in an image using default parameter values.  All
detected features are stored in the array pointed to by \a feat.

@param img the image in which to detect features
@param feat a pointer to an array in which to store detected features

@return Returns the number of features stored in \a feat or -1 on failure
@see _sift_features()
*/

/**
Finda SIFT features in an image using user-specified parameter values.  All
detected features are stored in the array pointed to by \a feat.

@param img the image in which to detect features
@param feat a pointer to an array in which to store detected features
@param intvls the number of intervals sampled per octave of scale space
@param sigma the amount of Gaussian smoothing applied to each image level
	before building the scale space representation for an octave
@param contr_thr a threshold on the value of the scale space function
	\f$\left|D(\hat{x})\right|\f$, where \f$\hat{x}\f$ is a vector specifying
	feature location and scale, used to reject unstable features;  assumes
pixel values in the range [0, 1]
@param curv_thr threshold on a feature's ratio of principle curvatures
	used to reject features that are too edge-like
@param img_dbl should be 1 if image doubling prior to scale space
	construction is desired or 0 if not
@param descr_width the width, \f$n\f$, of the \f$n \times n\f$ array of
	orientation histograms used to compute a feature's descriptor
@param descr_hist_bins the number of orientations in each of the
	histograms in the array used to compute a feature's descriptor

@return Returns the number of keypoints stored in \a feat or -1 on failure
@see sift_features()
*/
/************************* Local Function Prototypes *************************/

IplImage* create_init_img( IplImage*, int, double );
IplImage* convert_to_gray32( IplImage* );
IplImage*** build_gauss_pyr( IplImage*, int, int, double );
IplImage* downsample( IplImage* );
IplImage*** build_dog_pyr( IplImage***, int, int );
CvSeq* scale_space_extrema( IplImage***, int, int, double, int, CvMemStorage*);
int is_extremum( IplImage***, int, int, int, int );
struct feature* interp_extremum( IplImage***, int, int, int, int, int, double);
void interp_step( IplImage***, int, int, int, int, double*, double*, double* );
CvMat* deriv_3D( IplImage***, int, int, int, int );
CvMat* hessian_3D( IplImage***, int, int, int, int );
double interp_contr( IplImage***, int, int, int, int, double, double, double );
struct feature* new_feature( void );
int is_too_edge_like( IplImage*, int, int, int );
void calc_feature_scales( CvSeq*, double, int );
void adjust_for_img_dbl( CvSeq* );
void calc_feature_oris( CvSeq*, IplImage*** );
double* ori_hist( IplImage*, int, int, int, int, double );
int calc_grad_mag_ori( IplImage*, int, int, double*, double* );
void smooth_ori_hist( double*, int );
double dominant_ori( double*, int );
void add_good_ori_features( CvSeq*, double*, int, double, struct feature* );
struct feature* clone_feature( struct feature* );
void compute_descriptors( CvSeq*, IplImage***, int, int );
double*** descr_hist( IplImage*, int, int, double, double, int, int );
void interp_hist_entry( double***, double, double, double, double, int, int);
void hist_to_descr( double***, int, int, struct feature* );
void normalize_descr( struct feature* );
int feature_cmp( void*, void*, void* );
void release_descr_hist( double****, int );
void release_pyr( IplImage****, int, int );
extern int sift_features( IplImage* img, struct feature** feat, int intvls,
						  double sigma, double contr_thr, int curv_thr,
						  int img_dbl, int descr_width, int descr_hist_bins );

int sift_main(uint8_t *grayImg , int width, int height, int pLineByte, int frameNum, char *filename);
#endif
