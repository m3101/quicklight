/*
Quicklight raycaster-like renderer

Copyright (c) 2020 Amélia O. F. da S.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef QUICKLIGHT
#define QUICKLIGHT
#define QL_PI 3.14159265358979323846
/*Data structures and allocation functions*/

/*
A raster image.
*/
typedef struct _qlraster {
    char* data;/*Raster data. 0-th byte belongs to the top left pixel. (w*s)-th byte belongs to the leftmost pixel of the second line.*/
    int w;/*Width, in pixels*/
    int h;/*Height, in pixels*/
    int s;/*size of each pixel (e.g. 3 for 0-255 RGB pixels)*/
}qlraster;
/*
Instantiates a qlraster object.
One should use the freeqlraster to free its memory (as it allocates memory for storing the image data)
*/
qlraster* Qlraster(int w, int h, int s);
/*Frees a qlraster object*/
void freeqlraster(qlraster** obj);

/*
A vector object.
*/
typedef struct _qlvect {
    double x;/*The point's x coordinate*/
    double y;/*The point's y coordinate*/
    double z;/*The point's z coordinate*/ 
} qlvect;
/*Instantiates a qlvect object*/
qlvect* Qlvect(double x, double y, double z);

/*
A ray object.
*/
typedef struct _qlray {
    qlraster* screen;/*The raster image that'll receive the ray's value when it hits something*/
    int rx;/*The x coordinate of the pixel that'll be updated*/
    int ry;/*The y coordinate of the pixel that'll be updated*/
    qlvect pos;/*A vector that marks the ray's position*/
    qlvect dir;/*A direction vector*/
    double depth;/*Depth at which the ray return black*/
}qlray;
/*Instantiates a qlray object. Vectors will be copied to the ray, not passed by reference.*/
qlray* Qlray(qlraster* screen, int rx, int ry, const qlvect *pos, const qlvect *dir);

/*
A triangle object
*/
typedef struct _qltri {
    qlvect a;/*First vertext*/
    qlvect b;/*Second vertex*/
    qlvect c;/*Third vertex*/
    char colour[3];/*Triangle colour*/
}qltri;
/*Instantiates a qltri object. Vectors will be copied to the triangle, not passed by reference.*/
qltri* Qltri(const qlvect *a,const qlvect *b,const qlvect *c);

/*
A camera object.
One should free its memory with freeqlcamera, as it allocates many ray objects which might not be freed automatically.
*/
typedef struct _qlcamera{
    qlraster *image;/*Image object*/
    qlray** rays;/*qlray objects associated with this camera*/
    qlvect pos;/*Camera position*/
    qlvect dir;/*Camera orientation*/
    double roll;/*Camera roll angle (extra orientation component)*/
    double fl;/*"Focal length" of the camera. Affects the ray angles*/
    double w;/*Camera width in "real-world" units (same units as the vectors)*/
    double h;/*Camera height in "real-world" units (same units as the vectors)*/
    double depth;/*Depth at which rays respawn*/
} qlcamera;
/*
Generates a qlcamera object from a qlraster object and parameters.
Most parameters come from qlcamera properties.
Vectors will be copied, not stored by reference, so you can reuse them without affecting the camera.
raystep dictates how long each ray will move for each step
*/
qlcamera *Qlcamera(qlraster *image,const qlvect *pos,const qlvect *dir,const double roll,const double fl,const double w,const double h,const double depth);
/*
Updates a camera's rays to its current parameters and position
rnd defines the amplitude of the random starting length of the rays
(rnd=0 initializes the ray with length 0, that is, at the camera)
*/
void qlupdatecamera(qlcamera *camera);
/*Frees a qlcamera object*/
void freeqlcamera(qlcamera **camera);

/*Vector functions*/

/*
Scalar product (takes two vector as input and outputs their internal product (sum of products of coordinates))
*/
double qlscproduct(const qlvect *a,const qlvect *b);
/*
3d-Vectorial product. Outputs a vector perpendicular to both input vectors at c.
C = A x B
qlvects A,B,C;
qlscproduct(&A,&B,&C);
A=(a,b,c), b=(d,e,f)
# # | i  j  k |
p = | a  b  c | = (bf-ce,cd-af,ae-bd)
# # | d  e  f |
*/
void qlvectproduct(const qlvect *a,const qlvect *b,qlvect *c);
/*
Multiply a vector by a scalar. B=s*A
*/
void qlvectscale(const qlvect *a,double s,qlvect *b);
/*
Sum two vectors. C=A+B
*/
void qlvectsum(const qlvect *a,const qlvect *b, qlvect *c);
/*
Subtract two vectors. C=A-B
*/
void qlvectsub(const qlvect *a,const qlvect *b, qlvect *c);

/*Normalizes a vector (so that its length is 1)*/
void qlvectnormalize(qlvect *a);

/*Rotates a vector around the x,y and z axes by rx,ry and rz radians.*/
void qlvectrotate(qlvect *a,double rx,double ry,double rz);

/*Rotates a vector around the v axis by rv radians.*/
void qlvectrotateaxis(qlvect *a,const qlvect *r,double rv);

/*Vector-Triangle functions*/

/*
Calculates the factor by which a vector should be scaled to interesect a plane defined by a triangle t.
Said vector is defined by two vectors, one indicating its position and one for the orientation
Vector A,B; Triangle T
s=qlvectintersect(a,t)
If s is 0:
    Vector is already on plane.
If s is inf:
    Vector is parallel to the plane.
Else:
    Point p= A.s
    p lies on the plane on which T lies.
*/
double qlvectintersect(const qlvect *pos,const qlvect *dir,const qltri *t);
/*
Calculates whether a point (vector) lies within the subspace defined by a triangle qliding along its normal axis.
(E.g. if the point and the triangle are at the same plane, calculates whether the point is inside the triangle).
Returns 1 when the point lies within the subspace and 0 otherwise (or -1 for errors).
*/
char qlvectintri(const qlvect *a,const qltri *t);

/*Raycasting functions*/

/*Outputs a pointer to the address of the pixel at (x/s,y/s)*/
#define qlresolutionmultiply(x,y,scale) (floor(x/scale)+floor(y/scale)*xlen)*3
/*Calculates one cycle of a ray*/
void qlcalcray(qlray *ray,const qltri**triangles);
/*Cycles all the camera's rays*/
void qlstep(qlcamera *camera,const qltri**triangles);

/*Interaction functions*/

/*Multiplier for the direction vector when translating the camera*/
double walktick;
/*Rotation in radians for each camera update*/
double rottick;
/*Focal length change rate for each camera update*/
double fltick;

/*
Updates the camera position according to a keyboard event c
Use:
*w and s for forwards and backwards translation
*a and d for lateral translation
*t and g for vertical translation
*q and e for z-rotation
*r and f for y-rotation
*z and x for camera focal length
*/
void qlcameractl(qlcamera *camera,char c);

/*Constants*/
/*(1,0,0)*/
qlvect qlx;
/*(0,1,0)*/
qlvect qly;
/*(0,0,1)*/
qlvect qlz;

#endif