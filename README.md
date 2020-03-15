# quicklight
A C-based raycaster for X11.

## Building

If you are on a Linux environment, run the `source buildtests.sh` command at the rood directory of the project.

On other environments, build each example individually with the `gcc -o build/<FILENAME>.<EXE/OUT/Your binary extension> tests/<FILENAME> src/slowlight.c src/slt.c external/gfx.c -g -lm -lX11` command.

for each example you want to build on other environments (though this was only tested on linux).

## Maths
This project uses basic vector operations. If you want to understand them better, I have attached a GeoGebra 3D file at the docs folder with which you can play around to get a more intuitive notion of what is going on ([Triangle_Subspace_Collision(1).ggb](./docs/Triangle_Subspace_Collision(1).ggb)).

There are wikipedia links at the maths-heavy parts of the code.

## History
After finishing my other raycaster ([slowlight](https://github.com/m3101/slowlight)), I decided to make a simplified version for little pure C projects.

## Disclaimer
This project was built and is maintained by a single person, so there are probably bugs aplenty. If you have any suggestion, please open an Issue or e-mail me.

I hope this project brought you some interesting ideas or just a fun pastime.

^_^

\- Amélia