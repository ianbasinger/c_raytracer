# c_raytracer
raytracing in C

this relies on lodepng for .png encoding:
https://github.com/lvandeve/lodepng

this is a test to render a 3D scene using just C, it creates randomly placed spheres (and a floor that is actually just a giant sphere) 

for the scene, the camera and rays loop over 800 x 600 and for every pixel a ray from the camera of Vec3{0,0,0} based on the position.

if the ray hits a sphere, calculate lughting and see if that point is within a shadow (blocked by another sphere) if recflective, show a secondary reflection ray, and reflect base color.

then stores RGB to the image buffer which then goes to lodepng to write to a .png

![example](https://github.com/ianbasinger/c_raytracer/raw/main/example_1.png)
![example](https://github.com/ianbasinger/c_raytracer/raw/main/example_2.png)
