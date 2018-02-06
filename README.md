# Photo-realistic rendering in AR

## Description
Application which scans the natural environment around user , tracks various planes and features and creates a synthetic map of the environment. User can then provide 3D meshes to display in the scene and play with them.
The photorealistic rendering part includes 3D mesh rendering in global illumination accounting for various light sources in that natural environment.

## Milestones

### Tracking
- Use SLAM system to recreate the surroundings and get 3d points of location of scene and camera trajectory

### Analysis
- Save the points in database system in orderly fashion so as to perform fast retrieval and matching in rendering phase
- Construct approxiamte planes from the 3d points and store them

### Real Time Rendering 
- Render set of good available planes for user to select to place 3d mesh on
- Get user 3d mesh and render it
- Combine real time camera feed with the rendered synthetic image and display it to user

### Photorealistic Rendering
- Real research papers and look for implementation
- ...

## Status
- ORB SLAM library output 3d points obtained
- One Plane fitted to points in the space
- Getting input -> either attached camera/ video file input
- Processing file and displaying the progress window from ORBSLAM
- Displaying processed points in may be 3 different views (Without superimposing it on the image)
- Mouse selection tool for choosing a bunch of points
- Fitting a plane to those points

## Plan

### GUI Application Interface Actions Remaining
- Getting input meshes to place on the plane
- Render it and superimpose on the video and playback


#### Overview
A GUI application for user to visualize augmented reality using script that builds ORBSLAM, processing the output for purpose of mounting a 3d surface on input 2d video for rendering synthetic meshes

- ORB SLAM API 
	- Work 		(C++ program, using api calls of ORBSLAM and getting output)
	- Input		(Images in sequences)
	- Output 	(3D Points of scene)

- QT C++ GUI application that outputs planes from the given points (RANSAC and Regression)












