# Install script for directory: /home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/eigen3/Eigen/src/Core" TYPE FILE FILES
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Array.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/ArrayBase.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/ArrayWrapper.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Assign.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Assign_MKL.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/BandMatrix.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Block.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/BooleanRedux.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/CommaInitializer.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/CoreIterators.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/CwiseBinaryOp.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/CwiseNullaryOp.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/CwiseUnaryOp.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/CwiseUnaryView.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/DenseBase.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/DenseCoeffsBase.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/DenseStorage.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Diagonal.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/DiagonalMatrix.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/DiagonalProduct.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Dot.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/EigenBase.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Flagged.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/ForceAlignedAccess.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Functors.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Fuzzy.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/GeneralProduct.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/GenericPacketMath.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/GlobalFunctions.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/IO.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Map.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/MapBase.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/MathFunctions.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Matrix.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/MatrixBase.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/NestByValue.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/NoAlias.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/NumTraits.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/PermutationMatrix.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/PlainObjectBase.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/ProductBase.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Random.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Redux.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Ref.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Replicate.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/ReturnByValue.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Reverse.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Select.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/SelfAdjointView.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/SelfCwiseBinaryOp.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/SolveTriangular.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/StableNorm.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Stride.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Swap.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Transpose.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Transpositions.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/TriangularMatrix.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/VectorBlock.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/VectorwiseOp.h"
    "/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/Eigen/src/Core/Visitor.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/build/Eigen/src/Core/products/cmake_install.cmake")
  include("/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/build/Eigen/src/Core/util/cmake_install.cmake")
  include("/home/rishshah/git/ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/build/Eigen/src/Core/arch/cmake_install.cmake")

endif()

