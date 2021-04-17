---
layout: post
title:  "Project Details"
date:   2017-12-08 13:43:00 -0800
---

For my Graphics 471 final project, I used texture mapping to generate a 3d model of the city of San Luis Obispo, from a heightmap image and a satellite photo of SLO.

![image801][image702]

The source code for the project can be downloaded [here.][github_home]

Controls for the model are as follows:

W ==> Move forward\\
A ==> Move left\\
S ==> Move down\\
D ==> Move right\\
Space ==> Move up\\
Shift ==> Move down

P ==> Move quickly\\
G ==> Toggle gravity


I began with a flat mesh made of `(sideLength^2) * 2` triangles, where `sideLength` can be configured in the `main.cpp` source file. A larger value for `sideLength` increases the granularity of the map. The mesh is drawn using `GL_TRIANGLE_STRIP`.
![image101][image101]

I downloaded the heightmap file from the website [here][tparty]. The heightmap is a square greyscale image; lighter pixel values represent higher relative elevations. The image is centered on Cerro San Luis Obispo, AKA "Mount Madonna".
![image201][image201]

I began by texturing the heightmap directly onto the mesh, as an image.
![image202][image202]

I generated terrain elevations using the vertex shader, by mutating the y-coordinates of different vertices proportional to the pixel value at the nearest pixel on the heightmap.
![image301][image301]

At a significantly higher granularity, the details of the map are revealed.
![image401][image401]

The first pass of the terrain map came from a single screenshot of the area on Google Maps. I trimmed the image so that it covered exactly the same area as the heightmap.
![image501][image501]

In the fragment shader, I told the mesh to color each point according to the color of the nearest pixel in the terrain map.
![image502][image502]

Up close, the detail is revealed to be rather blurry...
![image601][image601]

The second pass of the terrain map came from several closer screenshots of the area, all stitched together to create a much higher-resolution image of the same area.
![image701][image701]

This one textures much nicer.
![image801][image702]

Still blurry on closer inspection, but not as bad. Also, the color palette of this texture seems to better reflect the normal state of the terrain.
![image802][image801]

[image101]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/101.png?raw=true
[image201]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/201.jpg?raw=true
[image202]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/202.png?raw=true
[image301]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/301.png?raw=true
[image401]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/401.png?raw=true
[image501]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/501.jpg?raw=true
[image502]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/502.png?raw=true
[image601]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/601.png?raw=true
[image701]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/701.jpg?raw=true
[image702]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/702.png?raw=true
[image801]: https://github.com/calpoly-csc471-fall-17/final-project-website-BMcGuffin/blob/master/docs/_assets/images/801.png?raw=true
[github_home]: https://github.com/calpoly-csc471-fall-17/final-project-BMcGuffin
[tparty]: https://terrain.party/
