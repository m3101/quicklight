#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "../src/quicklight.h"
#include "../src/qslt.h"
#include "../src/qlrender.h"

int main()
{
	int size=100;
	int scale=5;
	qlraster *raster=Qlraster(size,size,3);
	if(!raster)return -1;
	qlvect *pos=Qlvect(-3,3,4),*dir=Qlvect(1,-1,0);
	qlcamera *cam=Qlcamera(raster,pos,dir,-QL_PI/4,5,5,5,10);
	qltri** triangles=qltToQltriList("build/polgono.slt");
    if(!triangles)
	{
        printf("polgono.slt not found!\n");
		return -1;
	}
    else printf("polgono.slt imported. Length: %d\n",qllen((void**)triangles));
	qlscreen *scr=Qlscreen(cam,scale,"Quicklight");

	int cy=0,cyc=15000;
	struct timespec tim, tim2;
	tim.tv_sec = 0;
   	tim.tv_nsec = 1;
	while(cy<cyc) {
		qlrender(scr,triangles);
		char c=qlevent(scr);
		qlcameractl(cam,c);
		nanosleep(&tim,&tim2);
		cy++;
	}

	freeqlcamera(&cam);
	freeqlraster(&raster);
	free(pos);free(dir);
	freeqltriarray(&triangles);
	printf("Ok.");
	return 0;
}
