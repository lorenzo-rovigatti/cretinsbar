# Cretin's Bar
Cretin's Bar is an open-source program for music transcription/play-along practice written in qt5/c++. The general idea behind the program comes from the fully-featured, highly professional commercial software [Transcribe!](https://www.seventhstring.com/xscribe/overview.html). Cretin's Bar wants to be a stripped-down, much less featured version of it, mostly useful for practice. 

## Installation
Cretin's Bar requires cmake, qt5 and SoundTouch. Optional mp3 support is provided by mpg123. On Ubuntu (or any Debian-derived distro, I believe) this boils down to installing
* cmake
* qtbase5-dev
* qtmultimedia5-dev
* libsoundtouch-dev
* libmpg123-dev (optional)

Once all the dependencies are met, the code can be compiled as follows:
* ``$ mkdir build``
* ``$ cd build``
* ``$ cmake ..``
* ``$ make``

If the compilation is successful, the cretinsbar executable will be placed in the build/bin folder. 

## Features
* Support for mp3 and 16-bit WAV files
* Slow down/speed up 
* Change pitch

## Future features
* Add support for flac, ogg, etc.

## Tentative roadmap
* Add save/load facilities
* Add support for more formats

## Acknowledgements
* The wave form widget is based on [QCustomPlot](http://qcustomplot.com/)
* The SoundUtils class is heavily based on what I have found [here](http://www.morethantechnical.com/2014/10/13/touch-your-sound-with-soundtouch-wcode/)
* The FindSoundTouch.cmake file is from the [pcsx2](http://pcsx2.net/) project
* The Findmpg123.cmake file is from the [zdoom](https://github.com/rheit/zdoom) project
* The Wave class is heavily based on the one developed by [trodevel](https://github.com/trodevel/wave)
