orbslam2: pangolin	
	cd ORB_SLAM2; ./build.sh
pangolin:
	cd Pangolin;mkdir build;cd build;cmake ..;cmake --build .;

clean:
	rm -rf ORB_SLAM2/build/ ORB_SLAM2/lib/  
	rm -rf ORB_SLAM2/ThirdParty/DBoW2/build/ ORB_SLAM2/ThirdParty/DBoW2/lib/  
	rm -rf ORB_SLAM2/ThirdParty/g2o/build/ ORB_SLAM2/ThirdParty/g2o/lib/
	rm -rf Pangolin/build/
