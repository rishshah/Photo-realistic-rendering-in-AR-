ORB_INC= -I./ORB_SLAM2/ -I./ORB_SLAM2/include/ 
PANG_INC= -I./Pangolin/include/ -I./Pangolin/ -I./Pangolin/build/src/include/ 
EIGEN_INC= -I./ORB_SLAM2/Thirdparty/eigen-eigen-b9cd8366d4e8/

OPENGL_LIB= -lGL -lGLEW -lglfw
OPENCV_LIB=`pkg-config opencv --cflags --libs`
ORB_LIB=-L./ORB_SLAM2/lib/ -lORB_SLAM2
PANG_LIB=-L./Pangolin/build/src/ -lpangolin

INCS=$(ORB_INC) $(PANG_INC) $(EIGEN_INC)
LIBS= $(OPENGL_LIB) $(ORB_LIB) $(PANG_LIB) $(OPENCV_LIB)

VOCABULARY=ORB_SLAM2/Vocabulary/ORBvoc.txt
CAM_SETTING=ORB_SLAM2/Examples/Monocular/EuRoC.yaml

BASE_DIR_PATH=/home/rishshah/Downloads/Sem6/Study_Material/RnD/Photo-realistic-rendering-in-AR-
PANG_LIB_PATH=$(BASE_DIR_PATH)/Pangolin/build/src/
ORB_LIB_PATH=$(BASE_DIR_PATH)/ORB_SLAM2/lib/
DEFAULT_DL_PATH=/usr/local/lib/


api: slam_api.cpp slam
	g++ $(INCS) -Wall -std=c++11 -o test slam_api.cpp  $(LIBS)
slam: pangolin	
	cd ORB_SLAM2; ./build.sh
pangolin:
	cd Pangolin;mkdir build;cd build;cmake ..;cmake --build .;


run: slam_api.cpp
	@echo "If it doesn't work run this command\n"
	@echo "LD_LIBRARY_PATH=$(DEFAULT_DL_PATH):$(PANG_LIB_PATH):$(ORB_LIB_PATH); export LD_LIBRARY_PATH\n"
	./test  $(VOCABULARY) $(CAM_SETTING) V1_01_easy/mav0/cam0/data/ V1_01_easy/mav0/cam0/data.csv 	

clean:
	rm -rf ORB_SLAM2/build/ ORB_SLAM2/lib/  
	rm -rf ORB_SLAM2/ThirdParty/DBoW2/build/ ORB_SLAM2/ThirdParty/DBoW2/lib/  
	rm -rf ORB_SLAM2/ThirdParty/g2o/build/ ORB_SLAM2/ThirdParty/g2o/lib/
	rm -rf Pangolin/build/
	rm -rf test