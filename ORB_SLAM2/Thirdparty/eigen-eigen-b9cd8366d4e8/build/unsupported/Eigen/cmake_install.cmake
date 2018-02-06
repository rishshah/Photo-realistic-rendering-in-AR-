# Install script for directory: /home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/eigen3/unsupported/Eigen" TYPE FILE FILES
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/AdolcForward"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/AlignedVector3"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/ArpackSupport"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/AutoDiff"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/BVH"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/FFT"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/IterativeSolvers"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/KroneckerProduct"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/LevenbergMarquardt"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/MatrixFunctions"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/MoreVectorization"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/MPRealSupport"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/NonLinearOptimization"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/NumericalDiff"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/OpenGLSupport"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/Polynomials"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/Skyline"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/SparseExtra"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/unsupported/Eigen/Splines"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/build/unsupported/Eigen/src/cmake_install.cmake")

endif()

