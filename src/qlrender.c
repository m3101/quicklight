#include <stdio.h>
#include <math.h>
#include "./qlrender.h"

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

static Colormap colormap=0;
static int fast_color_mode=0;

/*
Many thanks to the University of Notre Dame for their gfx API.
Much of my X11 knowledge comes from reverse engineering it, and it's extremely simple and easy to use/understand.
The methods used here are basically exact copies of the ones available there.
(https://www3.nd.edu/~dthain/courses/cse20211/fall2013/gfx/)
*/

qlscreen* Qlscreen(qlcamera *cam,int scale,const char* title)
{
    qlscreen* ret;
    if(!cam||scale<=0)return NULL;
    ret=malloc(sizeof(qlscreen));
    ret->s=scale;
    ret->display=XOpenDisplay(0);
    ret->cam=cam;
    if(!ret->display)return NULL;
    Visual *visual = DefaultVisual(ret->display,0);
    fast_color_mode = visual && visual->class==TrueColor?1:0;
    int blackColor = BlackPixel(ret->display, DefaultScreen(ret->display));
    int whiteColor = WhitePixel(ret->display, DefaultScreen(ret->display));
    ret->window = XCreateSimpleWindow(ret->display, DefaultRootWindow(ret->display), 0, 0, cam->image->w*scale, cam->image->h*scale, 0, blackColor, blackColor);
    XSetWindowAttributes attr;
    attr.backing_store = Always;
    XChangeWindowAttributes(ret->display,ret->window,CWBackingStore,&attr);
    XStoreName(ret->display,ret->window,title);
    XSelectInput(ret->display, ret->window, StructureNotifyMask|KeyPressMask|ButtonPressMask);
    XMapWindow(ret->display,ret->window);
    ret->gc = XCreateGC(ret->display, ret->window, 0, 0);
    if(!colormap)colormap = DefaultColormap(ret->display,0);
    XSetForeground(ret->display, ret->gc, whiteColor);
    for(;;) {
        XEvent e;
        XNextEvent(ret->display, &e);
        if (e.type == MapNotify)
            break;
    }
    return ret;
}

void qlrender(qlscreen* screen,qltri** world)
{
    int x,y,xx,yy,xsize,ysize,xlen,ylen,r,g,b;
    if(!screen||!world||!world[0])return;
    xlen=screen->cam->image->w;
    ylen=screen->cam->image->h;
    xsize=xlen*screen->s;
    ysize=ylen*screen->s;
    qlstep(screen->cam,(const qltri**)world);
    for(x=0;x<xsize;x++)
    {
        for(y=0;y<ysize;y++)
        {
            xx=floor(x/screen->s);
            yy=floor(y/screen->s);
            r=screen->cam->image->data[(xx+yy*xlen)*3];
            g=screen->cam->image->data[(xx+yy*xlen)*3+1];
            b=screen->cam->image->data[(xx+yy*xlen)*3+2];

            XColor color;
            if(fast_color_mode) {
                color.pixel = ((b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16) );
            } else {
                color.pixel = 0;
                color.red = r<<8;
                color.green = g<<8;
                color.blue = b<<8;
                XAllocColor(screen->display,colormap,&color);
            }
            XSetForeground(screen->display, screen->gc, color.pixel);
            XDrawPoint(screen->display,screen->window,screen->gc,x,y);
        }
    }
}

void qlrendernoise(qlscreen* screen,qltri** world,unsigned char rnd)
{
    int x,y,xx,yy,xsize,ysize,xlen,ylen,r,g,b;
    if(!screen||!world||!world[0])return;
    xlen=screen->cam->image->w;
    ylen=screen->cam->image->h;
    xsize=xlen*screen->s;
    ysize=ylen*screen->s;
    qlstep(screen->cam,(const qltri**)world);
    double rn;
    for(x=0;x<xsize;x++)
    {
        for(y=0;y<ysize;y++)
        {
            rn=1-(((rand()%256)/255.0)*rnd)/255.0;
            xx=floor(x/screen->s);
            yy=floor(y/screen->s);
            r=(unsigned char)((double)screen->cam->image->data[(xx+yy*xlen)*3])*rn;
            g=(unsigned char)((double)screen->cam->image->data[(xx+yy*xlen)*3+1])*rn;
            b=(unsigned char)((double)screen->cam->image->data[(xx+yy*xlen)*3+2])*rn;

            XColor color;
            if(fast_color_mode) {
                color.pixel = ((b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16) );
            } else {
                color.pixel = 0;
                color.red = r<<8;
                color.green = g<<8;
                color.blue = b<<8;
                XAllocColor(screen->display,colormap,&color);
            }
            XSetForeground(screen->display, screen->gc, color.pixel);
            XDrawPoint(screen->display,screen->window,screen->gc,x,y);
        }
    }
}

char qlevent(qlscreen *screen)
{
	XEvent event;
	if(XPending(screen->display))
		while(XPending(screen->display))
			XNextEvent(screen->display,&event);
	else return 0;

	if(event.type==KeyPress) {
		return XLookupKeysym(&event.xkey,0);
	} else if(event.type==ButtonPress) {
		return event.xbutton.button;
	}
}