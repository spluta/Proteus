# Proteus
Sam Pluta

This is a SuperCollider UGen implementation of the Proteus VST plugin by GuitarML, aka Keith Bloemer. The LSTM models are modefied from the Proteus source: https://github.com/GuitarML/Proteus and this implementation is largely modeled on his excellent project. Go download the VST!

This implementation should be able to load any of the Proteus models found in the Tone Library at https://guitarml.com/ and you can even train your own models: https://youtu.be/vwsSYpqRqyM?feature=shared

The project uses the RTNeural library by Jatin Chowdhury of ChowDSP and libsamplerate or Secret Rabbit Code by Erik de Castro Lopo.

Installation:

If you can, you probably want to use one of the pre-build binaries available on GitHub. On mac you will need to 

Building:

1. Download this repository to its own directory.

2. Download the RTNeural and libsamplerate submodules:
```
git submodule update --init --recursive
```

3. Build libsamplerate:
```
cd libsamplerate
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
```

4. Build the Plugin (from the Proteus main directory):
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSC_PATH=<PATH TO SC SOURCE> 
cmake --build . --config Release
```

It should build Proteus plugin and leave the .scx file in the build directory

After building make sure this directory the scx, sc, and schelp files are in the SC path, recompile the SC libary, and they should work. 

