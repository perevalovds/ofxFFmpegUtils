//
//  ofxFFmpegUtils.cpp
//  BasicSketch
//
//  Created by Oriol Ferrer Mesià on 16/03/2018.
//
//

#include "ofxFFmpegUtils.h"

ofxFFmpegUtils::ofxFFmpegUtils(){
}

ofxFFmpegUtils::~ofxFFmpegUtils(){
	
}

void ofxFFmpegUtils::setup(const string & ffmpegBinaryPath, const string & ffProbeBinaryPath){
	this->ffmpegBinaryPath = ffmpegBinaryPath;
	this->ffProbeBinaryPath = ffProbeBinaryPath;
}

bool ofxFFmpegUtils::isFFMpegAvailable(){
	return ofFile::doesFileExist(ffmpegBinaryPath) && ofFile::doesFileExist(ffProbeBinaryPath);
}

ofJson ofxFFmpegUtils::getVideoInfo(const string & filePath){
	//https://gist.github.com/nrk/2286511
	//ffprobe -v quiet -print_format json -show_format -show_streams
	string jsonString = ofSystem(ffProbeBinaryPath + " -v quiet -print_format json -show_format -show_streams \"" + filePath + "\"");
	return ofJson::parse(jsonString);
}


ofVec2f ofxFFmpegUtils::getVideoResolution(const string & movieFilePath){

	string videoSize = ofSystem(ffProbeBinaryPath + " -v error -select_streams v -show_entries stream=width,height -of default=noprint_wrappers=1 \"" + movieFilePath + "\"");
	auto lines = ofSplitString(videoSize, "\n");
	int w = 0;
	int h = 0;
	ofRectangle r;
	for(auto & s : lines){
		ofStringReplace(s, " ", "");
		if(ofStringTimesInString(s, "width=")){
			ofStringReplace(s, "width=", "");
			w = ofToInt(s);
		}
		if(ofStringTimesInString(s, "height=")){
			ofStringReplace(s, "height=", "");
			h = ofToInt(s);
		}
	}
	ofLogNotice("ofxFFmpegUtils") << "detected resolution for video " << movieFilePath << " : [" << w << "x" << h << "]";
	return ofVec2f(w,h);
}


float ofxFFmpegUtils::getVideoFramerate(const string & movieFilePath){

	//ffprobe -v 0 -of csv=p=0 -select_streams 0 -show_entries stream=r_frame_rate infile
	//https://askubuntu.com/questions/110264/how-to-find-frames-per-second-of-any-video-file

	string framerate = ofSystem(ffProbeBinaryPath + " -v 0 -of csv=p=0 -select_streams v -show_entries stream=r_frame_rate \"" + movieFilePath + "\"");
	auto manyLines = ofSplitString(framerate, "\n");
	if(manyLines.size() > 1){
		framerate = manyLines[0];
	}
	auto split = ofSplitString(framerate, "/");
	if (split.size() != 2){
		ofLogError("ofxFFmpegUtils") << "can't detect framerate for video " << movieFilePath;
		return 0;
	}else{
		int val1 = ofToInt(split[0]);
		int val2 = ofToInt(split[1]);
		float fr = float(val1) / float(val2);
		ofLogNotice("ofxFFmpegUtils") << "detected framerate for video " << movieFilePath << " : " << fr;
		return fr;
	}
}

void ofxFFmpegUtils::imgSequenceToMP4(const string & imgFolder,
									  float framerate,
									  float compressQuality,
									  const string &filenameFormat, 		//ie frame_%08d
									  const string & imgFileExtension, 	//ie tiff
									  const string & outputMovieFilePath){

	vector<string> args;

	// yuv444p not supported in all apps, but keeps colors better
	args = {
		"-framerate", ofToString(framerate, 4),
		"-y", //overwrite
		"-i" , "\"" + imgFolder + "/" + filenameFormat + "." + imgFileExtension + "\"",
		"-profile:v", "high444", // "baseline", 
		"-level", "5.1", //"3.0",
		"-x264opts", "keyint=30",
		"-vf", "format=yuv444p,eq=gamma=1.0", //"format=yuv420p",
		"-crf", ofToString((int)ofMap(compressQuality, 0, 1, 51, 0, true), 0),
		"-loglevel", "30", //verbose
		"-nostdin",
		//"-nostats", //this removes the \r stuff with progresss
		outputMovieFilePath
	};

	if(extraArguments.size()){
		args.insert(args.begin(), extraArguments.begin(), extraArguments.end());
	}

	string command = ffmpegBinaryPath + " " + ofJoinString(args, " ");

	cout << "Starting ffmpeg with command:" << endl;
	cout << command << endl;
	string res = ofSystem(command);
	cout << "ffmpeg finished " << res << endl;
}


void ofxFFmpegUtils::convertToImageSequence(const string & movieFile, const string & imgFileExtension, float jpegQuality/*[0..1]*/,
											  const string & outputFolder, bool convertToGrayscale, int numFilenameDigits,
											ofVec2f resizeBox, ofVec2f cropToAspectRatio, float cropBalance){
	
	vector<string> args;

	//beware - this overwrites
	if(ofDirectory::doesDirectoryExist(ofToDataPath(outputFolder, true))){
		ofDirectory::removeDirectory(ofToDataPath(outputFolder, true), true); //remove old
	}
	ofDirectory::createDirectory(ofToDataPath(outputFolder, true), true, true);

	string imgNameScheme = ofToDataPath(outputFolder, true) + "/" + "%0" + ofToString(numFilenameDigits) + "d." + imgFileExtension;

	//ffmpeg -i "$inputMovie" -q:v $jpegQuality -coder "raw" -y  -loglevel 40 "$folderName/output_%06d.$format"
	args = {
		"-i", ofToDataPath(movieFile, true),
		"-q:v", ofToString((int)ofMap(0, 1, 31, 1, true)),
		"-coder", "raw",
		"-y", //overwrite
		//"-pix_fmt", "yuv420p",
		"-loglevel", "40", //verbose
		//"-nostats", //this removes the \r stuff with progresss
		imgNameScheme
	};

	if(extraArguments.size()){
		args.insert(args.begin(), extraArguments.begin(), extraArguments.end());
	}

	auto fps = getVideoFramerate(movieFile);

	auto res = getVideoResolution(movieFile);
	int w = res.x;
	int h = res.y;
	ofRectangle r = ofRectangle(0,0,w,h); //the final img pixel size (may or may not be resized) b4 cropping
	ofRectangle resizeTarget = ofRectangle(0,0,resizeBox.x, resizeBox.y);

	bool isResizing = resizeBox.x > 0 && resizeBox.y > 0;
	bool isCropping = cropToAspectRatio.x > 0 && cropToAspectRatio.y > 0;

	ofJson json;
	json["framerate"] = fps;
	json["resolution"]["x"] = res.x;
	json["resolution"]["y"] = res.y;
	json["originalFile"] = movieFile;
	json["imgExtension"] = imgFileExtension;
	json["jpegQuality"] = jpegQuality;
	json["numFilenameDigits"] = numFilenameDigits;
	json["cropBalance"] = cropBalance;
	json["convertToGrayscale"] = convertToGrayscale;
	ofSaveJson(ofToDataPath(outputFolder, true) + "/info.json", json);

	vector<string> vfArgs;
	bool needVF = false;

	if( isResizing && !isCropping ){ //only resize

		r.scaleTo(resizeTarget);

		if(r.width != 0 && r.height != 0){
			needVF = true;
			vfArgs.push_back( "scale=" + ofToString(r.width,0) + ":" + ofToString(r.height,0) );
			isResizing = true;
		}else{
			ofLogError("ofxFFmpegUtils") << "cant get video res! cant resize video! " << movieFile;
		}
	}

	if(cropBalance < 0.0) cropBalance = 0.5; //if no balance defined (-1), crop in middle

	if(isCropping && !isResizing){ //only crop

		ofRectangle crop = ofRectangle(0,0,cropToAspectRatio.x,cropToAspectRatio.y);
		crop.scaleTo(r, OF_SCALEMODE_FIT);

		string command;

		if( int(crop.width) == int(r.width) ){ //we are cropping vertically - we must remove pix from top or bottom

			int diff = r.height - crop.height;
			int cropOffset = ofMap(cropBalance, 0, 1, 0, diff);
			command = "crop=" + ofToString(crop.width,0) + ":" + ofToString(crop.height,0) + ":" + "0" + ":" + ofToString(cropOffset);

		}else{ //we are cropping horizontally - we must remove pixels from left or right

			int diff = r.width - crop.width;
			int cropOffset = ofMap(cropBalance, 0, 1, 0, diff);
			command = "crop=" + ofToString(crop.width,0) + ":" + ofToString(crop.height,0) + ":" + ofToString(cropOffset) + ":" + "0";
		}

		needVF = true;
		vfArgs.push_back( command );
	}

	if(isCropping && isResizing){ //resize & crop

		ofRectangle crop = ofRectangle(0,0,cropToAspectRatio.x,cropToAspectRatio.y);
		crop.scaleTo(r, OF_SCALEMODE_FIT);
		crop.scaleTo(resizeTarget);

		string command;
		r.scaleTo(crop, OF_SCALEMODE_FILL);

		if( int(crop.width) == int(r.width) ){ //we are cropping vertically - we must remove pix from top or bottom

			int diff = r.height - crop.height;
			int cropOffset = ofMap(cropBalance, 0, 1, 0, diff);
			command = "scale=" + ofToString(r.width,0) + ":" + ofToString(r.height,0) + ",crop=" +
			ofToString(crop.width,0) + ":" + ofToString(crop.height,0) + ":" + "0" + ":" + ofToString(cropOffset);

		}else{ //we are cropping horizontally - we must remove pixels from left or right

			int diff = r.width - crop.width;
			int cropOffset = ofMap(cropBalance, 0, 1, 0, diff);
			command = "scale=" + ofToString(r.width,0) + ":" + ofToString(r.height,0) + ",crop=" +
			ofToString(crop.width,0) + ":" + ofToString(crop.height,0) + ":" + ofToString(cropOffset) + ":" + "0";
		}

		needVF = true;
		vfArgs.push_back( command );
	}


	//if(maxThreadsPerJob > 0){
	//	ofLogNotice("ofxFFmpegUtils") << "limiting ffmpeg job to " << maxThreadsPerJob << " threads.";
	//	args.insert(args.begin(), ofToString(maxThreadsPerJob));
	//	args.insert(args.begin(), "-threads");
	//}

	if(convertToGrayscale){
		needVF = true;
	}

	if(needVF){
		if(convertToGrayscale){ //note that grayscale conversion comes last!
			vfArgs.push_back("format=gray");
		}

		string totalVF;
		int c = 0;
		for(auto arg : vfArgs){
			totalVF += arg;
			c++;
			if(c < vfArgs.size()) totalVF += ",";
		}
		args.insert(args.begin() + args.size() - 1, "-vf");
		args.insert(args.begin() + args.size() - 1, totalVF);
	}

	cout << "Starting ffmpeg" << endl;
	string result = ofSystem(ffmpegBinaryPath + " " + ofJoinString(args, " "));
	cout << "ffmpeg finished " << res << endl;
}

