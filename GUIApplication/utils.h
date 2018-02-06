#ifndef UTILS_H
#define UTILS_H

#define BASE_DIR "/home/rishshah/Downloads/Sem6/Study_Material/RnD/Photo-realistic-rendering-in-AR-/"
#define PANGOLIN_LIB BASE_DIR "Pangolin/build/src/"
#define ORB_SLAM_LIB BASE_DIR "ORB_SLAM2/lib/"
#define THIRDPARTY1_LIB BASE_DIR "Thirdparty/DBoW2/lib/"
#define THIRDPARTY2_LIB BASE_DIR "Thirdparty/g2o/lib/"
#define LD_LIBRARY_PATH "/usr/local/lib/:" BASE_DIR ":" ORB_SLAM_LIB ":" PANGOLIN_LIB ":" THIRDPARTY1_LIB ":" THIRDPARTY2_LIB ";"

#define VOCABULARY BASE_DIR "ORB_SLAM2/Vocabulary/ORBvoc.txt"
#define CAM_SETTING BASE_DIR "ORB_SLAM2/Examples/Monocular/EuRoC.yaml"

#define SLAM_POINTS_FILEPATH BASE_DIR "abc.txt"
#define SLAM_KFS_FILEPATH BASE_DIR "KeyFrameTrajectory.txt"

#define START_X 10
#define START_Y 10
#define SIZE_X 460
#define SIZE_Y 460

#endif // UTILS_H
