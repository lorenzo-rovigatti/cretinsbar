# Cretin's Bar
Cretin's Bar will be a much-less-featured (at least for the foreseeable future...) open-source alternative to [Transcribe!](https://www.seventhstring.com/xscribe/overview.html).

## Installation
Cretin's Bar requires cmake, qt5 and SoundTouch. On Ubuntu (or any Debian-derived distro, I believe) this boils down to installing
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
* Support for 16-bit WAV files
* Slow down/speed up 
* Change pitch

## Future features
* Add support for mp3, ogg, etc.

## Tentative roadmap

## Acknowledgements
* The SoundUtils class is heavily based on what I have found [here](http://www.morethantechnical.com/2014/10/13/touch-your-sound-with-soundtouch-wcode/)
* The FindSoundTouch.cmake file is from the [pcsx2](http://pcsx2.net/) project
* The Wave class is heavily based on the one developed by [trodevel](https://github.com/trodevel/wave)
