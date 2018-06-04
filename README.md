06/04/2018:
Micro$oft has acquired github, you can find updates from this software at: https://coto88.bitbucket.io/
Have a good day.

# SnemulDS
SnemulDS 0.6b Original Sources (because codebase it´s not either 0.5 or 0.6a from snemuldsx source codes)... 
This is based from real 0.6.a sources archeide left years ago and I managed to find.

/release folder has precompiled binaries for you to enjoy

Compile Toolchain: To compile this project you will need to follow the steps at https://github.com/cotodevel/ToolchainGenericDS: Then simply extract the project somewhere.

Compile this project: 

[Windows]
Open msys, through msys commands head to the directory your extracted this project. 
Then write: make clean make

[Linux]
Then write: make clean make

After compiling, run the example in NDS.

Project Specific description: 

Coto: added ZIP support:
The code was taken from NesDS, and changed some stuff so it could work with Toolchain Generic DS.

Requisites:

a) Every .zip file must be within root:/snes/ folder, where root is your sd letter.
b) The file extension must be renamed to .smc before compression.
c) The compressed file must use  .zip (deflate) or .gz, and the compressed .zip must have 1 only file.


Optional:
GDB Remote debugging is now possible, to enable GDB Remote Debugging you will head to: common/specific_shared.h and change the commented out line:
//#define GDB_ENABLE

into uncommented:
#define GDB_ENABLE

1.  Then recompile the project, note that games won´t boot while GDB Remote debugging due to TCP nature, this is normal. 
2.  Follow the steps @ https://github.com/cotodevel/ToolchainGenericDS-gdbstub-example

to disable GDB Remote debugging simply comment out the GDB_ENABLE line so it looks like this:
//#define GDB_ENABLE
and recompile project.


to do:

-   fix Mode 0 (no transparency/broken)
-   fix memory leaks when games exceed the existing paging memory pool (6MB games)


thanks to:

Archeide for the emulator & source code
Bubble2k for CPU core
Gladius for Sound Core


Coto.
