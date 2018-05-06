# Photo-realistic rendering in AR

## Overview
GUI based application that scans the natural environment around user, creates a synthetic map of the environment in form of sparse point cloud. User can then provide 3D meshes to display in the scene and play with them.

## Major Modules

### Preprocessing
* Use SLAM system to recreate the surroundings and get 3d points of location of scene and camera trajectory
* Get camera pose each each moment of tracking in SLAM system

### User Interaction 
* Construct approxiamte planes from the 3d points, add synthetic mesh and store their location for rendering in real time AR
* Save the feature points in orderly fashion so as to perform fast retrieval and matching in rendering phase

### Rendering 
* Render superposition of saved meshes on the texture background of images that are given as input 
* Perform matching algorithm to do augmentation in realtime


## Future Work
* Parallelize the code for matching algorithm
* Implement photorealistic part of rendering
* Some UI changes to make it look more pleasing
	

## Usage
Once you get the application succesfully compiling and running following are the UI instructions to play with the applciation

### Page 1
Firstly as soon as app starts you should be seeing a dialog box. Then you have to fill in the input and usage mode before proceeding. The "Direct Images" button is for debug purposes currently.
After filling up those radio buttons click on "Continue" 

### Page 2
Upload the required files reuired for the type of mode selected and press on "Continue" 

### Page 3
What happens after in this page is differnt according to mode selected in Page 1

#### Offline
* In Offline mode, SLAM system starts as soon as page 3 is created and ORB-SLAM 2 detects features points and the results are displayed on the screen. After this preprocessing phase, the systems saves the point cloud in "abc.txt" descriptors in "abc.desc" in the build directory.
* Make sure to save the augmentations created before quitting the application or hitting playback
* For webcam mode, all the feed seen during the SLAM duration is saved in build/images directory and accordingly a "image-info.csv" in created in the build directory.  

* Then after the prerocessing stage, you can interact through the UI buttons, keyboards keys and mouse motions to do following actions.
##### Pan Mode
In pan mode you can look around the scene captured point cloud data with apropriate background texture. 

###### Keyboard Events
* "WASDZX" Translation in the scene
* "OP" Traversal along keyframes ```O for previous keyframe and P for next```
* "Shift" Shift to fine translation steps 
* "Alt" Shift to big translation steps

###### Mouse Events
* "Right,Left" Click and Drag -> rotation along 2 global axes in the scene 

##### Select Mode
In select mode you can select points and planes in the point cloud and do following actions. All the actions are executed when you click the confirm button

###### Add Option
Select bunch of points and fit a plane to them to add to the scene

###### Remove Option
Select an already present plane and remove it from the scene

###### Adjust Planes Option
Given atleast 2 planes in the scene, one can be extended to join the other. Select one point on first plane, then select a point on second plane so that first plane can be extended to meet the second

###### Adjust Mesh Option
Given a plane and a mesh, first select plane corner, then select mesh point. Now the mesh will be moved to the plane to make those selected points coincide.

###### Mouse Events
* Selecting a point -> Try to click someplace near that point and drag the mouse to place that point in the rectangular region created by the mouse drag (while keeping mouse button pressed) and when you are sure that the reuired point is in the region release the mouse click

#### Online Mode
For using online mode, you should make sure you have completed the offline mode all the steps including saving of the augmentations.
Now meeting above requirements, in online mode, SLAM system starts and receives either the images/ web cam feed and the applciation tries to match the environment currently visible to the one that was saved in preprocessing phase of latest offline mode. As soon as a match is seen, the augmentations appear at required locations (that you placed it in when in interaction mode in previous offline mode). 









