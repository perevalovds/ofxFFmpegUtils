# ofxFFmpegUtils

An addon for creating MP4 videos from video sequences.

Addon calls ffmpeg.exe and ffprobe.exe.

# Requirements

* Windows 10 (will work on another OS, but reauires to change path to fmpeg binaries)

* oF 0.10.1, Visual Studio C++ 2017

* 64 bit (will work 32 bit, but currently ffmpeg in example's 'bin' folder is 64 bit).

# Dependencies

* ofxExternalProcess addon (which also requires standard ofxPoco addon)


# How to Use

Check the example: 

1. Create project for 'example' using Project Generator.

2. Copy ffmpeg.exe and ffprobe.exe from 'add_to_bin' folder to 'example/bin" folder.

3. Run the example, and press "1" to create video from the sequence placed in 'data' folder.

#Credits

Originall addon is forked from armadillu/ofxFFmpegUtils.
