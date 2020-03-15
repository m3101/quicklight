/*
Quicklight raycaster-like renderer - X11 API

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

#ifndef QLRENDER
#define QLRENDER

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include "quicklight.h"

/*Data structures and allocation functions*/

static Colormap colormap;
static int fast_color_mode;

/*
A qlrender screen instance (opens a X11 instance)
*/
typedef struct _qlscreen{
    Display *display;
    Window  window;
    qlcamera *cam;
    GC gc;
    int s;
} qlscreen;
/*
Instantiates a new screen bound to camera cam and a new X11 display. It will scale the image up <int scale>-fold.
*/
qlscreen* Qlscreen(qlcamera *cam,int scale,const char* title);

/*Renders a frame. qltri** world is a list of all the triangles in the scene*/
void qlrender(qlscreen* screen,qltri** world);

/*Renders a frame. qltri** world is a list of all the triangles in the scene. Randomizes the shadows so it looks more like a camera*/
void qlrendernoise(qlscreen* screen,qltri** world,unsigned char rnd);

/*Returns the charcode of the last event (or 0 if there are no events in the buffer)*/
char qlevent(qlscreen *screen);

#endif