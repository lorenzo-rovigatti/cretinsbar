# cretinsbar

## Installation
cretinsbar requires cmake, qt5 and SoundTouch. On Ubuntu (or any Debian-derived distro, I believe) this boils down to installing
* cmake
* qtbase5-dev
* qtmultimedia5-dev
* libsoundtouch-dev

Once all the dependencies are met, the code can be compiled as follows:
* ``$ mkdir build``
* ``$ cd build``
* ``$ cmake ..``
* ``$ make``

If the compilation is successful, the cretinsbar executable will be placed in the build/bin folder. 

## Features

## Future features

## Tentative roadmap

## Acknowledgements
* The SoundUtils class is heavily based on what I have found [here](http://www.morethantechnical.com/2014/10/13/touch-your-sound-with-soundtouch-wcode/)
* The FindSoundTouch.cmake file is from the [pcsx2](http://pcsx2.net/) project 
