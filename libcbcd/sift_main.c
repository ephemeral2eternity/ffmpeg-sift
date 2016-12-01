/*
Functions for detecting SIFT image features.

For more information, refer to:

Lowe, D.  Distinctive image features from scale-invariant keypoints.
<EM>International Journal of Computer Vision, 60</EM>, 2 (2004),
pp.91--110.

Copyright (C) 2006  Rob Hess <hess@eecs.oregonstate.edu>

Note: The SIFT algorithm is patented in the United States and cannot be
used in commercial products without a license from the University of
British Columbia.  For more information, refer to the file LICENSE.ubc
that accompanied this distribution.

@version 1.1.1-20070913
*/
#include <string.h>
#include "imgfeatures.h"
#include "sift_main.h"
#include "siftutils.h"

/********************************* Globals ************************************/
int display = 1;
static int Intvls = SIFT_INTVLS;
static double Sigma = SIFT_SIGMA;
static double Contr_thr = SIFT_CONTR_THR;
static int Curv_thr = SIFT_CURV_THR;
static int Img_dbl = SIFT_IMG_DBL;
static int Descr_width = SIFT_DESCR_WIDTH;
static int Descr_hist_bins = SIFT_DESCR_HIST_BINS;

MYSQL_RES *result_set;
MYSQL_ROW row;

unsigned int ctr;

/*************************** Function Prototypes of mysql *******************************************/
void writeSift2Mysql(struct feature* features, int frameNum, char *filename, int siftNums);
void show_result_set (MYSQL_RES *in_result_set);

/*************************** imgfeatures.c *******************************************/
int import_oxfd_features( char*, struct feature** );
int export_oxfd_features( char*, struct feature*, int );
void draw_oxfd_features( IplImage*, struct feature*, int );
void draw_oxfd_feature( IplImage*, struct feature*, CvScalar );

int import_lowe_features( char*, struct feature** );
int export_lowe_features( char*, struct feature*, int );
void draw_lowe_features( IplImage*, struct feature*, int );
void draw_lowe_feature( IplImage*, struct feature*, CvScalar );


/********************************* sift_main ***********************************/
/* 函数功能：该函数实现了离线系统中对当前视频当前帧的sift特征的提取，*/
/* 并存储入mysql的cbcd_feat数据库*/
/*********************** INPUT ***********************************************/
/*uint8_t *grayImg —— 等间隔选择的关键帧图像 */
/*int width —— 等间隔选择的关键帧图像的宽度 */
/*int height —— 等间隔选择的关键帧图像的高度 */
/*int pLineByte —— 关键帧在内存中存储的宽度 */
/*int frameNum —— 关键帧I帧帧号 */
/*char *filename —— 当前视频名称 */
/*********************************jhx_0320************************************/

int sift_main(uint8_t *grayImg , int width, int height, int pLineByte, int frameNum, char *filename)
{
	IplImage* img;
	struct feature* features;
	int i , j , temp1 ,temp2;
	int n = 0;
	int indFileExist = 0;	// Add by WCC, CBCD group, 2009-0409
	char tmpChar;
	char num[3][10];
	int lastLineNum;
	char createOrder[100];
	
	// printf("=========================================================================\n");
	fprintf( stderr, "Finding SIFT features...\n" );
        /* 为需要提取sift特征的图像img分配内存空间 */
        img = cvCreateImage(cvSize(width , height) , 8 , 1);

        /* 初始化图像img */
	for(i = 0; i < height; i++)
        {
            temp1 = i * width;
            temp2 = i * pLineByte;
            for(j = 0; j < width; j++)
                img->imageData[temp1 + j] = (char)*(grayImg + temp2 + j);
        }
	
	if( ! img )
	{
		fprintf( stderr, "unable to load image from %s", filename );
		exit( 1 );
	}
	
	n = sift_features( img, &features, Intvls, Sigma, Contr_thr, Curv_thr,
			   Img_dbl, Descr_width, Descr_hist_bins );

	// Add by WCC, CBCD group, 2009-0601
	/*********************** FILE and MYSQL operations **********************/
	/* FILE *fp;
	FILE *fpInd;
	fp = fopen("/home/cbcd/Data/files/siftFeatures.txt", "a+");

	if( (fopen( "/home/cbcd/Data/files/siftIndex.txt", "r" )) == NULL )
	{
		indFileExist = -1;
		fpInd = fopen("/home/cbcd/Data/files/siftIndex.txt", "w+");
	}
	else
	{
		indFileExist = 1;
		fpInd = fopen("/home/cbcd/Data/files/siftIndex.txt", "a+");
	}

	for (i = 0; i < n; i ++)
	{
		fprintf(fp, "%d ", features[i].d);
		for (j = 0; j < features[i].d; j ++)
		{
			fprintf(fp, "%d ", (int) features[i].descr[j]);
		}
		fprintf(fp, "%f %f %f %f %f %f %f %f %f %f %f", 
			        features[i].x, features[i].y, features[i].scl, features[i].ori,
			        features[i].a, features[i].b, features[i].c,
					features[i].img_pt.x, features[i].img_pt.y,
					features[i].mdl_pt.x, features[i].mdl_pt.y);
		fprintf(fp, "\n");
	}

	if (indFileExist == -1)
	{
		fprintf(fpInd, "%d %d %d %d %s \n", n, 1, n + 1, frameNum, filename );
	}
	else
	{
		// 找到上一数据的末尾行号
		fseek(fpInd, -3, SEEK_END);
		tmpChar = (char) fgetc(fpInd);
		while( (tmpChar != '\n') && (ftell(fpInd) != 1) )
		{
			fseek(fpInd, -2, SEEK_CUR);
			tmpChar = (char) fgetc(fpInd);
		}
		fscanf( fpInd, "%s %s %s", num[0], num[1], num[2]);
		lastLineNum = atoi( num[2] );

		fseek(fpInd, 0, SEEK_END);
		fprintf(fpInd, "%d %d %d %d %s \n", n, lastLineNum + 1, lastLineNum + n + 1, frameNum, filename);
	}

	fclose(fpInd);
	fclose(fp);*/

	writeSift2Mysql(features, frameNum, filename, n); 
	/*********************** FILE and MYSQL operations **********************/

	fprintf( stderr, "Found %d features.\n", n );

	if( display )
	{
		draw_features( img, features, n);
		cvNamedWindow( filename, 1 );
		cvShowImage( filename, img );
		cvWaitKey( 1000 );
	}

	free( features );
	cvReleaseImage( &img );
	return 0;
}

/********************************* writeSift2Mysql *********************************/
/* FUNCTION：Write sift features into mysql database cbcd_feat */
/*********************** INPUT *****************************************************/
/* struct feature* features —— Sift features of current frame in current video */
/* int frameNum —— Current frame index */
/* char *filename —— Current video name */
/* int siftNums —— Number of sift points in current frame */
/********************************* WCC-090601 **************************************/
void writeSift2Mysql(struct feature* features, int frameNum, char *filename, int siftNums)
{
	int i, j;
	unsigned long lineNum;
	int vidID;
	char vidIDName[20];
	char selectVidID[100];
	char lnNumStr[20];
	char insertOrder1[4000];
	char insertOrder4[4000];
	char insertOrder2[200];
	char insertOrder3[200];
	char tmpOrder[100];
	char desName[20];
	char frmName[20];
	char indName[20];
	char tmp[20];
	MYSQL_RES *results;
	MYSQL_ROW record;	

	// printf("Writing sift features for frame %d of video %s now!!!\n", frameNum, filename);

	sprintf(desName, "sift_des_%s", filename);
	sprintf(frmName, "sift_frm_%s", filename);
	sprintf(indName, "sift_ind_%s", filename);

	// CBCD group, WCC-090603, get the line number of current tbl_sift_feat table!
	sprintf(tmpOrder, "SELECT count(0) FROM %s", desName);
	mysql_query(cnx_init, tmpOrder);
   	results = mysql_store_result(cnx_init);
	record = mysql_fetch_row(results);
	sprintf(lnNumStr, "%s", record[0]);
	lineNum = atol(lnNumStr);
	// printf("The line number is computed as %d!!!!!\n", lineNum);
	mysql_free_result (results);

	for (i = 0; i < siftNums; i ++)
	{
		sprintf(insertOrder1, "INSERT INTO %s VALUES (", desName);
		sprintf(insertOrder4, "INSERT INTO sift_sample VALUES (", desName);
		for (j = 0; j < 128; j ++)
		{	
			if (j < 127)
			{
				sprintf(tmp, "%d, ", (int) features[i].descr[j]);
			}
			else
			{
				sprintf(tmp, "%d )", (int) features[i].descr[j]);
			}
			strcat(insertOrder1, tmp);
			strcat(insertOrder4, tmp);
		}

		sprintf(insertOrder2, "INSERT INTO %s VALUES ( %f, %f, %f, %f )", frmName,
		        features[i].x, features[i].y, features[i].scl, features[i].ori );

		if (mysql_query (cnx_init, insertOrder1) != 0)
		{
			printf ("failure in inserting into %s.\n", desName);
		}

		if (sampleNum % 10 == 0)
		{
			if (mysql_query (cnx_init, insertOrder4) != 0)
			{
				printf ("failure in inserting into sample_sift.\n");
			}			
		}

		if (mysql_query (cnx_init, insertOrder2) != 0)
		{
			printf ("failure in inserting into %s.\n", frmName);
		}
	}

	sprintf(insertOrder3, "insert into %s values ( %d, %lu, %lu, %d)", indName, frameNum, lineNum + 1, lineNum + siftNums, siftNums);

	if (mysql_query (cnx_init, insertOrder3) != 0)
	{
		printf ("failure in inserting into %s.\n", indName);
	}
	else
	{
		// printf ("insert succeeded %s: %lu row(s) affected\n", indName, (unsigned long) mysql_affected_rows(cnx_init));
	}
	
}

/********************************* show_result_set *********************************/
/* FUNCTION：Show results of mysql order */
/*********************** INPUT *****************************************************/
/* MYSQL_RES *in_result_set —— The result of a mysql query */
/********************************* WCC-090601 **************************************/
void show_result_set (MYSQL_RES *in_result_set)
{
	while ((row = mysql_fetch_row (in_result_set)) != NULL)
	{
		for (ctr=0; ctr < mysql_num_fields (in_result_set); ctr ++)
		{
			if (ctr > 0)
				fputc ('\t', stdout);
			printf ("%s", row[ctr] != NULL ? row[ctr] : "Null-val5");
		}
		fputc ('\n', stdout);
	}
	if (mysql_errno (cnx_init) != 0)
	{
		printf ("failure in mysql_fetch_row\n");
		printf ("Exit code 3\n");
		printf ("Error %u -- %s\n", mysql_errno (cnx_init), mysql_error (cnx_init));
		exit (3);
	}

	mysql_free_result (in_result_set);
}

/*********************** Functions prototyped in sift.h **********************/

/**
Finds SIFT features in an image using user-specified parameter values.  All
detected features are stored in the array pointed to by \a feat.

@param img the image in which to detect features
@param fea a pointer to an array in which to store detected features
@param intvls the number of intervals sampled per octave of scale space
@param sigma the amount of Gaussian smoothing applied to each image level
	before building the scale space representation for an octave
@param cont_thr a threshold on the value of the scale space function
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
@see sift_keypoints()
*/
int sift_features( IplImage* img, struct feature** feat, int intvls,
				   double sigma, double contr_thr, int curv_thr,
				   int img_dbl, int descr_width, int descr_hist_bins )
{
	IplImage* init_img;
	IplImage*** gauss_pyr, *** dog_pyr;
	CvMemStorage* storage;
	CvSeq* features;
	int octvs, i, n = 0;
	
	/* build scale space pyramid; smallest dimension of top level is ~4 pixels */
	init_img = create_init_img( img, img_dbl, sigma );
	octvs = log( MIN( init_img->width, init_img->height ) ) / log(2) - 2;
	gauss_pyr = build_gauss_pyr( init_img, octvs, intvls, sigma );
	dog_pyr = build_dog_pyr( gauss_pyr, octvs, intvls );

	storage = cvCreateMemStorage( 0 );
	features = scale_space_extrema( dog_pyr, octvs, intvls, contr_thr,
		curv_thr, storage );
	calc_feature_scales( features, sigma, intvls );
	if( img_dbl )
		adjust_for_img_dbl( features );
	calc_feature_oris( features, gauss_pyr );
	compute_descriptors( features, gauss_pyr, descr_width, descr_hist_bins );

	/* sort features by decreasing scale and move from CvSeq to array */
	cvSeqSort( features, (CvCmpFunc)feature_cmp, NULL );
	n = features->total;
	*feat = calloc( n, sizeof(struct feature) );
	*feat = cvCvtSeqToArray( features, *feat, CV_WHOLE_SEQ );
	for( i = 0; i < n; i++ )
	{
		free( (*feat)[i].feature_data );
		(*feat)[i].feature_data = NULL;
	}

	cvReleaseMemStorage( &storage );
	cvReleaseImage( &init_img );
	release_pyr( &gauss_pyr, octvs, intvls + 3 );
	release_pyr( &dog_pyr, octvs, intvls + 2 );
//	free(*feat);
	return n;
}


/************************ Functions prototyped here **************************/

/*
Converts an image to 8-bit grayscale and Gaussian-smooths it.  The image is
optionally doubled in size prior to smoothing.

@param img input image
@param img_dbl if true, image is doubled in size prior to smoothing
@param sigma total std of Gaussian smoothing
*/
IplImage* create_init_img( IplImage* img, int img_dbl, double sigma )
{
	IplImage* gray, * dbl;
	float sig_diff;

	gray = convert_to_gray32( img );
	if( img_dbl )
	{
		sig_diff = sqrt( sigma * sigma - SIFT_INIT_SIGMA * SIFT_INIT_SIGMA * 4 );
		dbl = cvCreateImage( cvSize( img->width*2, img->height*2 ),
			IPL_DEPTH_32F, 1 );
		cvResize( gray, dbl, CV_INTER_CUBIC );
		cvSmooth( dbl, dbl, CV_GAUSSIAN, 0, 0, sig_diff, sig_diff );
		cvReleaseImage( &gray );
		return dbl;
	}
	else
	{
		sig_diff = sqrt( sigma * sigma - SIFT_INIT_SIGMA * SIFT_INIT_SIGMA );
		cvSmooth( gray, gray, CV_GAUSSIAN, 0, 0, sig_diff, sig_diff );
		return gray;
	}
}



/*
Converts an image to 32-bit grayscale

@param img a 3-channel 8-bit color (BGR) or 8-bit gray image

@return Returns a 32-bit grayscale image
*/
IplImage* convert_to_gray32( IplImage* img )
{
	IplImage* gray8, * gray32;

	gray8 = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
	gray32 = cvCreateImage( cvGetSize(img), IPL_DEPTH_32F, 1 );

	if( img->nChannels == 1 )
		gray8 = cvClone( img );
	else
		cvCvtColor( img, gray8, CV_BGR2GRAY );
	cvConvertScale( gray8, gray32, 1.0 / 255.0, 0 );

	cvReleaseImage( &gray8 );
	return gray32;
}



/*
Builds Gaussian scale space pyramid from an image

@param base base image of the pyramid
@param octvs number of octaves of scale space
@param intvls number of intervals per octave
@param sigma amount of Gaussian smoothing per octave

@return Returns a Gaussian scale space pyramid as an octvs x (intvls + 3) array
*/
IplImage*** build_gauss_pyr( IplImage* base, int octvs,
							int intvls, double sigma )
{
	IplImage*** gauss_pyr;
	double* sig = calloc( intvls + 3, sizeof(double));
	double sig_total, sig_prev, k;
	int i, o;

	gauss_pyr = calloc( octvs, sizeof( IplImage** ) );
	for( i = 0; i < octvs; i++ )
		gauss_pyr[i] = calloc( intvls + 3, sizeof( IplImage* ) );

	/*
		precompute Gaussian sigmas using the following formula:

		\sigma_{total}^2 = \sigma_{i}^2 + \sigma_{i-1}^2
	*/
	sig[0] = sigma;
	k = pow( 2.0, 1.0 / intvls );
	for( i = 1; i < intvls + 3; i++ )
	{
		sig_prev = pow( k, i - 1 ) * sigma;
		sig_total = sig_prev * k;
		sig[i] = sqrt( sig_total * sig_total - sig_prev * sig_prev );
	}

	for( o = 0; o < octvs; o++ )
		for( i = 0; i < intvls + 3; i++ )
		{
			if( o == 0  &&  i == 0 )
				gauss_pyr[o][i] = cvCloneImage(base);

			/* base of new octvave is halved image from end of previous octave */
			else if( i == 0 )
				gauss_pyr[o][i] = downsample( gauss_pyr[o-1][intvls] );

			/* blur the current octave's last image to create the next one */
			else
			{
				gauss_pyr[o][i] = cvCreateImage( cvGetSize(gauss_pyr[o][i-1]),
					IPL_DEPTH_32F, 1 );
				cvSmooth( gauss_pyr[o][i-1], gauss_pyr[o][i],
					CV_GAUSSIAN, 0, 0, sig[i], sig[i] );
			}
		}

	free( sig );
	return gauss_pyr;
}



/*
Downsamples an image to a quarter of its size (half in each dimension)
using nearest-neighbor interpolation

@param img an image

@return Returns an image whose dimensions are half those of img
*/
IplImage* downsample( IplImage* img )
{
	IplImage* smaller = cvCreateImage( cvSize(img->width / 2, img->height / 2),
		img->depth, img->nChannels );
	cvResize( img, smaller, CV_INTER_NN );

	return smaller;
}



/*
Builds a difference of Gaussians scale space pyramid by subtracting adjacent
intervals of a Gaussian pyramid

@param gauss_pyr Gaussian scale-space pyramid
@param octvs number of octaves of scale space
@param intvls number of intervals per octave

@return Returns a difference of Gaussians scale space pyramid as an
	octvs x (intvls + 2) array
*/
IplImage*** build_dog_pyr( IplImage*** gauss_pyr, int octvs, int intvls )
{
	IplImage*** dog_pyr;
	int i, o;

	dog_pyr = calloc( octvs, sizeof( IplImage** ) );
	for( i = 0; i < octvs; i++ )
		dog_pyr[i] = calloc( intvls + 2, sizeof(IplImage*) );

	for( o = 0; o < octvs; o++ )
		for( i = 0; i < intvls + 2; i++ )
		{
			dog_pyr[o][i] = cvCreateImage( cvGetSize(gauss_pyr[o][i]),
				IPL_DEPTH_32F, 1 );
			cvSub( gauss_pyr[o][i+1], gauss_pyr[o][i], dog_pyr[o][i], NULL );
		}

	return dog_pyr;
}



/*
Detects features at extrema in DoG scale space.  Bad features are discarded
based on contrast and ratio of principal curvatures.

@param dog_pyr DoG scale space pyramid
@param octvs octaves of scale space represented by dog_pyr
@param intvls intervals per octave
@param contr_thr low threshold on feature contrast
@param curv_thr high threshold on feature ratio of principal curvatures
@param storage memory storage in which to store detected features

@return Returns an array of detected features whose scales, orientations,
	and descriptors are yet to be determined.
*/
CvSeq* scale_space_extrema( IplImage*** dog_pyr, int octvs, int intvls,
						   double contr_thr, int curv_thr,
						   CvMemStorage* storage )
{
	CvSeq* features;
	double prelim_contr_thr = 0.5 * contr_thr / intvls;
	struct feature* feat;
	struct detection_data* ddata;
	int o, i, r, c;

	features = cvCreateSeq( 0, sizeof(CvSeq), sizeof(struct feature), storage );
	for( o = 0; o < octvs; o++ )
		for( i = 1; i <= intvls; i++ )
			for(r = SIFT_IMG_BORDER; r < dog_pyr[o][0]->height-SIFT_IMG_BORDER; r++)
				for(c = SIFT_IMG_BORDER; c < dog_pyr[o][0]->width-SIFT_IMG_BORDER; c++)
					/* perform preliminary check on contrast */
					if( ABS( pixval32f( dog_pyr[o][i], r, c ) ) > prelim_contr_thr )
						if( is_extremum( dog_pyr, o, i, r, c ) )
						{
							feat = interp_extremum(dog_pyr, o, i, r, c, intvls, contr_thr);
							if( feat )
							{
								ddata = feat_detection_data( feat );
								if( ! is_too_edge_like( dog_pyr[ddata->octv][ddata->intvl],
									ddata->r, ddata->c, curv_thr ) )
								{
									cvSeqPush( features, feat );
								}
								else
									free( ddata );
								free( feat );
							}
						}

	return features;
}



/*
Determines whether a pixel is a scale-space extremum by comparing it to it's
3x3x3 pixel neighborhood.

@param dog_pyr DoG scale space pyramid
@param octv pixel's scale space octave
@param intvl pixel's within-octave interval
@param r pixel's image row
@param c pixel's image col

@return Returns 1 if the specified pixel is an extremum (max or min) among
	it's 3x3x3 pixel neighborhood.
*/
int is_extremum( IplImage*** dog_pyr, int octv, int intvl, int r, int c )
{
	float val = pixval32f( dog_pyr[octv][intvl], r, c );
	int i, j, k;

	/* check for maximum */
	if( val > 0 )
	{
		for( i = -1; i <= 1; i++ )
			for( j = -1; j <= 1; j++ )
				for( k = -1; k <= 1; k++ )
					if( val < pixval32f( dog_pyr[octv][intvl+i], r + j, c + k ) )
						return 0;
	}

	/* check for minimum */
	else
	{
		for( i = -1; i <= 1; i++ )
			for( j = -1; j <= 1; j++ )
				for( k = -1; k <= 1; k++ )
					if( val > pixval32f( dog_pyr[octv][intvl+i], r + j, c + k ) )
						return 0;
	}

	return 1;
}



/*
Interpolates a scale-space extremum's location and scale to subpixel
accuracy to form an image feature.  Rejects features with low contrast.
Based on Section 4 of Lowe's paper.  

@param dog_pyr DoG scale space pyramid
@param octv feature's octave of scale space
@param intvl feature's within-octave interval
@param r feature's image row
@param c feature's image column
@param intvls total intervals per octave
@param contr_thr threshold on feature contrast

@return Returns the feature resulting from interpolation of the given
	parameters or NULL if the given location could not be interpolated or
	if contrast at the interpolated loation was too low.  If a feature is
	returned, its scale, orientation, and descriptor are yet to be determined.
*/
struct feature* interp_extremum( IplImage*** dog_pyr, int octv, int intvl,
								int r, int c, int intvls, double contr_thr )
{
	struct feature* feat;
	struct detection_data* ddata;
	double xi, xr, xc, contr;
	int i = 0;

	while( i < SIFT_MAX_INTERP_STEPS )
	{
		interp_step( dog_pyr, octv, intvl, r, c, &xi, &xr, &xc );
		if( ABS( xi ) < 0.5  &&  ABS( xr ) < 0.5  &&  ABS( xc ) < 0.5 )
			break;

		c += cvRound( xc );
		r += cvRound( xr );
		intvl += cvRound( xi );

		if( intvl < 1  ||
			intvl > intvls  ||
			c < SIFT_IMG_BORDER  ||
			r < SIFT_IMG_BORDER  ||
			c >= dog_pyr[octv][0]->width - SIFT_IMG_BORDER  ||
			r >= dog_pyr[octv][0]->height - SIFT_IMG_BORDER )
		{
			return NULL;
		}

		i++;
	}

	/* ensure convergence of interpolation */
	if( i >= SIFT_MAX_INTERP_STEPS )
		return NULL;

	contr = interp_contr( dog_pyr, octv, intvl, r, c, xi, xr, xc );
	if( ABS( contr ) < contr_thr / intvls )
		return NULL;

	feat = new_feature();
	ddata = feat_detection_data( feat );
	feat->img_pt.x = feat->x = ( c + xc ) * pow( 2.0, octv );
	feat->img_pt.y = feat->y = ( r + xr ) * pow( 2.0, octv );
	ddata->r = r;
	ddata->c = c;
	ddata->octv = octv;
	ddata->intvl = intvl;
	ddata->subintvl = xi;

	return feat;
}



/*
Performs one step of extremum interpolation.  Based on Eqn. (3) in Lowe's
paper.

@param dog_pyr difference of Gaussians scale space pyramid
@param octv octave of scale space
@param intvl interval being interpolated
@param r row being interpolated
@param c column being interpolated
@param xi output as interpolated subpixel increment to interval
@param xr output as interpolated subpixel increment to row
@param xc output as interpolated subpixel increment to col
*/

void interp_step( IplImage*** dog_pyr, int octv, int intvl, int r, int c,
				 double* xi, double* xr, double* xc )
{
	CvMat* dD, * H, * H_inv, X;
	double x[3] = { 0 };

	dD = deriv_3D( dog_pyr, octv, intvl, r, c );
	H = hessian_3D( dog_pyr, octv, intvl, r, c );
	H_inv = cvCreateMat( 3, 3, CV_64FC1 );
	cvInvert( H, H_inv, CV_SVD );
	cvInitMatHeader( &X, 3, 1, CV_64FC1, x, CV_AUTOSTEP );
	cvGEMM( H_inv, dD, -1, NULL, 0, &X, 0 );

	cvReleaseMat( &dD );
	cvReleaseMat( &H );
	cvReleaseMat( &H_inv );

	*xi = x[2];
	*xr = x[1];
	*xc = x[0];
}



/*
Computes the partial derivatives in x, y, and scale of a pixel in the DoG
scale space pyramid.

@param dog_pyr DoG scale space pyramid
@param octv pixel's octave in dog_pyr
@param intvl pixel's interval in octv
@param r pixel's image row
@param c pixel's image col

@return Returns the vector of partial derivatives for pixel I
	{ dI/dx, dI/dy, dI/ds }^T as a CvMat*
*/
CvMat* deriv_3D( IplImage*** dog_pyr, int octv, int intvl, int r, int c )
{
	CvMat* dI;
	double dx, dy, ds;

	dx = ( pixval32f( dog_pyr[octv][intvl], r, c+1 ) -
		pixval32f( dog_pyr[octv][intvl], r, c-1 ) ) / 2.0;
	dy = ( pixval32f( dog_pyr[octv][intvl], r+1, c ) -
		pixval32f( dog_pyr[octv][intvl], r-1, c ) ) / 2.0;
	ds = ( pixval32f( dog_pyr[octv][intvl+1], r, c ) -
		pixval32f( dog_pyr[octv][intvl-1], r, c ) ) / 2.0;

	dI = cvCreateMat( 3, 1, CV_64FC1 );
	cvmSet( dI, 0, 0, dx );
	cvmSet( dI, 1, 0, dy );
	cvmSet( dI, 2, 0, ds );

	return dI;
}



/*
Computes the 3D Hessian matrix for a pixel in the DoG scale space pyramid.

@param dog_pyr DoG scale space pyramid
@param octv pixel's octave in dog_pyr
@param intvl pixel's interval in octv
@param r pixel's image row
@param c pixel's image col

@return Returns the Hessian matrix (below) for pixel I as a CvMat*

	/ Ixx  Ixy  Ixs \ <BR>
	| Ixy  Iyy  Iys | <BR>
	\ Ixs  Iys  Iss /
*/
CvMat* hessian_3D( IplImage*** dog_pyr, int octv, int intvl, int r, int c )
{
	CvMat* H;
	double v, dxx, dyy, dss, dxy, dxs, dys;

	v = pixval32f( dog_pyr[octv][intvl], r, c );
	dxx = ( pixval32f( dog_pyr[octv][intvl], r, c+1 ) + 
			pixval32f( dog_pyr[octv][intvl], r, c-1 ) - 2 * v );
	dyy = ( pixval32f( dog_pyr[octv][intvl], r+1, c ) +
			pixval32f( dog_pyr[octv][intvl], r-1, c ) - 2 * v );
	dss = ( pixval32f( dog_pyr[octv][intvl+1], r, c ) +
			pixval32f( dog_pyr[octv][intvl-1], r, c ) - 2 * v );
	dxy = ( pixval32f( dog_pyr[octv][intvl], r+1, c+1 ) -
			pixval32f( dog_pyr[octv][intvl], r+1, c-1 ) -
			pixval32f( dog_pyr[octv][intvl], r-1, c+1 ) +
			pixval32f( dog_pyr[octv][intvl], r-1, c-1 ) ) / 4.0;
	dxs = ( pixval32f( dog_pyr[octv][intvl+1], r, c+1 ) -
			pixval32f( dog_pyr[octv][intvl+1], r, c-1 ) -
			pixval32f( dog_pyr[octv][intvl-1], r, c+1 ) +
			pixval32f( dog_pyr[octv][intvl-1], r, c-1 ) ) / 4.0;
	dys = ( pixval32f( dog_pyr[octv][intvl+1], r+1, c ) -
			pixval32f( dog_pyr[octv][intvl+1], r-1, c ) -
			pixval32f( dog_pyr[octv][intvl-1], r+1, c ) +
			pixval32f( dog_pyr[octv][intvl-1], r-1, c ) ) / 4.0;

	H = cvCreateMat( 3, 3, CV_64FC1 );
	cvmSet( H, 0, 0, dxx );
	cvmSet( H, 0, 1, dxy );
	cvmSet( H, 0, 2, dxs );
	cvmSet( H, 1, 0, dxy );
	cvmSet( H, 1, 1, dyy );
	cvmSet( H, 1, 2, dys );
	cvmSet( H, 2, 0, dxs );
	cvmSet( H, 2, 1, dys );
	cvmSet( H, 2, 2, dss );

	return H;
}



/*
Calculates interpolated pixel contrast.  Based on Eqn. (3) in Lowe's paper.

@param dog_pyr difference of Gaussians scale space pyramid
@param octv octave of scale space
@param intvl within-octave interval
@param r pixel row
@param c pixel column
@param xi interpolated subpixel increment to interval
@param xr interpolated subpixel increment to row
@param xc interpolated subpixel increment to col

@param Returns interpolated contrast.
*/
double interp_contr( IplImage*** dog_pyr, int octv, int intvl, int r,
					int c, double xi, double xr, double xc )
{
	CvMat* dD, X, T;
	double t[1], x[3] = { xc, xr, xi };

	cvInitMatHeader( &X, 3, 1, CV_64FC1, x, CV_AUTOSTEP );
	cvInitMatHeader( &T, 1, 1, CV_64FC1, t, CV_AUTOSTEP );
	dD = deriv_3D( dog_pyr, octv, intvl, r, c );
	cvGEMM( dD, &X, 1, NULL, 0, &T,  CV_GEMM_A_T );
	cvReleaseMat( &dD );

	return pixval32f( dog_pyr[octv][intvl], r, c ) + t[0] * 0.5;
}



/*
Allocates and initializes a new feature

@return Returns a pointer to the new feature
*/
struct feature* new_feature( void )
{
	struct feature* feat;
	struct detection_data* ddata;

	feat = malloc( sizeof( struct feature ) );
	memset( feat, 0, sizeof( struct feature ) );
	ddata = malloc( sizeof( struct detection_data ) );
	memset( ddata, 0, sizeof( struct detection_data ) );
	feat->feature_data = ddata;
	feat->type = FEATURE_LOWE;

	return feat;
}



/*
Determines whether a feature is too edge like to be stable by computing the
ratio of principal curvatures at that feature.  Based on Section 4.1 of
Lowe's paper.

@param dog_img image from the DoG pyramid in which feature was detected
@param r feature row
@param c feature col
@param curv_thr high threshold on ratio of principal curvatures

@return Returns 0 if the feature at (r,c) in dog_img is sufficiently
	corner-like or 1 otherwise.
*/
int is_too_edge_like( IplImage* dog_img, int r, int c, int curv_thr )
{
	double d, dxx, dyy, dxy, tr, det;

	/* principal curvatures are computed using the trace and det of Hessian */
	d = pixval32f(dog_img, r, c);
	dxx = pixval32f( dog_img, r, c+1 ) + pixval32f( dog_img, r, c-1 ) - 2 * d;
	dyy = pixval32f( dog_img, r+1, c ) + pixval32f( dog_img, r-1, c ) - 2 * d;
	dxy = ( pixval32f(dog_img, r+1, c+1) - pixval32f(dog_img, r+1, c-1) -
			pixval32f(dog_img, r-1, c+1) + pixval32f(dog_img, r-1, c-1) ) / 4.0;
	tr = dxx + dyy;
	det = dxx * dyy - dxy * dxy;

	/* negative determinant -> curvatures have different signs; reject feature */
	if( det <= 0 )
		return 1;

	if( tr * tr / det < ( curv_thr + 1.0 )*( curv_thr + 1.0 ) / curv_thr )
		return 0;
	return 1;
}



/*
Calculates characteristic scale for each feature in an array.

@param features array of features
@param sigma amount of Gaussian smoothing per octave of scale space
@param intvls intervals per octave of scale space
*/
void calc_feature_scales( CvSeq* features, double sigma, int intvls )
{
	struct feature* feat;
	struct detection_data* ddata;
	double intvl;
	int i, n;

	n = features->total;
	for( i = 0; i < n; i++ )
	{
		feat = CV_GET_SEQ_ELEM( struct feature, features, i );
		ddata = feat_detection_data( feat );
		intvl = ddata->intvl + ddata->subintvl;
		feat->scl = sigma * pow( 2.0, ddata->octv + intvl / intvls );
		ddata->scl_octv = sigma * pow( 2.0, intvl / intvls );
	}
}



/*
Halves feature coordinates and scale in case the input image was doubled
prior to scale space construction.

@param features array of features
*/
void adjust_for_img_dbl( CvSeq* features )
{
	struct feature* feat;
	int i, n;

	n = features->total;
	for( i = 0; i < n; i++ )
	{
		feat = CV_GET_SEQ_ELEM( struct feature, features, i );
		feat->x /= 2.0;
		feat->y /= 2.0;
		feat->scl /= 2.0;
		feat->img_pt.x /= 2.0;
		feat->img_pt.y /= 2.0;
	}
}



/*
Computes a canonical orientation for each image feature in an array.  Based
on Section 5 of Lowe's paper.  This function adds features to the array when
there is more than one dominant orientation at a given feature location.

@param features an array of image features
@param gauss_pyr Gaussian scale space pyramid
*/
void calc_feature_oris( CvSeq* features, IplImage*** gauss_pyr )
{
	struct feature* feat;
	struct detection_data* ddata;
	double* hist;
	double omax;
	int i, j, n = features->total;

	for( i = 0; i < n; i++ )
	{
		feat = malloc( sizeof( struct feature ) );
		cvSeqPopFront( features, feat );
		ddata = feat_detection_data( feat );
		hist = ori_hist( gauss_pyr[ddata->octv][ddata->intvl],
						ddata->r, ddata->c, SIFT_ORI_HIST_BINS,
						cvRound( SIFT_ORI_RADIUS * ddata->scl_octv ),
						SIFT_ORI_SIG_FCTR * ddata->scl_octv );
		for( j = 0; j < SIFT_ORI_SMOOTH_PASSES; j++ )
			smooth_ori_hist( hist, SIFT_ORI_HIST_BINS );
		omax = dominant_ori( hist, SIFT_ORI_HIST_BINS );
		add_good_ori_features( features, hist, SIFT_ORI_HIST_BINS,
								omax * SIFT_ORI_PEAK_RATIO, feat );
		free( ddata );
		free( feat );
		free( hist );
	}
}



/*
Computes a gradient orientation histogram at a specified pixel.

@param img image
@param r pixel row
@param c pixel col
@param n number of histogram bins
@param rad radius of region over which histogram is computed
@param sigma std for Gaussian weighting of histogram entries

@return Returns an n-element array containing an orientation histogram
	representing orientations between 0 and 2 PI.
*/
double* ori_hist( IplImage* img, int r, int c, int n, int rad, double sigma)
{
	double* hist;
	double mag, ori, w, exp_denom, PI2 = CV_PI * 2.0;
	int bin, i, j;

	hist = calloc( n, sizeof( double ) );
	exp_denom = 2.0 * sigma * sigma;
	for( i = -rad; i <= rad; i++ )
		for( j = -rad; j <= rad; j++ )
			if( calc_grad_mag_ori( img, r + i, c + j, &mag, &ori ) )
			{
				w = exp( -( i*i + j*j ) / exp_denom );
				bin = cvRound( n * ( ori + CV_PI ) / PI2 );
				bin = ( bin < n )? bin : 0;
				hist[bin] += w * mag;
			}

	return hist;
}



/*
Calculates the gradient magnitude and orientation at a given pixel.

@param img image
@param r pixel row
@param c pixel col
@param mag output as gradient magnitude at pixel (r,c)
@param ori output as gradient orientation at pixel (r,c)

@return Returns 1 if the specified pixel is a valid one and sets mag and
	ori accordingly; otherwise returns 0
*/
int calc_grad_mag_ori( IplImage* img, int r, int c, double* mag, double* ori )
{
	double dx, dy;

	if( r > 0  &&  r < img->height - 1  &&  c > 0  &&  c < img->width - 1 )
	{
		dx = pixval32f( img, r, c+1 ) - pixval32f( img, r, c-1 );
		dy = pixval32f( img, r-1, c ) - pixval32f( img, r+1, c );
		*mag = sqrt( dx*dx + dy*dy );
		*ori = atan2( dy, dx );
		return 1;
	}

	else
		return 0;
}



/*
Gaussian smooths an orientation histogram.

@param hist an orientation histogram
@param n number of bins
*/
void smooth_ori_hist( double* hist, int n )
{
	double prev, tmp, h0 = hist[0];
	int i;

	prev = hist[n-1];
	for( i = 0; i < n; i++ )
	{
		tmp = hist[i];
		hist[i] = 0.25 * prev + 0.5 * hist[i] + 
			0.25 * ( ( i+1 == n )? h0 : hist[i+1] );
		prev = tmp;
	}
}



/*
Finds the magnitude of the dominant orientation in a histogram

@param hist an orientation histogram
@param n number of bins

@return Returns the value of the largest bin in hist
*/
double dominant_ori( double* hist, int n )
{
	double omax;
	int maxbin, i;

	omax = hist[0];
	maxbin = 0;
	for( i = 1; i < n; i++ )
		if( hist[i] > omax )
		{
			omax = hist[i];
			maxbin = i;
		}
	return omax;
}



/*
Interpolates a histogram peak from left, center, and right values
*/
#define interp_hist_peak( l, c, r ) ( 0.5 * ((l)-(r)) / ((l) - 2.0*(c) + (r)) )



/*
Adds features to an array for every orientation in a histogram greater than
a specified threshold.

@param features new features are added to the end of this array
@param hist orientation histogram
@param n number of bins in hist
@param mag_thr new features are added for entries in hist greater than this
@param feat new features are clones of this with different orientations
*/
void add_good_ori_features( CvSeq* features, double* hist, int n,
						   double mag_thr, struct feature* feat )
{
	struct feature* new_feat;
	double bin, PI2 = CV_PI * 2.0;
	int l, r, i;

	for( i = 0; i < n; i++ )
	{
		l = ( i == 0 )? n - 1 : i-1;
		r = ( i + 1 ) % n;

		if( hist[i] > hist[l]  &&  hist[i] > hist[r]  &&  hist[i] >= mag_thr )
		{
			bin = i + interp_hist_peak( hist[l], hist[i], hist[r] );
			bin = ( bin < 0 )? n + bin : ( bin >= n )? bin - n : bin;
			new_feat = clone_feature( feat );
			new_feat->ori = ( ( PI2 * bin ) / n ) - CV_PI;
			cvSeqPush( features, new_feat );
			free( new_feat );
		}
	}
}



/*
Makes a deep copy of a feature

@param feat feature to be cloned

@return Returns a deep copy of feat
*/
struct feature* clone_feature( struct feature* feat )
{
	struct feature* new_feat;
	struct detection_data* ddata;

	new_feat = new_feature();
	ddata = feat_detection_data( new_feat );
	memcpy( new_feat, feat, sizeof( struct feature ) );
	memcpy( ddata, feat_detection_data(feat), sizeof( struct detection_data ) );
	new_feat->feature_data = ddata;

	return new_feat;
}



/*
Computes feature descriptors for features in an array.  Based on Section 6
of Lowe's paper.

@param features array of features
@param gauss_pyr Gaussian scale space pyramid
@param d width of 2D array of orientation histograms
@param n number of bins per orientation histogram
*/
void compute_descriptors( CvSeq* features, IplImage*** gauss_pyr, int d, int n)
{
	struct feature* feat;
	struct detection_data* ddata;
	double*** hist;
	int i, k = features->total;

	for( i = 0; i < k; i++ )
	{
		feat = CV_GET_SEQ_ELEM( struct feature, features, i );
		ddata = feat_detection_data( feat );
		hist = descr_hist( gauss_pyr[ddata->octv][ddata->intvl], ddata->r,
			ddata->c, feat->ori, ddata->scl_octv, d, n );
		hist_to_descr( hist, d, n, feat );
		release_descr_hist( &hist, d );
	}
}



/*
Computes the 2D array of orientation histograms that form the feature
descriptor.  Based on Section 6.1 of Lowe's paper.

@param img image used in descriptor computation
@param r row coord of center of orientation histogram array
@param c column coord of center of orientation histogram array
@param ori canonical orientation of feature whose descr is being computed
@param scl scale relative to img of feature whose descr is being computed
@param d width of 2d array of orientation histograms
@param n bins per orientation histogram

@return Returns a d x d array of n-bin orientation histograms.
*/
double*** descr_hist( IplImage* img, int r, int c, double ori,
					 double scl, int d, int n )
{
	double*** hist;
	double cos_t, sin_t, hist_width, exp_denom, r_rot, c_rot, grad_mag,
		grad_ori, w, rbin, cbin, obin, bins_per_rad, PI2 = 2.0 * CV_PI;
	int radius, i, j;

	hist = calloc( d, sizeof( double** ) );
	for( i = 0; i < d; i++ )
	{
		hist[i] = calloc( d, sizeof( double* ) );
		for( j = 0; j < d; j++ )
			hist[i][j] = calloc( n, sizeof( double ) );
	}

	cos_t = cos( ori );
	sin_t = sin( ori );
	bins_per_rad = n / PI2;
	exp_denom = d * d * 0.5;
	hist_width = SIFT_DESCR_SCL_FCTR * scl;
	radius = hist_width * sqrt(2) * ( d + 1.0 ) * 0.5 + 0.5;
	for( i = -radius; i <= radius; i++ )
		for( j = -radius; j <= radius; j++ )
		{
			/*
			Calculate sample's histogram array coords rotated relative to ori.
			Subtract 0.5 so samples that fall e.g. in the center of row 1 (i.e.
			r_rot = 1.5) have full weight placed in row 1 after interpolation.
			*/
			c_rot = ( j * cos_t - i * sin_t ) / hist_width;
			r_rot = ( j * sin_t + i * cos_t ) / hist_width;
			rbin = r_rot + d / 2 - 0.5;
			cbin = c_rot + d / 2 - 0.5;

			if( rbin > -1.0  &&  rbin < d  &&  cbin > -1.0  &&  cbin < d )
				if( calc_grad_mag_ori( img, r + i, c + j, &grad_mag, &grad_ori ))
				{
					grad_ori -= ori;
					while( grad_ori < 0.0 )
						grad_ori += PI2;
					while( grad_ori >= PI2 )
						grad_ori -= PI2;

					obin = grad_ori * bins_per_rad;
					w = exp( -(c_rot * c_rot + r_rot * r_rot) / exp_denom );
					interp_hist_entry( hist, rbin, cbin, obin, grad_mag * w, d, n );
				}
		}

	return hist;
}



/*
Interpolates an entry into the array of orientation histograms that form
the feature descriptor.

@param hist 2D array of orientation histograms
@param rbin sub-bin row coordinate of entry
@param cbin sub-bin column coordinate of entry
@param obin sub-bin orientation coordinate of entry
@param mag size of entry
@param d width of 2D array of orientation histograms
@param n number of bins per orientation histogram
*/
void interp_hist_entry( double*** hist, double rbin, double cbin,
					   double obin, double mag, int d, int n )
{
	double d_r, d_c, d_o, v_r, v_c, v_o;
	double** row, * h;
	int r0, c0, o0, rb, cb, ob, r, c, o;

	r0 = cvFloor( rbin );
	c0 = cvFloor( cbin );
	o0 = cvFloor( obin );
	d_r = rbin - r0;
	d_c = cbin - c0;
	d_o = obin - o0;

	/*
	The entry is distributed into up to 8 bins.  Each entry into a bin
	is multiplied by a weight of 1 - d for each dimension, where d is the
	distance from the center value of the bin measured in bin units.
	*/
	for( r = 0; r <= 1; r++ )
	{
		rb = r0 + r;
		if( rb >= 0  &&  rb < d )
		{
			v_r = mag * ( ( r == 0 )? 1.0 - d_r : d_r );
			row = hist[rb];
			for( c = 0; c <= 1; c++ )
			{
				cb = c0 + c;
				if( cb >= 0  &&  cb < d )
				{
					v_c = v_r * ( ( c == 0 )? 1.0 - d_c : d_c );
					h = row[cb];
					for( o = 0; o <= 1; o++ )
					{
						ob = ( o0 + o ) % n;
						v_o = v_c * ( ( o == 0 )? 1.0 - d_o : d_o );
						h[ob] += v_o;
					}
				}
			}
		}
	}
}



/*
Converts the 2D array of orientation histograms into a feature's descriptor
vector.

@param hist 2D array of orientation histograms
@param d width of hist
@param n bins per histogram
@param feat feature into which to store descriptor
*/
void hist_to_descr( double*** hist, int d, int n, struct feature* feat )
{
	int int_val, i, r, c, o, k = 0;

	for( r = 0; r < d; r++ )
		for( c = 0; c < d; c++ )
			for( o = 0; o < n; o++ )
				feat->descr[k++] = hist[r][c][o];

	feat->d = k;
	normalize_descr( feat );
	for( i = 0; i < k; i++ )
		if( feat->descr[i] > SIFT_DESCR_MAG_THR )
			feat->descr[i] = SIFT_DESCR_MAG_THR;
	normalize_descr( feat );

	/* convert floating-point descriptor to integer valued descriptor */
	for( i = 0; i < k; i++ )
	{
		int_val = SIFT_INT_DESCR_FCTR * feat->descr[i];
		feat->descr[i] = MIN( 255, int_val );
	}
}



/*
Normalizes a feature's descriptor vector to unitl length

@param feat feature
*/
void normalize_descr( struct feature* feat )
{
	double cur, len_inv, len_sq = 0.0;
	int i, d = feat->d;

	for( i = 0; i < d; i++ )
	{
		cur = feat->descr[i];
		len_sq += cur*cur;
	}
	len_inv = 1.0 / sqrt( len_sq );
	for( i = 0; i < d; i++ )
		feat->descr[i] *= len_inv;
}



/*
Compares features for a decreasing-scale ordering.  Intended for use with
CvSeqSort

@param feat1 first feature
@param feat2 second feature
@param param unused

@return Returns 1 if feat1's scale is greater than feat2's, -1 if vice versa,
and 0 if their scales are equal
*/
int feature_cmp( void* feat1, void* feat2, void* param )
{
	struct feature* f1 = (struct feature*) feat1;
	struct feature* f2 = (struct feature*) feat2;

	if( f1->scl < f2->scl )
		return 1;
	if( f1->scl > f2->scl )
		return -1;
	return 0;
}



/*
De-allocates memory held by a descriptor histogram

@param hist pointer to a 2D array of orientation histograms
@param d width of hist
*/
void release_descr_hist( double**** hist, int d )
{
	int i, j;

	for( i = 0; i < d; i++)
	{
		for( j = 0; j < d; j++ )
			free( (*hist)[i][j] );
		free( (*hist)[i] );
	}
	free( *hist );
	*hist = NULL;
}


/*
De-allocates memory held by a scale space pyramid

@param pyr scale space pyramid
@param octvs number of octaves of scale space
@param n number of images per octave
*/
void release_pyr( IplImage**** pyr, int octvs, int n )
{
	int i, j;
	for( i = 0; i < octvs; i++ )
	{
		for( j = 0; j < n; j++ )
			cvReleaseImage( &(*pyr)[i][j] );
		free( (*pyr)[i] );
	}
	free( *pyr );
	*pyr = NULL;
}

/*
Replaces a file's extension, which is assumed to be everything after the
last dot ('.') character.

@param file the name of a file

@param extn a new extension for \a file; should not include a dot (i.e.
	\c "jpg", not \c ".jpg") unless the new file extension should contain
	two dots.

@return Returns a new string formed as described above.  If \a file does
	not have an extension, this function simply adds one.
*/
char* replace_extension( const char* file, const char* extn )
{
	char* new_file, * lastdot;

	new_file = calloc( strlen( file ) + strlen( extn ) + 2,  sizeof( char ) );
	strcpy( new_file, file );
	lastdot = strrchr( new_file, '.' );
	if( lastdot )
		*(lastdot + 1) = '\0';
	else
		strcat( new_file, "." );
	strcat( new_file, extn );

	return new_file;
}



/*
A function that removes the path from a filename.  Similar to the Unix
basename command.

@param pathname a (full) path name

@return Returns the basename of \a pathname.
*/
char* basename( const char* pathname )
{
	char* base, * last_slash;

	last_slash = strrchr( pathname, '/' );
	if( ! last_slash )
	{
		base = calloc( strlen( pathname ) + 1, sizeof( char ) );
		strcpy( base, pathname );
	}
	else
	{
		base = calloc( strlen( last_slash++ ), sizeof( char ) );
		strcpy( base, last_slash );
	}

	return base;
}



/*
Displays progress in the console with a spinning pinwheel.  Every time this
function is called, the state of the pinwheel is incremented.  The pinwheel
has four states that loop indefinitely: '|', '/', '-', '\'.

@param done if 0, this function simply increments the state of the pinwheel;
	otherwise it prints "done"
*/
void progress( int done )
{
	char state[4] = { '|', '/', '-', '\\' };
	static int cur = -1;

	if( cur == -1 )
		fprintf( stderr, "  " );

	if( done )
	{
		fprintf( stderr, "\b\bdone\n");
		cur = -1;
	}
	else
	{
		cur = ( cur + 1 ) % 4;
		fprintf( stdout, "\b\b%c ", state[cur] );
		fflush(stderr);
	}
}



/*
Erases a specified number of characters from a stream.

@param stream the stream from which to erase characters
@param n the number of characters to erase
*/
void erase_from_stream( FILE* stream, int n )
{
	int j;
	for( j = 0; j < n; j++ )
		fprintf( stream, "\b" );
	for( j = 0; j < n; j++ )
		fprintf( stream, " " );
	for( j = 0; j < n; j++ )
		fprintf( stream, "\b" );
}



/*
Doubles the size of an array with error checking

@param array pointer to an array whose size is to be doubled
@param n number of elements allocated for \a array
@param size size in bytes of elements in \a array

@return Returns the new number of elements allocated for \a array.  If no
	memory is available, returns 0 and frees array.
*/
int array_double( void** array, int n, int size )
{
	void* tmp;

	tmp = realloc( *array, 2 * n * size );
	if( ! tmp )
	{
		fprintf( stderr, "Warning: unable to allocate memory in array_double(),"
				" %s line %d\n", __FILE__, __LINE__ );
		if( *array )
			free( *array );
		*array = NULL;
		return 0;
	}
	*array = tmp;
	return n*2;
}



/*
Calculates the squared distance between two points.

@param p1 a point
@param p2 another point
*/
double dist_sq_2D( CvPoint2D64f p1, CvPoint2D64f p2 )
{
	double x_diff = p1.x - p2.x;
	double y_diff = p1.y - p2.y;

	return x_diff * x_diff + y_diff * y_diff;
}



/*
Draws an x on an image.

@param img an image
@param pt the center point of the x
@param r the x's radius
@param w the x's line weight
@param color the color of the x
*/
void draw_x( IplImage* img, CvPoint pt, int r, int w, CvScalar color )
{
	cvLine( img, pt, cvPoint( pt.x + r, pt.y + r), color, w, 8, 0 );
	cvLine( img, pt, cvPoint( pt.x - r, pt.y + r), color, w, 8, 0 );
	cvLine( img, pt, cvPoint( pt.x + r, pt.y - r), color, w, 8, 0 );
	cvLine( img, pt, cvPoint( pt.x - r, pt.y - r), color, w, 8, 0 );
}



/*
Combines two images by scacking one on top of the other

@param img1 top image
@param img2 bottom image

@return Returns the image resulting from stacking \a img1 on top if \a img2
*/
extern IplImage* stack_imgs( IplImage* img1, IplImage* img2 )
{
	IplImage* stacked = cvCreateImage( cvSize( MAX(img1->width, img2->width),
										img1->height + img2->height ),
										IPL_DEPTH_8U, 3 );

	cvZero( stacked );
	cvSetImageROI( stacked, cvRect( 0, 0, img1->width, img1->height ) );
	cvAdd( img1, stacked, stacked, NULL );
	cvSetImageROI( stacked, cvRect(0, img1->height, img2->width, img2->height) );
	cvAdd( img2, stacked, stacked, NULL );
	cvResetImageROI( stacked );

	return stacked;
}

/*
@param filename location of a file containing image features
@param type determines how features are input.  If \a type is FEATURE_OXFD,
	the input file is treated as if it is from the code provided by the VGG
	at Oxford:

	http://www.robots.ox.ac.uk:5000/~vgg/research/affine/index.html

	If \a type is FEATURE_LOWE, the input file is treated as if it is from
	David Lowe's SIFT code:

	http://www.cs.ubc.ca/~lowe/keypoints  
@param features pointer to an array in which to store features

@return Returns the number of features imported from filename or -1 on error
*/

int import_features( char* filename, int type, struct feature** feat )
{
	int n;

	switch( type )
	{
	case FEATURE_OXFD:
		n = import_oxfd_features( filename, feat );
		break;
	case FEATURE_LOWE:
		n = import_lowe_features( filename, feat );
		break;
	default:
		fprintf( stderr, "Warning: import_features(): unrecognized feature" \
				"type, %s, line %d\n", __FILE__, __LINE__ );
		return -1;
	}

	if( n == -1 )
		fprintf( stderr, "Warning: unable to import features from %s,"	\
			" %s, line %d\n", filename, __FILE__, __LINE__ );
	return n;
}



/*
Exports a feature set to a file formatted depending on the type of
features, as specified in the feature struct's type field.

@param filename name of file to which to export features
@param feat feature array
@param n number of features 

@return Returns 0 on success or 1 on error
*/
int export_features( char* filename, struct feature* feat, int n )
{
	int r, type;

	if( n <= 0  ||  ! feat )
	{
		fprintf( stderr, "Warning: no features to export, %s line %d\n",
				__FILE__, __LINE__ );
		return 1;
	}
	type = feat[0].type;
	switch( type )
	{
	case FEATURE_OXFD:
		r = export_oxfd_features( filename, feat, n );
		break;
	case FEATURE_LOWE:
		r = export_lowe_features( filename, feat, n );
		break;
	default:
		fprintf( stderr, "Warning: export_features(): unrecognized feature" \
				"type, %s, line %d\n", __FILE__, __LINE__ );
		return -1;
	}

	if( r )
		fprintf( stderr, "Warning: unable to export features to %s,"	\
				" %s, line %d\n", filename, __FILE__, __LINE__ );
	return r;
}


/*
Draws a set of features on an image

@param img image on which to draw features
@param feat array of Oxford-type features
@param n number of features
*/
void draw_features( IplImage* img, struct feature* feat, int n )
{
	int type;

	if( n <= 0  ||  ! feat )
	{
		fprintf( stderr, "Warning: no features to draw, %s line %d\n",
				__FILE__, __LINE__ );
		return;
	}
	type = feat[0].type;
	switch( type )
	{
	case FEATURE_OXFD:
		draw_oxfd_features( img, feat, n );
		break;
	case FEATURE_LOWE:
		draw_lowe_features( img, feat, n );
		break;
	default:
		fprintf( stderr, "Warning: draw_features(): unrecognized feature" \
			" type, %s, line %d\n", __FILE__, __LINE__ );
		break;
	}
}



/*
Calculates the squared Euclidian distance between two feature descriptors.

@param f1 first feature
@param f2 second feature

@return Returns the squared Euclidian distance between the descriptors of
f1 and f2.
*/
double descr_dist_sq( struct feature* f1, struct feature* f2 )
{
	double diff, dsq = 0;
	double* descr1, * descr2;
	int i, d;

	d = f1->d;
	if( f2->d != d )
		return DBL_MAX;
	descr1 = f1->descr;
	descr2 = f2->descr;

	for( i = 0; i < d; i++ )
	{
		diff = descr1[i] - descr2[i];
		dsq += diff*diff;
	}
	return dsq;
}



/***************************** Local Functions *******************************/


/*
Reads image features from file.  The file should be formatted as from
the code provided by the Visual Geometry Group at Oxford:

http://www.robots.ox.ac.uk:5000/~vgg/research/affine/index.html

@param filename location of a file containing image features
@param features pointer to an array in which to store features

@return Returns the number of features imported from filename or -1 on error
*/
int import_oxfd_features( char* filename, struct feature** features )
{
	struct feature* f;
	int i, j, n, d;
	double x, y, a, b, c, dv;
	FILE* file;

	if( ! ( file = fopen( filename, "r" ) ) )
	{
		fprintf( stderr, "Warning: error opening %s, %s, line %d\n",
				filename, __FILE__, __LINE__ );
		return -1;
	}

	/* read dimension and number of features */
	if( fscanf( file, " %d %d ", &d, &n ) != 2 )
	{
		fprintf( stderr, "Warning: file read error, %s, line %d\n",
				__FILE__, __LINE__ );
		return -1;
	}
	if( d > FEATURE_MAX_D )
	{
		fprintf( stderr, "Warning: descriptor too long, %s, line %d\n",
				__FILE__, __LINE__ );
		return -1;
	}


	f = calloc( n, sizeof(struct feature) );
	for( i = 0; i < n; i++ )
	{
		/* read affine region parameters */
		if( fscanf( file, " %lf %lf %lf %lf %lf ", &x, &y, &a, &b, &c ) != 5 )
		{
			fprintf( stderr, "Warning: error reading feature #%d, %s, line %d\n",
					i+1, __FILE__, __LINE__ );
			free( f );
			return -1;
		}
		f[i].img_pt.x = f[i].x = x;
		f[i].img_pt.y = f[i].y = y;
		f[i].a = a;
		f[i].b = b;
		f[i].c = c;
		f[i].d = d;
		f[i].type = FEATURE_OXFD;

		/* read descriptor */
		for( j = 0; j < d; j++ )
		{
			if( ! fscanf( file, " %lf ", &dv ) )
			{
				fprintf( stderr, "Warning: error reading feature descriptor" \
						" #%d, %s, line %d\n", i+1, __FILE__, __LINE__ );
				free( f );
				return -1;
			}
			f[i].descr[j] = dv;
		}

		f[i].scl = f[i].ori = 0;
		f[i].category = 0;
		f[i].fwd_match = f[i].bck_match = f[i].mdl_match = NULL;
		f[i].mdl_pt.x = f[i].mdl_pt.y = -1;
		f[i].feature_data = NULL;
	}

	if( fclose(file) )
	{
		fprintf( stderr, "Warning: file close error, %s, line %d\n",
				__FILE__, __LINE__ );
		free( f );
		return -1;
	}

	*features = f;
	return n;
}




/*
Exports a feature set to a file formatted as one from the code provided
by the Visual Geometry Group at Oxford:

http://www.robots.ox.ac.uk:5000/~vgg/research/affine/index.html

@param filename name of file to which to export features
@param feat feature array
@param n number of features

@return Returns 0 on success or 1 on error
*/
int export_oxfd_features( char* filename, struct feature* feat, int n )
{
	FILE* file;
	int i, j, d;

	if( n <= 0 )
	{
		fprintf( stderr, "Warning: feature count %d, %s, line %s\n",
				n, __FILE__, __LINE__ );
		return 1;
	}
	if( ! ( file = fopen( filename, "w" ) ) )
	{
		fprintf( stderr, "Warning: error opening %s, %s, line %d\n",
				filename, __FILE__, __LINE__ );
		return 1;
	}

	d = feat[0].d;
	fprintf( file, "%d\n%d\n", d, n );
	for( i = 0; i < n; i++ )
	{
		fprintf( file, "%f %f %f %f %f", feat[i].x, feat[i].y, feat[i].a,
				feat[i].b, feat[i].c );
		for( j = 0; j < d; j++ )
			fprintf( file, " %f", feat[i].descr[j] );
		fprintf( file, "\n" );
	}

	if( fclose(file) )
	{
		fprintf( stderr, "Warning: file close error, %s, line %d\n",
				__FILE__, __LINE__ );
		return 1;
	}

	return 0;
}



/*
Draws Oxford-type affine features

@param img image on which to draw features
@param feat array of Oxford-type features
@param n number of features
*/
void draw_oxfd_features( IplImage* img, struct feature* feat, int n )
{
	CvScalar color = CV_RGB( 255, 255, 255 );
	int i;

	if( img-> nChannels > 1 )
		color = FEATURE_OXFD_COLOR;
	for( i = 0; i < n; i++ )
		draw_oxfd_feature( img, feat + i, color );
}



/*
Draws a single Oxford-type feature

@param img image on which to draw
@param feat feature to be drawn
@param color color in which to draw
*/
void draw_oxfd_feature( IplImage* img, struct feature* feat, CvScalar color )
{
	double m[4] = { feat->a, feat->b, feat->b, feat->c };
	double v[4] = { 0 };
	double e[2] = { 0 };
	CvMat M, V, E;
	double alpha, l1, l2;

	/* compute axes and orientation of ellipse surrounding affine region */
	cvInitMatHeader( &M, 2, 2, CV_64FC1, m, CV_AUTOSTEP );
	cvInitMatHeader( &V, 2, 2, CV_64FC1, v, CV_AUTOSTEP );
	cvInitMatHeader( &E, 2, 1, CV_64FC1, e, CV_AUTOSTEP );
	cvEigenVV( &M, &V, &E, DBL_EPSILON );
	l1 = 1 / sqrt( e[1] );
	l2 = 1 / sqrt( e[0] );
	alpha = -atan2( v[1], v[0] );
	alpha *= 180 / CV_PI;

	cvEllipse( img, cvPoint( feat->x, feat->y ), cvSize( l2, l1 ), alpha,
				0, 360, CV_RGB(0,0,0), 3, 8, 0 );
	cvEllipse( img, cvPoint( feat->x, feat->y ), cvSize( l2, l1 ), alpha,
				0, 360, color, 1, 8, 0 );
	cvLine( img, cvPoint( feat->x+2, feat->y ), cvPoint( feat->x-2, feat->y ),
			color, 1, 8, 0 );
	cvLine( img, cvPoint( feat->x, feat->y+2 ), cvPoint( feat->x, feat->y-2 ),
			color, 1, 8, 0 );
}



/*
Reads image features from file.  The file should be formatted as from
the code provided by David Lowe:

http://www.cs.ubc.ca/~lowe/keypoints/

@param filename location of a file containing image features
@param features pointer to an array in which to store features

@return Returns the number of features imported from filename or -1 on error
*/
int import_lowe_features( char* filename, struct feature** features )
{
	struct feature* f;
	int i, j, n, d;
	double x, y, s, o, dv;
	FILE* file;
	

	if( ! ( file = fopen( filename, "r" ) ) )
	{
		fprintf( stderr, "Warning: error opening %s, %s, line %d\n",
			filename, __FILE__, __LINE__ );
		return -1;
	}

	/* read number of features and dimension */
	if( fscanf( file, " %d %d ", &n, &d ) != 2 )
	{
		fprintf( stderr, "Warning: file read error, %s, line %d\n",
				__FILE__, __LINE__ );
		return -1;
	}
	if( d > FEATURE_MAX_D )
	{
		fprintf( stderr, "Warning: descriptor too long, %s, line %d\n",
				__FILE__, __LINE__ );
		return -1;
	}

	f = calloc( n, sizeof(struct feature) );
	for( i = 0; i < n; i++ )
	{
		/* read affine region parameters */
		if( fscanf( file, " %lf %lf %lf %lf ", &y, &x, &s, &o ) != 4 )
		{
			fprintf( stderr, "Warning: error reading feature #%d, %s, line %d\n",
					i+1, __FILE__, __LINE__ );
			free( f );
			return -1;
		}
		f[i].img_pt.x = f[i].x = x;
		f[i].img_pt.y = f[i].y = y;
		f[i].scl = s;
		f[i].ori = o;
		f[i].d = d;
		f[i].type = FEATURE_LOWE;

		/* read descriptor */
		for( j = 0; j < d; j++ )
		{
			if( ! fscanf( file, " %lf ", &dv ) )
			{
				fprintf( stderr, "Warning: error reading feature descriptor" \
						" #%d, %s, line %d\n", i+1, __FILE__, __LINE__ );
				free( f );
				return -1;
			}
			f[i].descr[j] = dv;
		}

		f[i].a = f[i].b = f[i].c = 0;
		f[i].category = 0;
		f[i].fwd_match = f[i].bck_match = f[i].mdl_match = NULL;
		f[i].mdl_pt.x = f[i].mdl_pt.y = -1;
	}

	if( fclose(file) )
	{
		fprintf( stderr, "Warning: file close error, %s, line %d\n",
				__FILE__, __LINE__ );
		free( f );
		return -1;
	}

	*features = f;
	return n;
}



/*
Exports a feature set to a file formatted as one from the code provided
by David Lowe:

http://www.cs.ubc.ca/~lowe/keypoints/

@param filename name of file to which to export features
@param feat feature array
@param n number of features

@return Returns 0 on success or 1 on error
*/
int export_lowe_features( char* filename, struct feature* feat, int n )
{
	FILE* file;
	int i, j, d;

	if( n <= 0 )
	{
		fprintf( stderr, "Warning: feature count %d, %s, line %s\n",
				n, __FILE__, __LINE__ );
		return 1;
	}
	if( ! ( file = fopen( filename, "w" ) ) )
	{
		fprintf( stderr, "Warning: error opening %s, %s, line %d\n",
				filename, __FILE__, __LINE__ );
		return 1;
	}

	d = feat[0].d;
	fprintf( file, "%d %d\n", n, d );
	for( i = 0; i < n; i++ )
	{
		fprintf( file, "%f %f %f %f", feat[i].y, feat[i].x,
				feat[i].scl, feat[i].ori );
		for( j = 0; j < d; j++ )
		{
			/* write 20 descriptor values per line */
			if( j % 20 == 0 )
				fprintf( file, "\n" );
			fprintf( file, " %d", (int)(feat[i].descr[j]) );
		}
		fprintf( file, "\n" );
	}

	if( fclose(file) )
	{
		fprintf( stderr, "Warning: file close error, %s, line %d\n",
				__FILE__, __LINE__ );
		return 1;
	}

	return 0;
}


/*
Draws Lowe-type features

@param img image on which to draw features
@param feat array of Oxford-type features
@param n number of features
*/
void draw_lowe_features( IplImage* img, struct feature* feat, int n )
{
	CvScalar color = CV_RGB( 255, 255, 255 );
	int i;

	if( img-> nChannels > 1 )
		color = FEATURE_LOWE_COLOR;
	for( i = 0; i < n; i++ )
		draw_lowe_feature( img, feat + i, color );
}



/*
Draws a single Lowe-type feature

@param img image on which to draw
@param feat feature to be drawn
@param color color in which to draw
*/
void draw_lowe_feature( IplImage* img, struct feature* feat, CvScalar color )
{
	int len, hlen, blen, start_x, start_y, end_x, end_y, h1_x, h1_y, h2_x, h2_y;
	double scl, ori;
	double scale = 5.0;
	double hscale = 0.75;
	CvPoint start, end, h1, h2;

	/* compute points for an arrow scaled and rotated by feat's scl and ori */
	start_x = cvRound( feat->x );
	start_y = cvRound( feat->y );
	scl = feat->scl;
	ori = feat->ori;
	len = cvRound( scl * scale );
	hlen = cvRound( scl * hscale );
	blen = len - hlen;
	end_x = cvRound( len *  cos( ori ) ) + start_x;
	end_y = cvRound( len * -sin( ori ) ) + start_y;
	h1_x = cvRound( blen *  cos( ori + CV_PI / 18.0 ) ) + start_x;
	h1_y = cvRound( blen * -sin( ori + CV_PI / 18.0 ) ) + start_y;
	h2_x = cvRound( blen *  cos( ori - CV_PI / 18.0 ) ) + start_x;
	h2_y = cvRound( blen * -sin( ori - CV_PI / 18.0 ) ) + start_y;
	start = cvPoint( start_x, start_y );
	end = cvPoint( end_x, end_y );
	h1 = cvPoint( h1_x, h1_y );
	h2 = cvPoint( h2_x, h2_y );

	cvLine( img, start, end, color, 1, 8, 0 );
	cvLine( img, end, h1, color, 1, 8, 0 );
	cvLine( img, end, h2, color, 1, 8, 0 );
}

