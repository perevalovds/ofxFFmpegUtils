# ofxFFmpegUtils

An addon for creating MP4 videos from video sequences.

Addon calls ffmpeg.exe and ffprobe.exe.

perevalovds: removed many-threaded implementation and dependencies from ofxExternalProcess and ofxPoco.

# Requirements

* Windows 10 (will work on another OS, but reauires to change path to fmpeg binaries)

* oF 0.10.1, Visual Studio C++ 2017

* 64 bit (will work 32 bit, but currently ffmpeg in example's 'bin' folder is 64 bit).


# How to Use

Check the example: 

1. Create project for 'example' using Project Generator.

2. Copy ffmpeg.exe and ffprobe.exe from 'add_to_bin' folder to 'example/bin" folder.

3. Run the example, and press "1" to create video from the sequence placed in 'data' folder.

# Using virtual drive in RAM

When creating MP4 files, the image sequence is stored in disk.
To fasten this, it's possible to creative virtual drive in RAM, using special software.
For example, look at imdiskinst application. It's installer is in utils folder. 
Install and create a virtual drive using Control Panel.
See http://www.ltr-data.se/opencode.html/#ImDisk for details.


# Credits

Addon is forked from armadillu/ofxFFmpegUtils and modyfied for my needs (Denis Perevalov).
