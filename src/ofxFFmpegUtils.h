//
//  ofxFFmpegUtils.h
//  BasicSketch
//
//  Created by Oriol Ferrer Mesià on 16/03/2018.
//  Modified by perevalovds
//

#pragma once
#include "ofMain.h"

class ofxFFmpegUtils{

public:
	
	ofxFFmpegUtils();
	~ofxFFmpegUtils();

	void setup(const string & ffmpegBinaryPath, const string & ffProbeBinaryPath);

	bool isFFMpegAvailable();

	//returns video res of a specific file (spaws an external process & blocks!)
	ofVec2f getVideoResolution(const string & movieFilePath);
	float getVideoFramerate(const string & movieFilePath);
	ofJson getVideoInfo(const string & movieFilePath); //returns a json object

	void setExtraArguments(vector<string> args){extraArguments = args;};
	void clearExtraArguments(){extraArguments.clear();}

	//returns a jobID
	void convertToImageSequence(const string & movieFilePath,
								const string & imgFileExtension, //"jpeg", "tiff", etc
								float jpegQuality/*[0..1]*/,
								const string & outputFolder,
								bool convertToGrayscale,
								int numFilenameDigits = 6, // "output_00004.jpg" ctrl # of leading zeros
								ofVec2f resizeBox = ofVec2f(-1,-1), //if you supply a size, img sequence will be resized so that it fits in that size (keeping aspect ratio)
								ofVec2f cropToAspectRatio = ofVec2f(-1,-1),
								float cropBalance = -1 	//[0..1] if we are cropping, what's the crop mapping?
														//this is a loose param, works for horizontal and vertical crop
														// 0 would mean crop all the "right" (or bottom) pixels that dont fit in the A/R

								);

	//returns a jobID
	void imgSequenceToMP4(	const string & imgFolder,
							float framerate,
							float compressQuality, /*0..1*/
						  	const string &filenameFormat, 		//ie frame_%08d
						  	const string & imgFileExtension, 	//ie tiff
						  	const string & outputMovieFilePath 		//result movie file path
						  );	


protected:

	vector<string> extraArguments;

	string ffmpegBinaryPath;
	string ffProbeBinaryPath;

};
