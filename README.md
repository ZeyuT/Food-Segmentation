# Food-Segmentation
## Introduction

This is a project that segmented foods in photographs via an active contour algorithm. The project is the semester project of Professor Hoover's course ECE6310
in Clemson University.

[//]: # (Image References)
[image1]: ./examples/Grow1.png
[image2]: ./examples/Grow2.png
[image3]: ./examples/Grow3.png
[image4]: ./examples/Shrink1.png
[image5]: ./examples/Shrink2.png
[image6]: ./examples/Shrink3.png
[image7]: ./examples/Shrink4.png

## Implementation ##
The goal is to implement an active contour algorithm that allows a user to semi-automatically segment food items in a photograph of multiple foods on a dinner plate.
The program must load a color PNM image but only needs to display the greyscale version (average of the three color bands). Users have
the following options: left-click to draw around a food item that subsequently automatically shrinks to wrap to the food boundary; right-click a point within a food item that subsequently
automatically grows a contour to its outer boundary; shift-click (either button) to manually drag a contour point to a new location.

There are several energy terms being considered.Three internal energy terms includes the distance between points, the deviation from the average distance between points, and
the distance to the centroid of the current contour. Three external energy terms includes the image Sobel edge gradient magnitude, the average intensity within the contour, and the
Canny edge gradient magnitude.

Besides, the project files integrates a region grow algorithm that could grow a region from a point in a photograph. However, the region grow algorithm only 
works well on objects with obvious edges.

## Dependencies
The project requires:
* Window 8 SDK
* Visual Studio 2017 or above

## Files
* Emerge.sln: the structure file for VS.
* Emerge.exe: the executable file of the project.
* main.c: the C code of the GUI.
* ActiveContour.c: the C code of the active contour algorithm
* RegionGrow.c: the C code of the region grow algorithm.

## Examples
Here are some examples of food segmentation in growing mode.

![Growing Coutour Examples][image1]

![Growing Coutour Examples][image2]

![Growing Coutour Examples][image3]

Here are some examples of food segmentation in shrinking mode.


![Shrinking Coutour Examples][image4]

![Shrinking Coutour Examples][image5]

![Shrinking Coutour Examples][image6]

![Shrinking Coutour Examples][image7]
