#include "opencv2/core/utility.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/optflow.hpp"
#include "opencv2/aruco.hpp"

#include "aruco/aruco.h"
#include "aruco/highlyreliablemarkers.h"


class openCVStuff
{
public:
	openCVStuff()
		: m_algorithm(createOptFlow_DIS(cv::optflow::DISOpticalFlow::PRESET_MEDIUM))
	{}
	~openCVStuff() {};

	cv::Mat getRVec()
	{
		return rotation_vector;
	}

	cv::Mat getTVec()
	{
		return translation_vector;
	}

	void doOpticalFlow() {
		if (m_grey0.empty())
		{
			m_grey1.copyTo(m_grey0);
		}

		//cv::imshow("g0", m_grey0);
		//cv::imshow("g1", m_grey1);

		m_algorithm->calc(m_grey0, m_grey1, m_flow);

		


	}

	void setColorMat(float* colorArray)
	{
		m_colorImage = cv::Mat(1080, 1920, CV_8UC4, colorArray);
	}

	//void setInfraMat(float* infraArray, float &irBrightness)
	//{
	//	//std::cout << irBrightness << std::endl;
	//	cv::Mat tIR = cv::Mat(424, 512, CV_32FC1, infraArray);
	//	tIR.convertTo(m_infraImage, CV_8UC1, 1.0f / (irBrightness) );
	//	cv::imshow("irsss", m_infraImage);
	//}

	void setInfraMat(cv::Mat ir)
	{
		ir.copyTo(m_infraImage);
	}

	void resetColorPoints()
	{
	
	}

	void resetDepthPoints()
	{

	}

	void findCheckerBoard()
	{
		//bool findChessboardCorners(InputArray image, Size patternSize, OutputArray corners, int flags=CALIB_CB_ADAPTIVE_THRESH+CALIB_CB_NORMALIZE_IMAGE )
		// find in color image
		cv::Mat pDownCol0;
		cv::Mat pDownCol1;
		cv::pyrDown(m_colorImage, pDownCol0);
		cv::pyrDown(pDownCol0, pDownCol1);

		std::vector<cv::Point2f> pDownDetectedPoints;
		bool found = findChessboardCorners(pDownCol1, cv::Size(7, 5), pDownDetectedPoints);
		m_detectedPointsColor.resize(pDownDetectedPoints.size());
		for (int i = 0; i < pDownDetectedPoints.size(); i++)
		{
			m_detectedPointsColor[i].x = pDownDetectedPoints[i].x * 4.0f;
			m_detectedPointsColor[i].y = pDownDetectedPoints[i].y * 4.0f;

		}
		// and ir image
		findChessboardCorners(m_infraImage, cv::Size(7, 5), m_detectedPointsInfra);

		//cv::drawChessboardCorners(m_colorImage, cv::Size(5, 7), m_detectedPointsColor, found);
		//cv::imshow("irss", m_colorImage);

		//std::cout << "found points " << m_detectedPointsInfra << std::endl;

	}

	void addCurrentColorImage()
	{
		// save image to disk for further viewing if needed
		std::stringstream ss;
		ss << "./data/calib/color_" << nColorImageSaved << ".png";
		std::string fName = ss.str();
		saveImage(m_colorImage, fName);
		// add image to vector of cv::Mat
		calibrationImagesColor.push_back(m_colorImage);
		nColorImageSaved++;



	}

	void addCurrentInfraImage()
	{
		std::stringstream ss;
		ss << "./data/calib/infra_" << nInfraImageSaved << ".png";
		std::string fName = ss.str();
		saveImage(m_infraImage, fName);
		cv::Mat tMat;
		m_infraImage.copyTo(tMat);
		calibrationImagesInfra.push_back(tMat);
		nInfraImageSaved++;

	}

	void saveCalibrationResults(const std::string &outFile, const cv::Size &imageSize, const cv::Mat &cameraMatrix, const cv::Mat &distCoeffs, double rpError)
	{
		cv::FileStorage fs(outFile, cv::FileStorage::WRITE);
		if (fs.isOpened())
		{
			fs
				<< "image_width" << imageSize.width
				<< "image_height" << imageSize.height
				<< "camera_matrix" << cameraMatrix
				<< "distortion_coefficients" << distCoeffs
				<< "reprojection_error" << rpError
				;

			fs.release();
			std::cout << "Calibration parameters saved to " << outFile << std::endl;
		}
		else
		{
			std::cerr << "Cannot open output file " << outFile << std::endl;
		}
	}

	void calibrateImagesColor()
	{
		cv::Size frameSize = cv::Size(1920.0f, 1080.0f);
		std::vector<std::vector<cv::Point3f>> objectPoints;
		std::vector<std::vector<cv::Point2f>> imagePoints;

		std::vector<cv::Point3f> vertices;

		for (int h = 0; h < m_boardSize.height; h++)
		{
			for (int w = 0; w < m_boardSize.width; w++)
			{
				vertices.push_back(cv::Point3f(h * m_squareSize, w * m_squareSize, 0.0f));
			}
		}

		cv::Mat cameraMatrix, distCoeffs;

		std::cout << "calibrating " << calibrationImagesColor.size() << " RGB images " << std::endl;

		//using fullsize images for better qaultiy than the live rendering ones
		for (int i = 0; i < calibrationImagesColor.size(); i++)
		{
			std::cout << "calibrating image " << i << std::endl;

			std::vector<cv::Point2f> corners;
			bool found = cv::findChessboardCorners(calibrationImagesColor[i], cv::Size(7, 5), corners);

			if (found)
			{

				cv::Mat viewGray;
				cv::cvtColor(calibrationImagesColor[i], viewGray, CV_BGRA2GRAY);

				cv::cornerSubPix(viewGray, corners, cv::Size(11, 11),
					cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

				imagePoints.push_back(corners);
				objectPoints.push_back(vertices);

				std::stringstream ss;
				ss << "./data/calib/color_detected" << nColorCalibrationImageSaved << ".png";
				std::string fName = ss.str();
				saveImage(m_colorImage, fName);
				nColorCalibrationImageSaved++;

			}

		}

		if (imagePoints.empty())
		{
			std::cout << "No valid calibration samples. Calibration aborted." << std::endl;
			exit(EXIT_FAILURE);
		}

		std::vector<cv::Mat> rVecs, tVecs;
		cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
		std::cout << "Start calibration using " << imagePoints.size() << " samples...";
		double rpError = cv::calibrateCamera(
			objectPoints, imagePoints, frameSize,
			cameraMatrix, distCoeffs,
			rVecs, tVecs,
			// CV_CALIB_USE_INTRINSIC_GUESS | CV_CALIB_FIX_PRINCIPAL_POINT | CV_CALIB_FIX_ASPECT_RATIO
			CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5 | CV_CALIB_FIX_K6,	// set them to 0
			cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, DBL_EPSILON)
		);
		std::cout << "done. Re-projection error: " << rpError << std::endl;

		string outFile = "./data/calib/colorCalibration.txt";
		saveCalibrationResults(outFile, frameSize, cameraMatrix, distCoeffs, rpError);
		
	}

	void calibrateImagesInfra()
	{
		cv::Size frameSize = cv::Size(512.0f, 424.0f);
		std::vector<std::vector<cv::Point3f>> objectPoints;
		std::vector<std::vector<cv::Point2f>> imagePoints;

		std::vector<cv::Point3f> vertices;

		for (int h = 0; h < m_boardSize.height; h++)
		{
			for (int w = 0; w < m_boardSize.width; w++)
			{
				vertices.push_back(cv::Point3f(h * m_squareSize, w * m_squareSize, 0.0f));
			}
		}

		cv::Mat cameraMatrix, distCoeffs;

		//using fullsize images for better qaultiy than the live rendering ones
		std::cout << "calibrating " << calibrationImagesInfra.size() << " IR images " << std::endl;
		for (int i = 0; i < calibrationImagesInfra.size(); i++)
		{
			std::vector<cv::Point2f> corners;
			bool found = cv::findChessboardCorners(calibrationImagesInfra[i], cv::Size(7, 5), corners);
			std::cout << "calibrating image " << i << std::endl;

			if (found)
			{

				cv::Mat viewGray;
				calibrationImagesInfra[i].copyTo(viewGray);
				//cv::cvtColor(calibrationImagesInfra[i], viewGray, CV_BGRA2GRAY);

				cv::cornerSubPix(viewGray, corners, cv::Size(11, 11),
					cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

				imagePoints.push_back(corners);
				objectPoints.push_back(vertices);

				cv::drawChessboardCorners(viewGray, cv::Size(7, 5), corners, found);


				std::stringstream ss;
				ss << "./data/calib/infra_detected" << nInfraCalibrationImageSaved << ".png";
				std::string fName = ss.str();
				saveImage(viewGray, fName);
				nInfraCalibrationImageSaved++;

			}

		}

		if (imagePoints.empty())
		{
			std::cerr << "No valid calibration samples. Calibration aborted." << std::endl;
			//exit(EXIT_FAILURE);
		}

		std::vector<cv::Mat> rVecs, tVecs;
		cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
		std::cout << "Start calibration using " << imagePoints.size() << " samples...";
		double rpError = cv::calibrateCamera(
			objectPoints, imagePoints, frameSize,
			cameraMatrix, distCoeffs,
			rVecs, tVecs,
			// CV_CALIB_USE_INTRINSIC_GUESS | CV_CALIB_FIX_PRINCIPAL_POINT | CV_CALIB_FIX_ASPECT_RATIO
			CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5 | CV_CALIB_FIX_K6,	// set them to 0
			cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, DBL_EPSILON)
		);
		std::cout << "done. Re-projection error: " << rpError << std::endl;

		string outFile = "./data/calib/infraCalibration.txt";
		saveCalibrationResults(outFile, frameSize, cameraMatrix, distCoeffs, rpError);

	}

	std::vector<cv::Point2f> getCheckerBoardPointsColor()
	{
		return m_detectedPointsColor;
	}

	std::vector<cv::Point2f> getCheckerBoardPointsInfra()
	{
		return m_detectedPointsInfra;
	}

	// 
	cv::Mat getDepthToColorMatrix(std::vector<cv::Point3f> depthPoints, std::vector<cv::Point2f> colorPoints)
	{
		cv::Mat depthToColorMatrix = cv::Mat::eye(4,4,CV_32F);

		cv::Mat rvec, tvec;
		// Camera internals
		cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<float>::type); // Assuming no lens distortion

		//cout << "Camera Matrix " << endl << camera_matrix << endl;
		// Output rotation and translation


		// Solve for pose
		cv::solvePnP(depthPoints, colorPoints, colorCamPams, dist_coeffs, rvec, tvec);

		cv::Mat rod;
		cv::Rodrigues(rvec, rod);

		//std::cout << rod << std::endl;

		cv::Mat tmp = depthToColorMatrix(cv::Rect(0, 0, 3, 3));
		rod.copyTo(tmp);

		depthToColorMatrix.at<float>(0, 3) = tvec.at<float>(0);
		depthToColorMatrix.at<float>(1, 3) = tvec.at<float>(1);
		depthToColorMatrix.at<float>(2, 3) = tvec.at<float>(2);

		std::cout << depthToColorMatrix << std::endl;


		return depthToColorMatrix;
	}

	void detectMarkersInfra(cv::Mat inputImage)
	{
		cv::Mat greyColor;

		cv::cvtColor(inputImage, greyColor, CV_RGBA2RGB);

		m_infraCamPams.at<float>(0, 0) = 364.7546f;
		m_infraCamPams.at<float>(1, 1) = 365.5064f;
		m_infraCamPams.at<float>(0, 2) = 254.0044f;
		m_infraCamPams.at<float>(1, 2) = 200.9755f;

		m_infraCamDist.at<float>(0, 0) = 0.0900f;
		m_infraCamDist.at<float>(1, 0) = -0.2460f;
		m_infraCamDist.at<float>(2, 0) = 0.0018f;
		m_infraCamDist.at<float>(3, 0) = 0.0017f;


		cv::Mat inputImageFlip;
		cv::flip(greyColor, inputImageFlip, 1);
		std::vector<int> markerIds;
		std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;

		camParamsInfra.CameraMatrix = colorCamPams;
		camParamsInfra.Distorsion = colorCamDist;
		camParamsInfra.CamSize = cv::Size(512, 424); // change this if you use full resolution, or just code it properly you idito
		markerDetector.detect(inputImageFlip, detectedMarkers, camParamsInfra, markerSize);




		cv::Mat outputImage = inputImageFlip;
		for (vector<aruco::Marker>::iterator it = detectedMarkers.begin(); it != detectedMarkers.end(); it++)
		{
			aruco::Marker marker = *it;
			aruco::CvDrawingUtils::draw3dAxis(outputImage, marker, camParamsInfra);

		}
		//cv::aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);
		//cv::imshow("oC", outputImage);
	}
	void detectMarkersColor(cv::Mat inputImage)
	{
		cv::Mat greyColor;

		cv::cvtColor(inputImage, greyColor, CV_RGBA2RGB);

		colorCamPams.at<float>(0, 0) = 992.2276f;
		colorCamPams.at<float>(1, 1) = 990.6481f;
		colorCamPams.at<float>(0, 2) = (1920.0f - 940.4056f);
		colorCamPams.at<float>(1, 2) = (1080.0f - 546.1401f);

		colorCamDist.at<float>(0, 0) = 0.002308769f;
		colorCamDist.at<float>(1, 0) = -0.03766959f;
		colorCamDist.at<float>(2, 0) = -0.0063f;
		colorCamDist.at<float>(3, 0) = 0.0036f;

		cv::Mat inputImageFlip;
		cv::flip(greyColor, inputImageFlip, 1);
		std::vector<int> markerIds;
		std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;

		camParamsColor.CameraMatrix = colorCamPams;
		camParamsColor.Distorsion = colorCamDist;
		camParamsColor.CamSize = cv::Size(1920, 1080); // change this if you use full resolution, or just code it properly you idito
		markerDetectorColor.detect(inputImageFlip, detectedMarkersColor, camParamsColor, markerSize);




		cv::Mat outputImage = inputImageFlip;
		for (vector<aruco::Marker>::iterator it = detectedMarkersColor.begin(); it != detectedMarkersColor.end(); it++)
		{
			aruco::Marker marker = *it;
			aruco::CvDrawingUtils::draw3dAxis(outputImage, marker, camParamsColor);

		}
		//cv::aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);
		//cv::imshow("oI", outputImage);
	}



	void displayFlow()
	{
		cv::split(m_flow, m_flow_uv);
		cv::multiply(m_flow_uv[1], 1, m_flow_uv[1]);
		cv::cartToPolar(m_flow_uv[0], m_flow_uv[1], m_mag, m_ang, true);

		cv::Mat mag = cv::Mat(424, 512, CV_32FC1);
		cv::Mat ang = cv::Mat(424, 512, CV_32FC1);

		int angInDegree = 1; // get ang in degrees

		for (int i = 0; i < 424; i++)
		{
			for (int j = 0; j < 512; j++)
			{
				float x = m_flow_uv[0].at<float>(i, j);
				float y = m_flow_uv[1].at<float>(i, j);
				float x2 = x * x;
				float y2 = y * y;
				
				mag.at<float>(i, j) = sqrt(x2 + y2);
				
				float cartToPolar;

				float tmp = y >= 0 ? 0 : CV_PI * 2;
				tmp = x < 0 ? CV_PI : tmp;

				float tmp1 = y >= 0 ? CV_PI*0.5 : CV_PI*1.5;
				cartToPolar = y2 <= x2 ? x*y / (x2 + 0.28f*y2 + (float)DBL_EPSILON) + tmp :
					tmp1 - x*y / (y2 + 0.28f*x2 + (float)DBL_EPSILON);

				cartToPolar = angInDegree == 0 ? cartToPolar : cartToPolar * (float)(180 / CV_PI);

				ang.at<float>(i, j) = cartToPolar;
			}
		}

		// we are good so far
		cv::imshow("diff of mag", 1000.0f * (m_mag - mag));
		// cv one outputs in degrees
		cv::imshow("diff of ang", (m_ang - ang) / 1000.0f);




		//cv::threshold(m_mag, m_magThresh, 0, 255, CV_THRESH_TOZERO);
		cv::normalize(m_mag, m_mag, 0, 1, cv::NORM_MINMAX);

		m_hsv_split[0] = m_ang;
		m_hsv_split[1] = m_mag;
		m_hsv_split[2] = cv::Mat::ones(m_ang.size(), m_ang.type());

		cv::merge(m_hsv_split, 3, m_hsv);
		cv::cvtColor(m_hsv, m_rgb, cv::COLOR_HSV2RGB);
		//cv::imshow("g0", m_grey0);
		//cv::imshow("g1", m_grey1);
		cv::imshow("diff", (m_grey0 - m_grey1) * 10);
		cv::imshow("flow1", m_rgb);
	}

	cv::Mat getRGBFlow()
	{
		return m_rgb;
	}

	void setCurrentFrame()
	{
		m_infraImage.copyTo(m_grey1);
	}
	void setPreviousFrame(cv::Mat grey0)
	{
		grey0.copyTo(m_grey0);
	}
	void swapFrames()
	{
		cv::swap(m_grey0, m_grey1);
	}
	cv::Mat getFlow()
	{
		return m_flow;
	}
	cv::Mat getFlowAng()
	{
		return m_ang;
	}
	cv::Mat getFlowMag()
	{
		return m_mag;
	}
	bool setupAruco()
	{
		// README SINCE YOU FORGOT AGAIN, FOOL. just set the aruco130d.dll rather than the releasse one?


		cv::FileStorage fs(dictFile, cv::FileStorage::READ);
		int nmarkers, markersize;
		fs["nmarkers"] >> nmarkers;                     // 
		fs["markersize"] >> markersize;                 // 
		fs.release();

		if (!markerDict.fromFile(dictFile))
		{
			cerr << "cannot open dictionary file " << dictFile << endl;
			return false;
		}
		if (markerDict.empty())
		{
			cerr << "marker dictionary file is empty" << endl;
			return false;
		}

		aruco::HighlyReliableMarkers::loadDictionary(markerDict);
		cout << "Dictionary Loaded" << endl;

		//for Color
		markerDetectorColor.setMakerDetectorFunction(aruco::HighlyReliableMarkers::detect);
		markerDetectorColor.setThresholdParams(25, 7);
		//markerDetector.setThresholdMethod(aruco::MarkerDetector::ADPT_THRES);
		markerDetectorColor.setCornerRefinementMethod(aruco::MarkerDetector::LINES);
		markerDetectorColor.setWarpSize((markerDict[0].n() + 2) * 8);
		markerDetectorColor.setMinMaxSize(0.005f, 0.5f);

		//for IR
		markerDetector.setMakerDetectorFunction(aruco::HighlyReliableMarkers::detect);
		markerDetector.setThresholdParams(25, 7);
		//markerDetector.setThresholdMethod(aruco::MarkerDetector::ADPT_THRES);
		markerDetector.setCornerRefinementMethod(aruco::MarkerDetector::LINES);
		markerDetector.setWarpSize((markerDict[0].n() + 2) * 8);
		markerDetector.setMinMaxSize(0.005f, 0.5f);
		//markerDetector.setDesiredSpeed(3);
		//cout << "detector configured" << endl;



	}
	
	void setInfraCamPams(cv::Mat irCP) 
	{
		m_infraCamPams = irCP;
	}

	void setInfraCamDist(cv::Mat irCD)
	{
		m_infraCamDist = irCD;
	}



	void saveImage(cv::Mat image, string fileName)
	{

		vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(0);


		try {
			cv::imwrite(fileName, image, compression_params);
		}
		catch (runtime_error& ex) {
			fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
		}

	}

	void saveImage(int type)
	{
		if (type == 0) // color image
		{
			nColorImageSaved++;
			std::stringstream ss;
			ss << "./data/calib/color_" << nColorImageSaved << ".png";
			std::string fName = ss.str();
			saveImage(m_colorImage, fName);
			saveImage(m_colorImage, fName);
		}

		if (type == 1) // infra image
		{
			nInfraImageSaved++;
			std::stringstream ss;
			ss << "./data/calib/infra_" << nInfraImageSaved << ".png";
			std::string fName = ss.str();
			saveImage(m_infraImage, fName);
			saveImage(m_colorImage, fName);
		}
	}



private:

	cv::Ptr<cv::DenseOpticalFlow> m_algorithm;
	cv::Mat m_colorImage;
	cv::Mat m_infraImage;
	//cv::Mat bigFlow;
	cv::Mat m_grey0;
	cv::Mat m_grey1;

	cv::Mat m_flow;
	cv::Mat m_flow_uv[2];
	cv::Mat m_mag, m_magThresh, m_ang;
	cv::Mat m_hsv_split[3], m_hsv;
	cv::Mat m_rgb = cv::Mat(424,512, CV_8UC3);

	std::vector<cv::Mat> calibrationImagesColor;
	std::vector<cv::Mat> calibrationImagesInfra;


	std::string dictFile = "resources/dodec.yml";
	aruco::Dictionary markerDict;
	aruco::MarkerDetector markerDetector, markerDetectorColor;
	vector<aruco::Marker> detectedMarkers;
	vector<aruco::Marker> detectedMarkersColor;
	aruco::CameraParameters camParamsColor, camParamsInfra;
	float markerSize = 0.0358f;
	cv::Mat colorCamPams = cv::Mat::eye(3, 3, CV_32F);
	cv::Mat colorCamDist = cv::Mat(5, 1, CV_32F);

	cv::Mat m_infraCamPams = cv::Mat::eye(3, 3, CV_32F);
	cv::Mat m_infraCamDist = cv::Mat(5, 1, CV_32F);

	cv::Mat rotation_vector; // Rotation in axis-angle form
	cv::Mat translation_vector;

	// x is x coord, y is y coord
	std::vector<cv::Point2f> m_detectedPointsColor;
	std::vector<cv::Point2f> m_detectedPointsInfra;

	cv::Size m_boardSize = cv::Size(7, 5);
	float m_squareSize = 0.036; // 36 mm


	int nColorImageSaved = 0;
	int nInfraImageSaved = 0;

	int nColorCalibrationImageSaved = 0;
	int nInfraCalibrationImageSaved = 0;

};






