* Block Detection Annotator

** Overview

This application serves as the preproessing unit for our central pipeline. Our preprocessing consists of finding the centroids of objects in the collaboration space. To detect blocks, we apply a collection of image recognition techniques to an RGB image obtained from a Kinect sensor. The image processing is done in MATLAB and the results can be optionally viewed through an option in the sample application's GUI.

** Dependencies
- Kinect SDK v1.8
  - Confirmed to work with Windows 7+
- MATLAB
  - Confirmed to work with R2017B or greater
- Visual Studio
  - Confirmed to work with VS 2017

** Installation
This repository is a VS project, so opening the solution file (`.sln`) with Visual Studio 2017 should automatically import the project. However, the project won't compile until references are made to the above dependencies. In particular, the following edits need to be made to the project's configuration.

To find these properties, right-click the project and select "Properties". From there, modify the following properties under the "VC++ Directories" heading.
- Include Directories
  - Add the path to the MATLAB include directory. Should be of the form `<MATLAB_DIR>\extern\include`
  - Add the path to the Kinect SDK include directory. Should be of the form `<KINECT_SDK_DIR>\inc`
- Library Directories
  - Add the path to the MATLAB library directory for your system. Should be of the form `<MATLAB_DIR>\extern\lib\win64`. Note that this directory should contain `.lib` files, so if that is not the case continue to drill down into more folders along the `<MATLAB_DIR>\extern\lib` path to find these files(for examplein R2018b, the path is `<MATLAB_DIR>\extern\lib\win64\microsoft`).
  - Add the path to the Kinect SDK library directory. Should be of the form `$(KINECTSDK10_DIR)\lib\amd64`.

After making these modifications, the application should compile. Once compiled, the application will require a Kinect v1 sensor to be plugged into the computer to actually run. Otherwise, running the application will lead to an error and an immediate closure of the GUI.
