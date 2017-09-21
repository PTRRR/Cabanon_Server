#include "ofApp.h"

//--------------------------------------------------------------
// Init

void ofApp::setup() {

	// Seting up project pathes
	if (arguments.size() > 2) {
		BASE_PATH = arguments[2];
	}
	else {
		BASE_PATH = "C:/Users/ooof/Desktop/Cabanon_Server/";
		BASE_PATH = "C:/Users/Pietro_Alberti/Documents/Code/Cabanon_Server/";
	}

	SYNC_RPI_1_CMD_PATH = BASE_PATH + "resources/synchro_RPI_1.cmd";
	SYNC_RPI_2_CMD_PATH = BASE_PATH + "resources/synchro_RPI_2.cmd";
	SYNC_RPI_3_CMD_PATH = BASE_PATH + "resources/synchro_RPI_3.cmd";
	LANDMARK_EXE_PATH = BASE_PATH + "resources/FaceLandmarkImg.exe -wild -fdir " + BASE_PATH + "RPI_sync -ofdir " + BASE_PATH + "landmarks";
	RPI_1_FOLDER_PATH = BASE_PATH + "RPI_1_sync";
	RPI_2_FOLDER_PATH = BASE_PATH + "RPI_2_sync";
	RPI_3_FOLDER_PATH = BASE_PATH + "RPI_3_sync";
	SYNC_FOLDER_PATH = BASE_PATH + "global_sync";
	TO_PRINT_FOLDER_PATH = BASE_PATH + "to_print";
	LANDMARKS_FOLDER_PATH = BASE_PATH + "landmarks";

	BACKUP_RAW_FOLDER_PATH = BASE_PATH + "resources/backup_raw";
	BACKUP_CROPPED_FOLDER_PATH = BASE_PATH + "resources/backup_cropped";
	BACKUP_LANDMARK_FOLDER_PATH = BASE_PATH + "resources/backup_landmarks";

	// Load font for displaying infos on screen
	font.loadFont(BASE_PATH + "resources/GT-Sectra-Display-Medium-Italic.OTF", 90);

	// Setup the background
	ofBackground(0, 0, 0);

	// Setup time variables useful for the timer
	time = float(ofGetElapsedTimeMillis() / 1000.0);
	nextTime = float(ofGetElapsedTimeMillis() / 1000.0) + intervals;

}

//--------------------------------------------------------------
// General

void ofApp::update() {

	// Update time variable
	time = float(ofGetElapsedTimeMillis() / 1000.0);

	// If the user pressed s then run the sync sequence.
	if (syncOnNextFrame) {

		runServerSequence();
		syncOnNextFrame = false;
		nextTime = float(ofGetElapsedTimeMillis() / 1000.0) + intervals;

	}

	// Check if time has arrived to next time tu run the sync sequence
	if (time >= nextTime) {
		
		runServerSequence();
		nextTime = float(ofGetElapsedTimeMillis() / 1000.0) + intervals;

	}

}

void ofApp::draw() {

	if (nextTime - time < 0.1 || syncOnNextFrame) {
		font.drawString("Syncronization...", ofGetWidth() * 0.5 - font.stringWidth("Syncronization...") * 0.5, ofGetHeight() * 0.5 + font.stringHeight("Syncronization...") * 0.5);
	}
	else {
		string txt = "Next sync in " + ofToString(round(nextTime - time)) + " seconds";
		font.drawString(txt, ofGetWidth() * 0.5 - font.stringWidth(txt) * 0.5, ofGetHeight() * 0.5 + font.stringHeight(txt) * 0.5);
	}

}

//--------------------------------------------------------------------------
// Utils

void ofApp::consoleText(string text) {

	cout << endl << "##############################################" << endl << text << endl << "##############################################" << endl;

}

void ofApp::keyReleased(int key) {

	switch (key)
	{
	case 32:
		isFullscreen = !isFullscreen;
		ofSetFullscreen(isFullscreen);
		break;
	case 113:
		ofExit();
		break;
	case 115:
		syncOnNextFrame = true;
		break;
	case 100:
		RPISync();
		//newFiles = getNewFiles(true);
		//for (int i = 0; i < newFiles.size(); i++) {

			//cout << newFiles[i] << endl;

		//}
		break;
	default:
		cout << key << endl;
		break;
	}

}

void ofApp::runServerSequence() {

	// Clear all
	clearFolder(RPI_1_FOLDER_PATH);
	clearFolder(RPI_2_FOLDER_PATH);
	clearFolder(RPI_3_FOLDER_PATH);
	clearFolder(SYNC_FOLDER_PATH);
	clearFolder(LANDMARKS_FOLDER_PATH);
	clearFolder(TO_PRINT_FOLDER_PATH);
	
	// Sync folder from the raspberry pi
	RPISync();

	//Debug -> comment on release
	copyFolder(BASE_PATH + "Backup_RPI_Sync/", RPI_1_FOLDER_PATH + "/");

	copyFolder(RPI_1_FOLDER_PATH + "/", SYNC_FOLDER_PATH + "/");
	clearFolder(RPI_1_FOLDER_PATH);
	copyFolder(RPI_2_FOLDER_PATH, SYNC_FOLDER_PATH);
	clearFolder(RPI_2_FOLDER_PATH);
	copyFolder(RPI_3_FOLDER_PATH, SYNC_FOLDER_PATH);
	clearFolder(RPI_3_FOLDER_PATH);

	// Check for new files
	auto newFiles = getNewFiles(5, true);

	// If new files are detected then run the server sequence.
	if (newFiles.size() > 0) {

		// Get all pathes needed for the sequence.
		vector<string> imagesToCompute;

		for (int i = 0; i < newFiles.size(); i++) {
			string imagePath = SYNC_FOLDER_PATH + "/" + newFiles[i] + ".jpg";
			imagesToCompute.push_back(imagePath);
		}

		// Compute landmarks and get the new files.
		runLandmarksComputation(imagesToCompute);

		consoleText("Cropping and saving faces");
		ofDirectory landmarksFiles(LANDMARKS_FOLDER_PATH);
		landmarksFiles.listDir();

		for (int i = 0; i < landmarksFiles.size(); i++) {
			string fileName = ofSplitString(landmarksFiles.getName(i), "_det_")[0];
			
			// Check if the name is correctly composed and that it exists
			ofFile image(SYNC_FOLDER_PATH + "/" + fileName + ".jpg");
			if (image.exists() && fileName.size() == 13) {

				vector<ofVec2f> parsedLandmarks = parseLandmarksFile(landmarksFiles[i].getAbsolutePath());

				ofImage inputImage;
				inputImage.load(SYNC_FOLDER_PATH + "/" + fileName + ".jpg");

				// Test, crop the face and save it
				if (inputImage.getWidth() > 0 && inputImage.getHeight() > 0) {

					ofImage croppedFace = getCroppedFace(inputImage, parsedLandmarks);
					ofSaveImage(croppedFace.getPixels(), TO_PRINT_FOLDER_PATH + "/" + fileName + ".png", OF_IMAGE_QUALITY_BEST);
					cout << fileName << " -> cropped and saved" << endl;

				}
				else {

					cout << fileName << " is corrupted" << endl;

				}

			}
			else {

				cout << image.path() << " doesn't exists" << endl;

			}
		}

		// Print all the cropped faces
		print();

	}
	else {

		consoleText("No new faces");

	}

	// Backup all files
	copyFolder(SYNC_FOLDER_PATH, BACKUP_RAW_FOLDER_PATH);
	copyFolder(TO_PRINT_FOLDER_PATH, BACKUP_CROPPED_FOLDER_PATH);
	copyFolder(LANDMARKS_FOLDER_PATH, BACKUP_LANDMARK_FOLDER_PATH);

	// Remove all files
	clearFolder(SYNC_FOLDER_PATH);
	clearFolder(LANDMARKS_FOLDER_PATH);
	clearFolder(TO_PRINT_FOLDER_PATH);

	consoleText("Done");
	consoleText("Wait " + ofToString(intervals) + " seconds");

}


//--------------------------------------------------------------------------
// Server
void ofApp::RPISync() {

	consoleText("RPI Sync");

	cout << ofSystem(SYNC_RPI_1_CMD_PATH) << endl;
	cout << ofSystem(SYNC_RPI_2_CMD_PATH) << endl;
	cout << ofSystem(SYNC_RPI_3_CMD_PATH) << endl;

}

vector<string> ofApp::getNewFiles(int maxNewFiles, bool removeOldFiles) {

	vector<string> newFiles;
	ofDirectory syncFolder(SYNC_FOLDER_PATH);
	syncFolder.allowExt("jpg");
	syncFolder.listDir();

	// Check if in the sync folder there are files that have already been processed.
	int numNewFilesFound = 0;
	for (int i = 0; i < syncFolder.size(); i++) {

		string syncFileName = ofSplitString(syncFolder.getName(i), ".")[0];
		bool foundSameImageName = false;

		for (int j = 0; j < cachedFiles.size(); j++) {

			string cachedFileName = cachedFiles[j];

			if (cachedFileName == syncFileName) {
				foundSameImageName = true;
				break;
			}

		}

		if (!foundSameImageName && numNewFilesFound < maxNewFiles) {
			newFiles.push_back(syncFileName);
			cachedFiles.push_back(syncFileName);
			numNewFilesFound++;
		}
		else if (foundSameImageName && removeOldFiles) {
			syncFolder[i].remove();
		}

	}

	consoleText(ofToString(newFiles.size()) + " new files detected");

	return newFiles;
		
}

void ofApp::runLandmarksComputation(vector<string> imagesPathes) {

	consoleText("Computing landmarks...");

	string EXE_CMD = BASE_PATH + "resources/FaceLandmarkImg.exe - wild";

	for (int i = 0; i < imagesPathes.size(); i++) {
		EXE_CMD += " -f " + imagesPathes[i];
	}

	EXE_CMD += " -ofdir " + BASE_PATH + "landmarks";
	cout << ofSystem(EXE_CMD) << endl;

}

vector<ofVec2f> ofApp::parseLandmarksFile(string filePath) {

	vector<ofVec2f> parsedLandmarksFile;
	ifstream file; //declare a file stream  
	file.open(ofToDataPath(filePath).c_str()); //open your text file

	int lineIndex = 0;
	string line;
	int nPoints = 0;

	while (getline(file, line)) //as long as theres still text to be read  
	{

		if (lineIndex == 1) {

			nPoints = std::stoi(ofSplitString(line, ": ")[1]);

		}
		else if (lineIndex > 2 && lineIndex <= 2 + nPoints) {

			parsedLandmarksFile.push_back(ofVec2f(std::stof(ofSplitString(line, " ")[0]), std::stof(ofSplitString(line, " ")[1])));

		}

		lineIndex++;

	}

	file.close();

	return parsedLandmarksFile;

}

ofImage ofApp::getCroppedFace(ofImage inputImage, vector<ofVec2f> landmarks)
{
	
	ofEnableAlphaBlending();
	ofDrawCircle(30, 30, 20);

	vector<ofVec2f> facialContour = getFacialContour(landmarks);
	vector<ofVec2f> boundingBox = getBoundingBox(facialContour);
	ofVec2f topLeft = boundingBox[0];
	ofVec2f dimensions = ofVec2f(boundingBox[1].x - boundingBox[0].x, boundingBox[1].y - boundingBox[0].y);

	// Polyline & mask

	ofFbo fbo;
	ofPolyline faceContour;

	fbo.allocate(inputImage.getWidth(), inputImage.getHeight());
	fbo.begin();
	ofClear(0, 0, 0, 0);

	inputImage.draw(0, 0, inputImage.getWidth(), inputImage.getHeight());

	// Draw landmarks for debug

	ofVec2f bottomRight = boundingBox[1];
	ofVec2f first = facialContour[0];
	ofVec2f mid = facialContour[facialContour.size() - 1];
	ofVec2f last = facialContour[facialContour.size() - 2];
	ofVec2f dir = (last - first).getNormalized();
	ofVec2f perp = (last - first).getPerpendicular();

	for (int j = 0; j < facialContour.size() - 1; j++) {

		faceContour.addVertex(facialContour[j].x, facialContour[j].y);

	}

	float deltaY = abs(topLeft.y - bottomRight.y);
	ofVec2f c1 = last - perp * deltaY * 0.1;
	ofVec2f c2 = mid + dir * deltaY * 0.4;
	faceContour.bezierTo(c1.x, c1.y, c2.x, c2.y, mid.x, mid.y);

	ofVec2f c3 = mid - dir * deltaY * 0.4;
	ofVec2f c4 = first - perp * deltaY * 0.1;
	faceContour.bezierTo(c3.x, c3.y, c4.x, c4.y, first.x, first.y);

	faceContour.close();

	fbo.end();

	ofPixels pixels;
	fbo.readToPixels(pixels);

	ofDrawCircle(inputImage.getWidth() * 0.5, inputImage.getHeight() * 0.5, 10);

	ofPixels imagePixels = inputImage.getPixels();
	ofImage newImage;
	newImage.allocate(ceil(dimensions.x), ceil(dimensions.y), OF_IMAGE_COLOR_ALPHA);

	for (int j = 0; j < dimensions.x; j++) {

		for (int k = 0; k < dimensions.y; k++) {

			if (faceContour.inside(j + topLeft.x, k + topLeft.y)) newImage.setColor(j, k, pixels.getColor(j + topLeft.x, k + topLeft.y));
			else
			{
				newImage.setColor(j, k, ofColor(0, 0, 0, 0));
			}

		}

	}

	return newImage;

}

void ofApp::print() {

	consoleText("Printing cropped faces...");
	cout << ofSystem(BASE_PATH + "/resources/psd_script.vbs") << endl;

}

void ofApp::clearFolder(string folderPath) {

	ofDirectory dir(folderPath);
	dir.listDir();

	for (int i = 0; i < dir.size(); i++) {

		dir[i].remove();

	}

	consoleText(folderPath + " CLEARED!");

}

void ofApp::copyFolder(string inputFolderPath, string outputFolderPath) {

	ofDirectory inputDir(inputFolderPath);
	inputDir.listDir();

	for (int i = 0; i < inputDir.size(); i++) {
		inputDir.getFile(i).copyTo(outputFolderPath, false, true);
	}

	consoleText(inputFolderPath + " COPIED IN: " + outputFolderPath);

}


//--------------------------------------------------------------------------
// Server utils
vector<ofVec2f> ofApp::getFacialContour(vector<ofVec2f> landmarks) {

	vector<ofVec2f> facialContour;

	for (int i = 0; i <= 16; i++) {

		facialContour.push_back(landmarks[i]);

	}

	// Get bounding box

	auto boundingBox = getBoundingBox(landmarks);

	// Crate upper landmarks

	ofVec2f first = landmarks[0];
	ofVec2f last = landmarks[16];

	ofVec2f dir = first - last;
	float length = dir.length();
	ofVec2f perp = dir.getPerpendicular();

	ofVec2f midPoint = first + perp * (first.y - boundingBox[0].y) * 1.9 - dir * 0.5;

	int smoothingValue = 10;
	vector<ofVec2f> leftPoints;

	facialContour.push_back(midPoint);

	return facialContour;

}

vector<ofVec2f> ofApp::getBoundingBox(vector<ofVec2f> landmarks) {

	ofVec2f topLeft = ofVec2f(INFINITE, INFINITE);
	ofVec2f	bottomRight = ofVec2f(0, 0);

	for (int i = 0; i < landmarks.size(); i++) {

		if (landmarks[i].x < topLeft.x) topLeft.x = landmarks[i].x;
		if (landmarks[i].y < topLeft.y) topLeft.y = landmarks[i].y;
		if (landmarks[i].x > bottomRight.x) bottomRight.x = landmarks[i].x;
		if (landmarks[i].y > bottomRight.y) bottomRight.y = landmarks[i].y;

	}

	vector<ofVec2f> boundingBox;
	boundingBox.push_back(topLeft);
	boundingBox.push_back(bottomRight);

	return boundingBox;

}
