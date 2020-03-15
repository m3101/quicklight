#include "./quicklight.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/*
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

/*
The following vectors are general-purpose vectors used under many different names by the functions below.
This makes it so they don't have to spend cycles allocating and freeing memory for temporary vectors.
*/
qlvect _qlgp0={0,0,0};
qlvect _qlgp1={0,0,0};
qlvect _qlgp2={0,0,0};
qlvect _qlgp3={0,0,0};
qlvect _qlgp4={0,0,0};

qlvect _qlgpm0={0,0,0};
qlvect _qlgpm1={0,0,0};
qlvect _qlgpm2={0,0,0};
qlvect _qlgpm3={0,0,0};

qlvect qlx={1,0,0};
qlvect qly={0,1,0};
qlvect qlz={0,0,1};

/*
Instantiators
*/

qlraster* Qlraster(int w, int h, int s)
{
    qlraster *ret=malloc(sizeof(qlraster));
    ret->data=malloc(w*h*s);
    ret->w=w;
    ret->h=h;
    ret->s=s;
    return ret;
}
void freeqlraster(qlraster** obj)
{
    if(!obj||!(*obj))return;
    if((*obj)->data)free((*obj)->data);
    free(*obj);
    *obj=NULL;
}

qlvect* Qlvect(double x, double y, double z)
{
    qlvect* ret=malloc(sizeof(qlvect));
    ret->x=x;
    ret->y=y;
    ret->z=z;
    return ret;
}

qlray* Qlray(qlraster* screen, int rx, int ry, const qlvect *pos, const qlvect *dir)
{
    qlray* ret=malloc(sizeof(qlray));
    ret->screen=screen;
    ret->rx=rx;
    ret->ry=ry;
    ret->pos=*pos;
    ret->dir=*dir;
    ret->depth=100;
    return ret;
}

qltri* Qltri(const qlvect *a,const qlvect *b,const qlvect *c)
{
    qltri* ret=malloc(sizeof(qltri));
    ret->a=*a;
    ret->b=*b;
    ret->c=*c;
    return ret;
}

qlcamera *Qlcamera(qlraster *image,const qlvect *pos,const qlvect *dir,const double roll,const double fl,const double w,const double h,const double depth)
{
    if(!image||!pos||!dir)return NULL;
    qlcamera *ret=malloc(sizeof(qlcamera));
    qlvect *rpos=&_qlgpm0,*rdir=&_qlgpm1,*focalpoint=&_qlgpm3;
    int length=image->h*image->w;
    int i,x,y;
    ret->pos=*pos;
    ret->dir=*dir;
    qlvectnormalize(&ret->dir);
    ret->image=image;
    ret->fl=fl;
    ret->w=w;
    ret->h=h;
    ret->roll=roll;
    ret->depth=depth;

    /*We first define the focal point of the camera*/
    qlvectscale(dir,fl,rdir);
    qlvectsub(pos,rdir,focalpoint);

    ret->rays=malloc(length*sizeof(qlray));
    for(i=0;i<length;i++)
    {
        x=i%image->w;
        y=floor(i/image->w);
        ret->rays[i]=Qlray(image,x,y,rpos,rdir);
    }
    qlupdatecamera(ret);
    return ret;
}

void qlupdatecamera(qlcamera *camera)
{
    if(!camera)return;
    qlvect *rpos=&_qlgpm0,*rdir=&_qlgpm1,*rotaxis=&_qlgpm2,*focalpoint=&_qlgpm3;
    int length=camera->image->h*camera->image->w;
    int i,x,y;

    /*We first define the focal point of the camera*/
    qlvectscale(&camera->dir,camera->fl,rdir);
    qlvectsub(&camera->pos,rdir,focalpoint);

    for(i=0;i<length;i++)
    {
        x=i%camera->image->w;
        y=floor(i/camera->image->w);
        /*First we place the vector as if the camera was pointing upwards, that is, (0,0,1) at (0,0,0)*/
        rpos->x=(-(camera->w/2))+((camera->w/(camera->image->w-1))*x);
        rpos->y=(-(camera->h/2))+((camera->h/(camera->image->h-1))*y);
        rpos->z=0;
        /*Then we rotate them so they align with the camera's normal vector*/
        qlvectproduct(&camera->dir,&qlz,rotaxis);
        qlvectrotateaxis(rpos,rotaxis,acos(qlscproduct(&camera->dir,&qlz)));
        /*Then we roll them to the specified roll*/
        qlvectrotateaxis(rpos,&camera->dir,camera->roll);
        /*Then displace them to the camera position*/
        qlvectsum(rpos,&camera->pos,rpos);
        /*Now we have positioned the ray, let's find its direction.*/
        qlvectsub(rpos,focalpoint,rdir);
        qlvectnormalize(rdir);
        camera->rays[i]->dir=*rdir;
        camera->rays[i]->pos=*rpos;
        camera->rays[i]->depth=camera->depth;
    }
}

void freeqlcamera(qlcamera **camera)
{
    if(!camera||!(*camera))return;
    int i,s;
    s=(*camera)->image->h*(*camera)->image->w*(*camera)->image->s;
    for(i=0;i<s;i++)free((*camera)->rays[i]);
    free(*camera);
    *camera=NULL;
}

/*
Vector functions
*/
double qlscproduct(const qlvect *a,const qlvect *b)
{
    if(!a||!b)return -1;
    return a->x*b->x+a->y*b->y+a->z*b->z;
}

void qlvectproduct(const qlvect *a,const qlvect *b,qlvect *c)
{
    if(!a||!b||!c)return;
    qlvect *temp=&_qlgp4;
    temp->x=a->y*b->z-a->z*b->y;
    temp->y=a->z*b->x-a->x*b->z;
    temp->z=a->x*b->y-a->y*b->x;
    c->x=temp->x;
    c->y=temp->y;
    c->z=temp->z;
}

void qlvectscale(const qlvect *a,double s,qlvect *b)
{
    if(!a||!b)return;
    b->x=a->x*s;
    b->y=a->y*s;
    b->z=a->z*s;
}

void qlvectsum(const qlvect *a,const qlvect *b, qlvect *c)
{
    if(!a||!b||!c)return;
    c->x=a->x+b->x;
    c->y=a->y+b->y;
    c->z=a->z+b->z;
}

void qlvectsub(const qlvect *a,const qlvect *b, qlvect *c)
{
    if(!a||!b||!c)return;
    c->x=a->x-b->x;
    c->y=a->y-b->y;
    c->z=a->z-b->z;
}

void qlvectnormalize(qlvect *a)
{
    double sf=sqrt(qlscproduct(a,a));
    a->x=a->x/sf;
    a->y=a->y/sf;
    a->z=a->z/sf;
}

/*See https://en.wikipedia.org/wiki/Rotation_matrix#In_three_dimensions - General Rotations*/
void qlvectrotate(qlvect *a,double rx,double ry,double rz)
{
    double srx=sin(rx),sry=sin(ry),srz=sin(rz),crx=cos(rx),cry=cos(ry),crz=cos(rz);
    qlvect* tmp=&_qlgp0;
    tmp->x=(a->x*(crz*cry))+(a->y*(crz*sry*srx-srz*crx))+(a->z*(crz*sry*crx+srz*srx));
    tmp->y=(a->x*(srz*cry))+(a->y*(srz*sry*srx+crz*crx))+(a->z*(srz*sry*crx-crz*srx));
    tmp->z=(a->x*(-sry))+(a->y*(cry*srx))+(a->z*(cry*crx));
    a->x=tmp->x;
    a->y=tmp->y;
    a->z=tmp->z;
}

/*See https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula*/
void qlvectrotateaxis(qlvect *a,const qlvect *r,double rv)
{
    qlvect* res=&_qlgp0,*temp=&_qlgp1;
    qlvectscale(a,cos(rv),res);
    qlvectproduct(r,a,temp);
    qlvectscale(temp,sin(rv),temp);
    qlvectsum(res,temp,res);
    qlvectscale(r,qlscproduct(r,a)*(1-cos(rv)),temp);
    qlvectsum(res,temp,a);
}

/*
Vector-Triangle functions
*/

/*See https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection - Algebraic Form*/
double qlvectintersect(const qlvect *pos,const qlvect *dir,const qltri *t)
{
    if(!pos||!dir||!t)return 0;
    qlvect *result=&_qlgp0;
    qlvect *normal=&_qlgp1;
    qlvect *edge1=&_qlgp2;
    qlvect *edge2=&_qlgp3;
    double s;
    /*First we calculate the vectors corresponding to the edges of the triangle*/
    qlvectsub(&t->a,&t->b,edge1);
    qlvectsub(&t->a,&t->c,edge2);
    /*Then the normal*/
    qlvectproduct(edge1,edge2,normal);
    s=qlscproduct(dir,normal);
    /*If the vector is parallel to the plane*/
    if(s==0)return INFINITY;
    qlvectsub(&t->a,pos,result);
    s=qlscproduct(result,normal)/s;
    return s;
}
char qlvectintri(const qlvect *a,const qltri *t)
{
    if(!a||!t)return -1;
    /*A vector representing the edge we are currently analysing*/
    qlvect *edge=&_qlgp0;
    /*A vector starting on the first vertex of the edge and ending on our point*/
    qlvect *vp=&_qlgp1;
    /*A normal vector. We'll use it for checking if the other vectors point roughly towards the same direction*/
    qlvect *normal=&_qlgp2;
    /*Result vector*/
    qlvect *result=&_qlgp3;
    double reference;/*Scalar product of the first vector and the normal vector*/
    /*AB edge*/
    qlvectsub(&t->a,&t->b,edge);

    /*We'll use this opportunity to calculate the normal vector, too*/
    qlvectsub(&t->a,&t->c,vp);
    qlvectproduct(vp,edge,normal);

    qlvectsub(a,&t->a,vp);
    qlvectproduct(vp,edge,result);
    reference=qlscproduct(result,normal);
    /*BC edge*/
    qlvectsub(&t->b,&t->c,edge);
    qlvectsub(a,&t->b,vp);
    qlvectproduct(vp,edge,result);
    if(qlscproduct(result,normal)*reference<0)return 0;
    /*CA edge*/
    qlvectsub(&t->c,&t->a,edge);
    qlvectsub(a,&t->c,vp);
    qlvectproduct(vp,edge,result);
    if(qlscproduct(result,normal)*reference<0)return 0;
    return 1;
}

/*Raycasting functions*/

double pmaxs=0;
double maxs=0;

#ifndef QL_CUSTOM_RAYS
void qlcalcray(qlray *ray,const qltri**triangles)
{
    int i=0;
    double s;
    double min=__DBL_MAX__;
    qlvect *pos=&_qlgpm0,*dir=&_qlgpm1;
    if(!ray||!triangles||!(triangles[0]))return;
    double S=0.0;
    double bright;
    while(triangles[i]!=NULL)
    {
        *dir=ray->dir;
        qlvectnormalize(dir);
        s=qlvectintersect(&ray->pos,dir,triangles[i]);
        if(s>=0&&s<min&&s<ray->depth)
        {
            qlvectscale(dir,s,dir);
            qlvectsum(&ray->pos,dir,pos);
            if(qlvectintri(pos,triangles[i]))
            {
                maxs=maxs>s?maxs:s;
                min=s;
                bright=s<maxs?(s/maxs):1;
                bright=(S+((1-S)*(1-bright)));
                ray->screen->data[(ray->rx+ray->ry*ray->screen->w)*ray->screen->s]=(unsigned char)triangles[i]->colour[0]*bright;
                ray->screen->data[(ray->rx+ray->ry*ray->screen->w)*ray->screen->s+1]=(unsigned char)triangles[i]->colour[1]*bright;
                ray->screen->data[(ray->rx+ray->ry*ray->screen->w)*ray->screen->s+2]=(unsigned char)triangles[i]->colour[2]*bright;
            }
        }
        i++;
    }
    if(min>ray->depth)
    {
        ray->screen->data[(ray->rx+ray->ry*ray->screen->w)*ray->screen->s]=0;
        ray->screen->data[(ray->rx+ray->ry*ray->screen->w)*ray->screen->s+1]=0;
        ray->screen->data[(ray->rx+ray->ry*ray->screen->w)*ray->screen->s+2]=0;
    }
}
#endif
#ifndef QL_CUSTOM_STEP
void qlstep(qlcamera *camera,const qltri**triangles)
{
    if(!camera||!triangles||!triangles[0])return;
    int i,s;
    maxs=(8*pmaxs)/10;
    s=camera->image->h*camera->image->w*camera->image->s;
    for(i=0;i<s;i++)
        qlcalcray(camera->rays[i],triangles);
    pmaxs=maxs;
}
#endif

/*Interaction functions*/

double walktick=0.3;
double rottick=10*(QL_PI/180);
double fltick=0.1;

void qlcameractl(qlcamera *camera,char c)
{
    qlvect *dir=&_qlgpm0,*norm=&_qlgpm1;
    if(c)
    {
        switch (c)
        {
        case 'w':
            qlvectscale(&camera->dir,walktick,dir);
            qlvectsum(&camera->pos,dir,&camera->pos);
            break;
        case 's':
            qlvectscale(&camera->dir,-walktick,dir);
            qlvectsum(&camera->pos,dir,&camera->pos);
            break;
        case 'a':
            qlvectscale(&camera->dir,walktick,dir);
            qlvectrotate(dir,0,0,-QL_PI/2);
            qlvectsum(&camera->pos,dir,&camera->pos);
            break;
        case 'd':
            qlvectscale(&camera->dir,walktick,dir);
            qlvectrotate(dir,0,0,QL_PI/2);
            qlvectsum(&camera->pos,dir,&camera->pos);
            break;
        case 't':
            qlvectscale(&camera->dir,walktick,dir);
            qlvectproduct(dir,&qlz,norm);
            qlvectrotateaxis(dir,norm,QL_PI/2);
            qlvectsum(&camera->pos,dir,&camera->pos);
            break;
        case 'g':
            qlvectscale(&camera->dir,walktick,dir);
            qlvectproduct(dir,&qlz,norm);
            qlvectrotateaxis(dir,norm,-QL_PI/2);
            qlvectsum(&camera->pos,dir,&camera->pos);
            break;
        case 'q':
            qlvectrotate(&camera->dir,0,0,-rottick);
            camera->roll+=rottick;
            break;
        case 'e':
            qlvectrotate(&camera->dir,0,0,rottick);
            camera->roll-=rottick;
            break;
         case 'r':
            qlvectproduct(&camera->dir,&qlz,norm);
            qlvectrotateaxis(&camera->dir,norm,rottick);
            //camera->roll-=rottick*sqrt(qlscproduct(norm,norm));
            break;
        case 'f':
            qlvectproduct(&camera->dir,&qlz,norm);
            qlvectrotateaxis(&camera->dir,norm,-rottick);
            //camera->roll+=rottick*sqrt(qlscproduct(norm,norm));
            break;
        case 'z':
            camera->fl+=fltick;
            break;
        case 'x':
            camera->fl-=fltick;
            break;
        default:
            break;
        }
    }
    qlupdatecamera(camera);
}