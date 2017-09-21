#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {

public:
	// Utils
	float time = 0;
	float nextTime = 0;
	float intervals = 20;
	ofTrueTypeFont font;
	bool isFullscreen = true;
	bool syncOnNextFrame = false;

	// General
	vector<string> arguments;
	vector<ofImage> images;
	vector<string> imagesNames;
	vector<vector<ofVec2f>> landmarks;
	vector<string> landmarksNames;
	vector<string> newLandmarks;

	// Cache that keeps track of processed images
	// We don't wnat to process twice an image
	// As the RPI_sync function may download a same image multiple times if the Raspberry Pi did not removed them,
	// We want to keep track of all processed images.

	vector<string> cachedFiles;

	// Pathes
	string BASE_PATH;
	string SYNC_RPI_1_CMD_PATH;
	string SYNC_RPI_2_CMD_PATH;
	string SYNC_RPI_3_CMD_PATH;
	string LANDMARK_EXE_PATH;
	string RPI_1_FOLDER_PATH;
	string RPI_2_FOLDER_PATH;
	string RPI_3_FOLDER_PATH;
	string SYNC_FOLDER_PATH;
	string TO_PRINT_FOLDER_PATH;
	string LANDMARKS_FOLDER_PATH;

	// Beta
	string BACKUP_RAW_FOLDER_PATH;
	string BACKUP_CROPPED_FOLDER_PATH;
	string BACKUP_LANDMARK_FOLDER_PATH;

	// General
	void setup();
	void update();
	void draw();

	// Utils
	void keyReleased(int key);
	void consoleText(string text);

	// Server
	void runServerSequence();
	void RPISync();
	vector<string> getNewFiles(bool removeOldFiles);
	void runLandmarksComputation(vector<string> imagesPathes);
	ofImage getCroppedFace(ofImage inputImage, vector<ofVec2f> landmarks);
	void print();
	void copyFolder(string inputFolderPath, string outputFolderPath);
	void clearFolder(string folderPath);

	// Server utils
	vector<ofVec2f> parseLandmarksFile(string filePath);
	vector<ofVec2f> getBoundingBox(vector<ofVec2f> landmarks);
	vector<ofVec2f> getFacialContour(vector<ofVec2f> landmarks);
	int newLandmarksLength();
	int newFacesLength();

};
