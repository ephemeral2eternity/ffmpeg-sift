## FFMPEG-SIFT

#### Extracting SIFT features for key frames when decoding a video in mp4, mpeg, h264 or other video formats. The SIFT features is dumped into a mysql database.
-------------

1. Other open source code used
  * FFMPEG: https://www.ffmpeg.org/
  * SIFT: http://www.cs.ubc.ca/~lowe/keypoints/
    - SIFT: Scale Invariant Feature Transformation (https://en.wikipedia.org/wiki/Scale-invariant_feature_transform)

2. Packages added
  * libcbcd/
  * Database configuration:
    - Edit following lines in libcbcd/sift_main.h
```    
// Add by WCC, cbcd group, 2009-0619
#define def_host_name "localhost"
#define def_user_name "*****"
#define def_password  "*****"
#define def_db_name   "feat_sift"
```

3. Usage:
  * Decoding the video while extracting features: `ffmpeg -i video_name.mpg video_name.yuv`
  * Delete the yuv file: `rm video_name.yuv`
  * Sample bash file in `run`
