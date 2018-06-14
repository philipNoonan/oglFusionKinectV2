#include "kinect.h"

#define USEINFRARED

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

void kRenderInit()
{
	krender.SetCallbackFunctions();
	krender.compileAndLinkShader();
	// Set locations
	krender.setLocations();
	krender.setVertPositions();
	krender.allocateBuffers();
	krender.setTextures(gfusion.getDepthImage(), gdisoptflow.getColorTexture(), gfusion.getVerts(), gfusion.getNorms(), gfusion.getVolume(), gfusion.getTrackImage()); //needs texture uints from gfusion init
	krender.anchorMW(std::make_pair<int, int>(1920 - 512 - krender.guiPadding().first, krender.guiPadding().second));
	//krender.genTexCoordOffsets(1, 1, 1.0f);
}

void gFusionInit()
{
	gfusion.queryDeviceLimits();
	gfusion.compileAndLinkShader();
	gfusion.setLocations();

	gconfig.volumeSize = glm::vec3(128);
	gconfig.volumeDimensions = glm::vec3(1.0f);
	gconfig.mu = 0.05f;
	gconfig.maxWeight = 100.0f;
	gconfig.iterations[0] = 2;
	gconfig.iterations[1] = 4;
	gconfig.iterations[2] = 10;

	glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));

	gfusion.setConfig(gconfig);
	gfusion.setPose(initPose);

	gfusion.initTextures();
	gfusion.initVolume();
	gfusion.allocateBuffers();

}

void mCubeInit()
{
	//mcconfig.gridSize = gconfig.volumeSize;
	/*mcconfig.gridSizeMask = glm::uvec3(mcconfig.gridSize.x - 1, mcconfig.gridSize.y - 1, mcconfig.gridSize.z - 1);
	mcconfig.gridSizeShift = glm::uvec3(0, log2(mcconfig.gridSize.x), log2(mcconfig.gridSize.y) + log2(mcconfig.gridSize.z));
	mcconfig.numVoxels = mcconfig.gridSize.x * mcconfig.gridSize.y * mcconfig.gridSize.z;
	mcconfig.voxelSize = glm::vec3(gconfig.volumeDimensions.x * 1000.0f / gconfig.volumeSize.x, gconfig.volumeDimensions.y * 1000.0f / gconfig.volumeSize.y, gconfig.volumeDimensions.z * 1000.0f / gconfig.volumeSize.z);
	mcconfig.maxVerts = std::min(mcconfig.gridSize.x * mcconfig.gridSize.y * 128, uint32_t(128 * 128 * 128));

	gfusion.setMcConfig(mcconfig);*/

}

std::vector<float> splitStringToVector(const char * xmlString)
{
	std::string strOutMat = xmlString;
	// this purges commas if present in the xml
	strOutMat.erase(std::remove(strOutMat.begin(), strOutMat.end(), ','), strOutMat.end());

	std::istringstream ss(strOutMat);
	float val;
	int idx = 0;

	std::vector<float> res;
	while (ss >> val)
	{
		res.push_back(val);
	}
	return res;
}

// this uses tinyxml rather than opencv xml reader
// implement a yaml and json reader too
// this currently works for opencv style xml where there is just whitepace separating the values
int loadCalibration(std::string calibFile)
{
	camPams newCamPams;

	if (std::filesystem::path(calibFile).extension() == ".xml")
	{
		// IF XML
		tinyxml2::XMLError eResult = calibrationXML.LoadFile(calibFile.c_str());

		tinyxml2::XMLNode * pRootCM = calibrationXML.FirstChildElement("Camera_Matrix");
		if (pRootCM == nullptr) return tinyxml2::XML_ERROR_FILE_READ_ERROR;
		tinyxml2::XMLElement * pElement = pRootCM->FirstChildElement("data");
		if (pElement == nullptr) return tinyxml2::XML_ERROR_PARSING_ELEMENT;
		const char * cameraString = pElement->GetText();

		std::vector<float> resCM = splitStringToVector(cameraString);

		tinyxml2::XMLNode * pRootDi = calibrationXML.FirstChildElement("Distortion_Coefficients");
		if (pRootDi == nullptr) return tinyxml2::XML_ERROR_FILE_READ_ERROR;
		tinyxml2::XMLElement * pElementDi = pRootDi->FirstChildElement("data");
		if (pElementDi == nullptr) return tinyxml2::XML_ERROR_PARSING_ELEMENT;
		const char * distortionString = pElementDi->GetText();

		std::vector<float> resDi = splitStringToVector(distortionString);

		newCamPams.fx = resCM[0];
		newCamPams.fy = resCM[4];
		newCamPams.ppx = resCM[2];
		newCamPams.ppy = resCM[5];

		// catch if crash here due to input xml not being correctly read in for the 5 values
		newCamPams.k1 = resDi[0];
		newCamPams.k2 = resDi[1];
		newCamPams.p1 = resDi[2];
		newCamPams.p2 = resDi[3];
		newCamPams.k3 = resDi[4];
	}
	else if (std::filesystem::path(calibFile).extension() == ".json")
	{
		kcamera.setDepthCamPams(newCamPams);

		// IF JSON
		std::ifstream i("resources/infrared.json");
		nlohmann::json j;
		i >> j;

		nlohmann::json dataCM = j["camera_matrix"];
		std::vector<std::vector<float>> tDataCM;

		for (auto it = dataCM.begin(); it != dataCM.end(); ++it)
		{
			tDataCM.push_back(*it);
		}

		nlohmann::json dataDis = j["dist_coeff"];
		std::vector<float> tDataDis;

		for (auto it = dataDis.begin(); it != dataDis.end(); ++it)
		{
			tDataDis.push_back(*it);
		}

		newCamPams.fx = tDataCM[0][0];
		newCamPams.fy = tDataCM[1][1];
		newCamPams.ppx = tDataCM[0][2];
		newCamPams.ppy = tDataCM[1][2];

		// catch if crash here due to input xml not being correctly read in for the 5 values
		newCamPams.k1 = tDataDis[0];
		newCamPams.k2 = tDataDis[1];
		newCamPams.p1 = tDataDis[2];
		newCamPams.p2 = tDataDis[3];
		newCamPams.k3 = tDataDis[4];
	}
	else
	{
		std::cout << "please use a .xml or .json file for loading calibration data" << std::endl;
		return 0;
	}
	
	kcamera.setDepthCamPams(newCamPams);
	
	// YAML as well?
	
	return 1;
}

void startKinect()
{
	kcamera.start();

	camPams currentCamPams = kcamera.getDepthCamPams();
	krender.setCameraParams(glm::vec4(currentCamPams.fx, currentCamPams.fx, currentCamPams.ppx, currentCamPams.ppy), glm::vec4(kcamera.fx_col(), kcamera.fx_col(), kcamera.ppx_col(), kcamera.ppy_col())); // FIX ME
	gfusion.setCameraParams(glm::vec4(currentCamPams.fx, currentCamPams.fx, currentCamPams.ppx, currentCamPams.ppy), glm::vec4(kcamera.fx_col(), kcamera.fx_col(), kcamera.ppx_col(), kcamera.ppy_col())); // FIX ME



	while (!kcamera.ready())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void setImguiWindows()
{
	int width = 1920;
	int height = 1080;
	int topBarHeight = 128;

	navigationWindow.set(0, topBarHeight, controlPoint0.x, height - topBarHeight, true, true, "navi");

	graphWindow.set(controlPoint0.x, controlPoint0.y, width - controlPoint0.x, height - controlPoint0.y, true, true, "graphs"); // 420 is its height

	display2DWindow.set(controlPoint0.x, topBarHeight, (width - controlPoint0.x) / 2, controlPoint0.y - topBarHeight, true, true, "2d");

	display3DWindow.set(controlPoint0.x + (width - controlPoint0.x) / 2, topBarHeight, (width - controlPoint0.x) / 2, controlPoint0.y - topBarHeight, true, true, "3d");

}

void setUIStyle()
{
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;

	style.WindowBorderSize = 1.0f;
	style.WindowRounding = 0.0f;

	style.FrameBorderSize = 1.0f;
	style.FrameRounding = 12.0f;

	style.PopupBorderSize = 0.0f;

	style.ScrollbarRounding = 12.0f;
	style.GrabRounding = 12.0f;
	style.GrabMinSize = 20.0f;


	// spacings
	style.ItemSpacing = ImVec2(8.0f, 8.0f);
	style.FramePadding = ImVec2(8.0f, 8.0f);
	style.WindowPadding = ImVec2(8.0f, 8.0f);



}


void setUpGPU()
{
	gFusionInit();
	//mCubeInit();
	//krender.setBuffersFromMarchingCubes(gfusion.getVertsMC(), gfusion.getNormsMC(), gfusion.getNumVerts());

	// op flow init
	gdisoptflow.compileAndLinkShader();
	gdisoptflow.setLocations();
#ifdef USEINFRARED
	gdisoptflow.setNumLevels(depthWidth);
	gdisoptflow.setTextureParameters(depthWidth, depthHeight);
	gdisoptflow.allocateTextures(true);
#else
	gdisoptflow.setNumLevels(colorWidth);
	gdisoptflow.setTextureParameters(colorWidth, colorHeight);
	gdisoptflow.allocateTextures(false);


#endif

	gdisoptflow.allocateBuffers();

	kRenderInit();




}


void setUI()
{
	setImguiWindows();
	// graphs
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		ImGui::SetNextWindowSize(ImVec2(graphWindow.w, graphWindow.h), ImGuiSetCond_Always);
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//if (graphWindow.resize == false) window_flags |= ImGuiWindowFlags_NoResize;
		//window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Slider Graph", &graphWindow.visible, window_flags);
		//ImGui::PushItemWidth(-krender.guiPadding().first);
		ImGui::SetWindowPos(ImVec2(graphWindow.x, graphWindow.y));
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0, 0.0, 0.0, 1.0));
		ImGui::PlotLines("X", &arrayX[0], graphWindow.w, 0, "", minmaxX.first, minmaxX.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0, 1.0, 0.0, 1.0));
		ImGui::PlotLines("Y", &arrayY[0], graphWindow.w, 0, "", minmaxY.first, minmaxY.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0, 191.0/255.0, 1.0, 1.0));
		ImGui::PlotLines("Z", &arrayZ[0], graphWindow.w, 0, "", minmaxZ.first, minmaxZ.second, ImVec2(graphWindow.w, graphWindow.h / 3));
		ImGui::PopStyleColor();

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

	}

	// 2d data
	{
		ImGui::SetNextWindowPos(ImVec2(display2DWindow.x, display2DWindow.y));
		ImGui::SetNextWindowSize(ImVec2(display2DWindow.w, display2DWindow.h), ImGuiSetCond_Always);

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Video ", &display2DWindow.visible, window_flags);
		imguiFocus2D = ImGui::IsWindowHovered();
		ImGui::End();
	}

	//3d data
	{
		ImGui::SetNextWindowPos(ImVec2(display3DWindow.x, display3DWindow.y));
		ImGui::SetNextWindowSize(ImVec2(display3DWindow.w, display3DWindow.h), ImGuiSetCond_Always);

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Video Sources", &display3DWindow.visible, window_flags);

		ImGui::End();
	}
	// navigation
	{
		ImGui::SetNextWindowPos(ImVec2(navigationWindow.x, navigationWindow.y));
		ImGui::SetNextWindowSize(ImVec2(navigationWindow.w, navigationWindow.h), ImGuiSetCond_Always);
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		//window_flags |= ImGuiWindowFlags_ShowBorders;
		//window_flags |= ImGuiWindowFlags_NoResize;
		//window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		float arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		arr[0] = gdisoptflow.getTimeElapsed();
		gfusion.getTimes(arr);
		arr[8] = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];

		ImGui::Begin("Menu", &navigationWindow.visible, window_flags);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);

		//ImGui::PushItemWidth(-krender.guiPadding().first);
		//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
		ImGui::Text("Help menu - press 'H' to hide");

		// ADD SLIDERS HERE TO CHOOSE PARAMS/PRESETS FOR NEAR MODE

		ImGui::Separator();
		ImGui::Text("Kinect Options");
		if (ImGui::Button("Open Calib"))
		{
			// open dialogue to set kinect calibration yml, xml
			//std::string calFile("resources/infrared.yml");
			std::string calFile("resources/infrared.xml");

			loadCalibration(calFile);
			defaultCalibration = false;
		}

		if (ImGui::Button("Start Kinect"))
		{
			if (cameraRunning == false)
			{
				cameraRunning = true;
				
				if (defaultCalibration)
					std::cout << "Default Calibration Used" << std::endl;
				startKinect();

				setUpGPU();

			}

		}



		static bool openFileDialog = false;

		if (ImGui::Button("Open File Dialog"))
		{
			openFileDialog = true;
		}

		static std::string filePathName = "";
		static std::string path = "";
		static std::string fileName = "";
		static std::string filter = "";

		if (openFileDialog)
		{
			if (ImGuiFileDialog::Instance()->FileDialog("Choose File", ".cpp\0.h\0.hpp\0\0", ".", ""))
			{
				if (ImGuiFileDialog::Instance()->IsOk == true)
				{
					filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
					path = ImGuiFileDialog::Instance()->GetCurrentPath();
					fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
					filter = ImGuiFileDialog::Instance()->GetCurrentFilter();
				}
				else
				{
					filePathName = "";
					path = "";
					fileName = "";
					filter = "";
				}
				openFileDialog = false;
			}
		}

		if (filePathName.size() > 0) ImGui::Text("Choosed File Path Name : %s", filePathName.c_str());
		if (path.size() > 0) ImGui::Text("Choosed Path Name : %s", path.c_str());
		if (fileName.size() > 0) ImGui::Text("Choosed File Name : %s", fileName.c_str());
		if (filter.size() > 0) ImGui::Text("Choosed Filter : %s", filter.c_str());
		ImGui::Separator();
		ImGui::Text("Fusion Options");

		if (ImGui::Button("P2P")) trackDepthToPoint ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToPoint);
		if (ImGui::Button("P2V")) trackDepthToVolume ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToVolume);

		if (ImGui::Button("Reset Volume"))
		{	// update config
			//m_center_pixX

			//glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));
			gdisoptflow.wipeFlow();

			bool deleteFlag = false;

			if (glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ])) != gconfig.volumeSize)
			{
				deleteFlag = true;
			}

			gconfig.volumeSize = glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ]));
			gconfig.volumeDimensions = glm::vec3(dimension);

			gfusion.setConfig(gconfig);

			iOff = initOffset(mousePos.x - controlPoint0.x, controlPoint0.y - mousePos.y);

			std::cout << "lalala " << mousePos.x - controlPoint0.x << " " << controlPoint0.y - mousePos.y<< std::endl;

			glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(-iOff.x + gconfig.volumeDimensions.x / 2.0f, -iOff.y + gconfig.volumeDimensions.y / 2.0f, -iOff.z + dimension / 2.0));


			krender.setVolumeSize(gconfig.volumeSize);

			gfusion.Reset(initPose, deleteFlag);
			reset = true;
			gfusion.raycast();

			counter = 0;
		}

		if (ImGui::Button("Integrate")) integratingFlag ^= 1; ImGui::SameLine();
		ImGui::Checkbox("", &integratingFlag);
		krender.setSelectInitPose(integratingFlag);

		if (ImGui::Button("ROI")) selectInitialPoseFlag ^= 1; ImGui::SameLine();
		ImGui::Checkbox("", &selectInitialPoseFlag);


		if (ImGui::Button("DO SUM")) gfusion.testPrefixSum();
		if (ImGui::Button("save stl"))
		{
			//gfusion.marchingCubes();
			//gfusion.exportSurfaceAsStlBinary();

			mcconfig.gridSize = glm::uvec3(gconfig.volumeSize.x, gconfig.volumeSize.y, gconfig.volumeSize.z);
			//mcconfig.numVoxels = mcconfig.gridSize.x * mcconfig.gridSize.y * mcconfig.gridSize.z;
			//mcconfig.maxVerts = std::min(mcconfig.gridSize.x * mcconfig.gridSize.y * 128, uint32_t(128 * 128 * 128));

			mcubes.setConfig(mcconfig);

			mcubes.setVolumeTexture(gfusion.getVolume());
			mcubes.init();

			mcubes.setIsolevel(0);

			mcubes.generateMarchingCubes();
			mcubes.exportMesh();

		}

		ImGui::PlotHistogram("Timing", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 33.0f, ImVec2(0, 80));


		ImGui::PushItemWidth(-1);
		float avail_width = ImGui::CalcItemWidth();
		float label_width = ImGui::CalcTextSize(" X ").x;
		ImGui::PopItemWidth();
		ImGui::PushItemWidth((avail_width / 3) - label_width);
		ImGui::Combo("X  ", &sizeX, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
		ImGui::Combo("Y  ", &sizeX, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
		ImGui::Combo("Z  ", &sizeX, sizes, IM_ARRAYSIZE(sizes));

		sizeY = sizeX;
		sizeZ = sizeX;

		ImGui::PopItemWidth();

		ImGui::SliderFloat("dim", &dimension, 0.005f, 0.5f);

		ImGui::SliderFloat("slice", &volSlice, 0, gconfig.volumeSize.z - 1);

		ImGui::Separator();
		ImGui::Text("View Options");


		if (ImGui::Button("Show Depth")) showDepthFlag ^= 1; ImGui::SameLine();	ImGui::Checkbox("", &showDepthFlag); ImGui::SameLine(); if (ImGui::Button("Show Big Depth")) showBigDepthFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showBigDepthFlag);
		if (ImGui::Button("Show Infra")) showInfraFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showInfraFlag); ImGui::SameLine(); if (ImGui::Button("Show Flow")) showFlowFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFlowFlag);
		if (ImGui::Button("Show Color")) showColorFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showColorFlag); ImGui::SameLine(); if (ImGui::Button("Show Edges")) showEdgesFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showEdgesFlag);
		if (ImGui::Button("Show Light")) showLightFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showLightFlag); ImGui::SameLine(); if (ImGui::Button("Show RayNorm")) showNormalFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showNormalFlag);
		if (ImGui::Button("Show Point")) showPointFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showPointFlag); ImGui::SameLine(); if (ImGui::Button("Show Volume")) showVolumeFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showVolumeFlag);
		if (ImGui::Button("Show Track")) showTrackFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showTrackFlag);

		ImGui::Separator();
		ImGui::Text("Other Options");

		if (ImGui::Button("Select color points")) select_color_points_mode ^= 1; ImGui::SameLine();
		//if (ImGui::Button("Reset")) OCVStuff.resetColorPoints();

		if (ImGui::Button("Select depth points")) select_depth_points_mode ^= 1; ImGui::SameLine();
		//if (ImGui::Button("Reset Depth")) krender.resetRegistrationMatrix();

		//if (ImGui::Button("Export PLY")) krender.setExportPly(true);
		//if (ImGui::Button("Export PLY")) krender.exportPointCloud();
		//if (ImGui::Button("Save Color")) OCVStuff.saveImage(0); // saving color image (flag == 0)


		ImGui::Separator();
		ImGui::Text("View Transforms");
		ImGui::SliderFloat("vFOV", &vertFov, 1.0f, 90.0f);
		krender.setFov(vertFov);


		ImGui::SliderFloat("xRot", &xRot, 0.0f, 90.0f);
		ImGui::SliderFloat("yRot", &yRot, 0.0f, 90.0f);
		ImGui::SliderFloat("zRot", &zRot, 0.0f, 90.0f);

		ImGui::SliderFloat("xTran", &xTran, -2000.0f, 2000.0f);
		ImGui::SliderFloat("yTran", &yTran, -2000.0f, 2000.0f);
		ImGui::SliderFloat("zTran", &zTran, 0.0f, 4000.0f);

		ImGui::SliderFloat("model z", &zModelPC_offset, -4000.0f, 4000.0f);
		if (ImGui::Button("Reset Sliders")) resetSliders();

		ImGui::Separator();
		ImGui::Text("Infrared Adj.");

		//cv::imshow("irg", infraGrey);

		//if (ImGui::Button("Save Infra")) OCVStuff.saveImage(1);  // saving infra image (flag == 1)
		//ImGui::SliderFloat("depthMin", &depthMin, 0.0f, 10.0f);
		/*if (irLow > (irHigh - 255.0f))
		{
		irHigh = irLow + 255.0f;
		}*/
		//ImGui::SliderFloat("depthMax", &depthMax, 0.0f, 10.0f);
		//if (irHigh < (irLow + 255.0f))
		//{
		//	irLow = irHigh - 255.0f;
		//}
		//krender.setDepthMinMax(depthMin, depthMax);

		ImGui::SliderFloat("irLow", &irLow, 0.0f, 65536.0f - 255.0f);
		if (irLow > (irHigh - 255.0f))
		{
			irHigh = irLow + 255.0f;
		}
		ImGui::SliderFloat("irHigh", &irHigh, 255.0f, 65536.0f);
		if (irHigh < (irLow + 255.0f))
		{
			irLow = irHigh - 255.0f;
		}
		krender.setIrBrightness(irLow, irHigh);

		ImGui::Separator();
		ImGui::Text("Calibration Misc.");
		if (ImGui::Button("Calibrate")) calibratingFlag ^= 1; ImGui::SameLine();
		ImGui::Checkbox("", &calibratingFlag);


		ImGui::End();

	}

	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::IsMouseClicked(0) && imguiFocus2D == true)
	{
		ImVec2 mPos = ImGui::GetMousePos();
		mousePos.x = mPos.x;
		mousePos.y = mPos.y;
		std::cout << "from imgui " << mousePos.x << " " << mousePos.y << std::endl;
		std::cout << "lalala " << mousePos.x - controlPoint0.x << " " << controlPoint0.y - mousePos.y << std::endl;
		iOff = initOffset(mousePos.x - controlPoint0.x, controlPoint0.y - mousePos.y);

	}
}


int main(int, char**)
{
	controlPoint0.x = 512;
	controlPoint0.y = 1080 - 420;

	int display_w, display_h;
	// load openGL window
	window = krender.loadGLFWWindow();

	glfwGetFramebufferSize(window, &display_w, &display_h);
	// Setup ImGui binding
	ImGui::CreateContext();

	ImGui_ImplGlfwGL3_Init(window, true);
	ImVec4 clear_color = ImColor(114, 144, 154);

	setUIStyle();
	setImguiWindows();

	gFusionInit();

	//krender.setBuffersFromMarchingCubes(gfusion.getVertsMC(), gfusion.getNormsMC(), gfusion.getNumVerts());
	//std::cout << "buffers set" << std::endl;

	// op flow init
	gdisoptflow.compileAndLinkShader();
	gdisoptflow.setLocations();
#ifdef USEINFRARED
	gdisoptflow.setNumLevels(depthWidth);
	gdisoptflow.setTextureParameters(depthWidth, depthHeight);
	gdisoptflow.allocateTextures(true);
#else
	gdisoptflow.setNumLevels(colorWidth);
	gdisoptflow.setTextureParameters(colorWidth, colorHeight);
	gdisoptflow.allocateTextures(false);


#endif
	gdisoptflow.allocateBuffers();

	kRenderInit();

	const int samples = 50;
	float time[samples];
	int index = 0;

	


	bool newFrame = false;
	bool show_slider_graph = true;

	float midDepth1 = 0.0f;
	float midDepth2 = 0.0f;
	float midDepth3 = 0.0f;
	



	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		//std::cout << "inside main loop" << std::endl;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		krender.renderScaleHeight((float)display_h / 1080.0f);
		krender.renderScaleWidth((float)display_w / 1920.0f);


		krender.anchorMW(std::make_pair<int, int>(50, 1080 - 424 - 50 ));

		//// Rendering
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		//gfusion.update(float(glfwGetTime()));

		//krender.requestShaderInfo();

	
		if (kcamera.ready())
		{
			kcamera.frames(colorArray, depthArray, infraredArray, bigDepthArray, colorDepthMap);
			//std::cout << "frames returned" << std::endl;

			//	gdisoptflow.setTexture(infraredArray);

		//	gdisoptflow.setTexture(colorArray);

		//	gdisoptflow.calc(true);

		//	gdisoptflow.calc(false);



			
			//gdisoptflow.track();
			
			//gfusion.trackPoints3D(gdisoptflow.getTrackedPointsBuffer());

			//krender.setTrackedPointsBuffer(gdisoptflow.getTrackedPointsBuffer());

			//krender.setFlowTexture(gdisoptflow.getFlowTexture());

			//cv::Mat totflow = cv::Mat(1080 >> 0, 1920 >> 0, CV_32FC2);

			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, gdisoptflow.getFlowTexture());
			//glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, totflow.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
			//glBindTexture(GL_TEXTURE_2D, 0);
			//glActiveTexture(0); 

			//cv::Mat tofl[2];
			//cv::split(totflow, tofl);  

			////cv::imshow("wwee", tofl[0] - tofl[1]);
			////cv::imshow("dwerwev", tofl[1]); 

			//  
			//cv::Mat mag, ang; 
			//cv::Mat hsv_split[3], hsv;
			//cv::Mat rgb; 
			//cv::cartToPolar(tofl[0], tofl[1], mag, ang, true);
			//cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
			//hsv_split[0] = ang;
			//hsv_split[1] = mag;
			//hsv_split[2] = cv::Mat::ones(ang.size(), ang.type()); 
			//cv::merge(hsv_split, 3, hsv);
			//cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
			//cv::Mat outrgb, outmix;
			//rgb.convertTo(outrgb, CV_8UC4, 255);
			//cv::addWeighted(outrgb, 0.7, col, 0.3, 1.0, outmix);
			//cv::imshow("totflowrgb", outrgb);


			//outWriter << outmix;



			gfusion.depthToVertex(depthArray);
			//std::cout << "projected to 3D " << std::endl;
			gfusion.vertexToNormal();
			//std::cout << "normailsed " << std::endl;
			//gfusion.showNormals();
			//gfusion.showRaycast();

			bool tracked;

			if (trackDepthToPoint)
			{
				tracked = gfusion.Track(); // this should return a bool for successful track
			}
			//std::cout << "tracked depth to point " << std::endl;

			if (trackDepthToVolume)
			{
				tracked = gfusion.TrackSDF();
			}
			//
			//std::cout << "tracked depth to volume " << std::endl;

			//gfusion.testLargeUpload();

			if (tracked && integratingFlag && ((counter % 1) == 0) || reset)
			{
				gfusion.integrate();
				if (counter > 2)
					reset = false;
			}

			if (!tracked)
			{
				std::cout << "tracking failed" << std::endl;

				gfusion.recoverPose();
			}


			counter++;


			gfusion.raycast();
			//std::cout << "raycasted " << std::endl;

			graphPoints.push_back(gfusion.getTransPose());

			if (graphPoints.size() > graphWindow.w)
			{
				graphPoints.pop_front();
			}
			minmaxX = std::make_pair<float, float>(1000, -1000.f);
			minmaxY = std::make_pair<float, float>(1000, -1000.f);;
			minmaxZ = std::make_pair<float, float>(1000, -1000.f);;

			int idx = 0;
			for (auto i : graphPoints)
			{
				if (i[0] < minmaxX.first) minmaxX.first = i[0];
				if (i[0] > minmaxX.second) minmaxX.second = i[0];

				if (i[1] < minmaxY.first) minmaxY.first = i[1];
				if (i[1] > minmaxY.second) minmaxY.second = i[1];

				if (i[2] < minmaxZ.first) minmaxZ.first = i[2];
				if (i[2] > minmaxZ.second) minmaxZ.second = i[2];

				arrayX[idx] = i[0];
				arrayY[idx] = i[1];
				arrayZ[idx] = i[2];

				idx++;
			}

			//gfusion.intensityProjection();
			//gfusion.marchingCubes();


			//gfusion.exportSurfaceAsStlBinary();

			//krender.setBuffersFromMarchingCubes(gfusion.getVertsMC(), gfusion.getNormsMC(), gfusion.getNumVerts());

			//gfusion.showDifference();

			newFrame = true; // make this a return bool on the kcamera.frames()


		}
		else
		{
			newFrame = false;
		}

		bool show_test_window = true;


			glfwPollEvents();
			ImGui_ImplGlfwGL3_NewFrame();
			setUI();


			// GRAPHS
			// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
			//{

			//	ImGui::SetNextWindowSize(ImVec2(display_w - (2 * 32), 390), ImGuiSetCond_Always);
			//	ImGuiWindowFlags window_flags = 0;
			//	window_flags |= ImGuiWindowFlags_NoTitleBar;
			//	//window_flags |= ImGuiWindowFlags_ShowBorders;
			//	window_flags |= ImGuiWindowFlags_NoResize;
			//	window_flags |= ImGuiWindowFlags_NoMove;
			//	window_flags |= ImGuiWindowFlags_NoCollapse;

			//	ImGui::Begin("Slider Graph", &show_slider_graph, window_flags);
			//	ImGui::PushItemWidth(-krender.guiPadding().first);
			//	ImGui::SetWindowPos(ImVec2(32, 900 - 32 - 390));
			//	ImGui::PlotLines("X", &arrayX[0], 900, 0, "", minmaxX.first, minmaxX.second, ImVec2(0, 80));
			//	ImGui::PlotLines("Y", &arrayY[0], 900, 0, "", minmaxY.first, minmaxY.second, ImVec2(0, 80));
			//	ImGui::PlotLines("Z", &arrayZ[0], 900, 0, "", minmaxZ.first, minmaxZ.second, ImVec2(0, 80));

			//	ImGui::End();

			//}


			//// MAIN VIDEO
			//bool show_main_video = true;
			//{
			//	ImGui::SetNextWindowPos(ImVec2(32, 32));
			//	ImGui::SetNextWindowSize(ImVec2(512, 424), ImGuiSetCond_Always);

			//	ImGuiWindowFlags window_flags = 0;
			//	window_flags |= ImGuiWindowFlags_NoTitleBar;
			//	//window_flags |= ImGuiWindowFlags_ShowBorders;
			//	window_flags |= ImGuiWindowFlags_NoResize;
			//	window_flags |= ImGuiWindowFlags_NoMove;
			//	window_flags |= ImGuiWindowFlags_NoCollapse;

			//	ImGui::Begin("Video ", &show_main_video, window_flags);

			//	ImGui::End();
			//}

			////// THUMBNAILS
			//bool show_thumbnails = true;
			//{
			//	ImGui::SetNextWindowPos(ImVec2(32 + 512 + 32, 32));
			//	ImGui::SetNextWindowSize(ImVec2(1600 - 32 - 512 - 32 - 32 - 32 - 528 - 150, 424), ImGuiSetCond_Always);

			//	ImGuiWindowFlags window_flags = 0;
			//	window_flags |= ImGuiWindowFlags_NoTitleBar;
			//	//window_flags |= ImGuiWindowFlags_ShowBorders;
			//	window_flags |= ImGuiWindowFlags_NoResize;
			//	window_flags |= ImGuiWindowFlags_NoMove;
			//	window_flags |= ImGuiWindowFlags_NoCollapse;

			//	ImGui::Begin("Video Sources", &show_thumbnails, window_flags);

			//	ImGui::End();
			//}


			//if (krender.showImgui())
			//{
			//	ImGui::SetNextWindowPos(ImVec2(1600 - 32 - 528 - 150, 32));
			//	ImGui::SetNextWindowSize(ImVec2(528 + 150, 424), ImGuiSetCond_Always);
			//	ImGuiWindowFlags window_flags = 0;
			//	window_flags |= ImGuiWindowFlags_NoTitleBar;
			//	//window_flags |= ImGuiWindowFlags_ShowBorders;
			//	window_flags |= ImGuiWindowFlags_NoResize;
			//	window_flags |= ImGuiWindowFlags_NoMove;
			//	window_flags |= ImGuiWindowFlags_NoCollapse;

			//	float arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			//	arr[0] = gdisoptflow.getTimeElapsed();
			//	gfusion.getTimes(arr);
			//	arr[8] = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];

			//	ImGui::Begin("Menu", &show_slider_graph, window_flags);
			//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);

			//	//ImGui::PushItemWidth(-krender.guiPadding().first);
			//	//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
			//	ImGui::Text("Help menu - press 'H' to hide");

			//	ImGui::Separator();
			//	ImGui::Text("Kinect Options");
			//	if (ImGui::Button("Open Calib"))
			//	{
			//		// open dialogue to set kinect calibration yml, xml
			//		//std::string calFile("resources/infrared.yml");
			//		std::string calFile("resources/infrared.xml");

			//		loadCalibration(calFile);
			//		defaultCalibration = false;
			//	}
			//	if (ImGui::Button("Start Kinect"))
			//	{
			//		if (defaultCalibration)
			//			std::cout << "Default Calibration Used" << std::endl;
			//		startKinect();
			//	}

			//	ImGui::Separator();
			//	ImGui::Text("Fusion Options");

			//	if (ImGui::Button("P2P")) trackDepthToPoint ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToPoint);
			//	if (ImGui::Button("P2V")) trackDepthToVolume ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &trackDepthToVolume);

			//	if (ImGui::Button("Reset Volume"))
			//	{	// update config
			//		//m_center_pixX

			//		//glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(gconfig.volumeDimensions.x / 2.0f, gconfig.volumeDimensions.y / 2.0f, 0.0f));
			//		gdisoptflow.wipeFlow();

			//		bool deleteFlag = false;
			//		
			//		if (glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ])) != gconfig.volumeSize)
			//		{
			//			deleteFlag = true;
			//		}

			//		gconfig.volumeSize = glm::vec3(std::stoi(sizes[sizeX]), std::stoi(sizes[sizeY]), std::stoi(sizes[sizeZ]));
			//		gconfig.volumeDimensions = glm::vec3(dimension);
			//		gfusion.setConfig(gconfig);

			//		iOff = initOffset(krender.getCenterPixX(), krender.getCenterPixY());

			//		glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(-iOff.x + gconfig.volumeDimensions.x / 2.0f, -iOff.y + gconfig.volumeDimensions.y / 2.0f, -iOff.z + 0.05f));


			//		krender.setVolumeSize(gconfig.volumeSize);

			//		gfusion.Reset(initPose, deleteFlag);
			//		reset = true;
			//		gfusion.raycast();

			//		counter = 0;
			//	}

			//	if (ImGui::Button("Integrate")) integratingFlag ^= 1; ImGui::SameLine();
			//	ImGui::Checkbox("", &integratingFlag);
			//	krender.setSelectInitPose(integratingFlag);

			//	if (ImGui::Button("ROI")) selectInitialPoseFlag ^= 1; ImGui::SameLine();
			//	ImGui::Checkbox("", &selectInitialPoseFlag);


			//	//if (ImGui::Button("DO SUM")) gfusion.testPrefixSum();
			//	//if (ImGui::Button("save stl")) gfusion.exportSurfaceAsStlBinary();
			//	if (ImGui::Button("save stl"))
			//	{
			//		//gfusion.marchingCubes();
			//		//gfusion.exportSurfaceAsStlBinary();

			//		mcconfig.gridSize = glm::uvec3(gconfig.volumeSize.x, gconfig.volumeSize.y, gconfig.volumeSize.z);
			//		//mcconfig.numVoxels = mcconfig.gridSize.x * mcconfig.gridSize.y * mcconfig.gridSize.z;
			//		//mcconfig.maxVerts = std::min(mcconfig.gridSize.x * mcconfig.gridSize.y * 128, uint32_t(128 * 128 * 128));

			//		mcubes.setConfig(mcconfig);

			//		mcubes.setVolumeTexture(gfusion.getVolume());
			//		mcubes.init();

			//		mcubes.setIsolevel(0);

			//		mcubes.generateMarchingCubes();
			//		mcubes.exportMesh();

			//	}

			//	ImGui::PlotHistogram("Timing", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 33.0f, ImVec2(0, 80));


			//	ImGui::PushItemWidth(-1);
			//	float avail_width = ImGui::CalcItemWidth();
			//	float label_width = ImGui::CalcTextSize(" X ").x;
			//	ImGui::PopItemWidth();
			//	ImGui::PushItemWidth((avail_width / 3) - label_width);
			//	ImGui::Combo("X  ", &sizeX, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
			//	ImGui::Combo("Y  ", &sizeY, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
			//	ImGui::Combo("Z  ", &sizeZ, sizes, IM_ARRAYSIZE(sizes));

			//	ImGui::PopItemWidth();

			//	ImGui::SliderFloat("dim", &dimension, 0.05f, 4.0f);

			//	ImGui::SliderFloat("slice", &volSlice, 0, gconfig.volumeSize.z - 1);

			//	ImGui::Separator();
			//	ImGui::Text("View Options");

			//	if (ImGui::Button("Show Depth")) showDepthFlag ^= 1; ImGui::SameLine();	ImGui::Checkbox("", &showDepthFlag); ImGui::SameLine(); if (ImGui::Button("Show Big Depth")) showBigDepthFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showBigDepthFlag);
			//	if (ImGui::Button("Show Infra")) showInfraFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showInfraFlag); ImGui::SameLine(); if (ImGui::Button("Show Flow")) showFlowFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFlowFlag);
			//	if (ImGui::Button("Show Color")) showColorFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showColorFlag); ImGui::SameLine(); if (ImGui::Button("Show Edges")) showEdgesFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showEdgesFlag);
			//	if (ImGui::Button("Show Light")) showLightFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showLightFlag); ImGui::SameLine(); if (ImGui::Button("Show RayNorm")) showNormalFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showNormalFlag);
			//	if (ImGui::Button("Show Point")) showPointFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showPointFlag); ImGui::SameLine(); if (ImGui::Button("Show Volume")) showVolumeFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showVolumeFlag);
			//	if (ImGui::Button("Show Track")) showTrackFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showTrackFlag); 

			//	ImGui::Separator();
			//	ImGui::Text("Other Options");

			//	if (ImGui::Button("Select color points")) select_color_points_mode ^= 1; ImGui::SameLine();
			//	//if (ImGui::Button("Reset")) OCVStuff.resetColorPoints();

			//	if (ImGui::Button("Select depth points")) select_depth_points_mode ^= 1; ImGui::SameLine();
			//	//if (ImGui::Button("Reset Depth")) krender.resetRegistrationMatrix();

			//	//if (ImGui::Button("Export PLY")) krender.setExportPly(true);
			//	//if (ImGui::Button("Export PLY")) krender.exportPointCloud();
			//	//if (ImGui::Button("Save Color")) OCVStuff.saveImage(0); // saving color image (flag == 0)


			//	ImGui::Separator();
			//	ImGui::Text("View Transforms");
			//	ImGui::SliderFloat("vFOV", &vertFov, 1.0f, 90.0f);
			//	krender.setFov(vertFov);


			//	ImGui::SliderFloat("xRot", &xRot, 0.0f, 90.0f);
			//	ImGui::SliderFloat("yRot", &yRot, 0.0f, 90.0f);
			//	ImGui::SliderFloat("zRot", &zRot, 0.0f, 90.0f);

			//	ImGui::SliderFloat("xTran", &xTran, -2000.0f, 2000.0f);
			//	ImGui::SliderFloat("yTran", &yTran, -2000.0f, 2000.0f);
			//	ImGui::SliderFloat("zTran", &zTran, 0.0f, 4000.0f);

			//	ImGui::SliderFloat("model z", &zModelPC_offset, -4000.0f, 4000.0f);
			//	if (ImGui::Button("Reset Sliders")) resetSliders();

			//	ImGui::Separator();
			//	ImGui::Text("Infrared Adj.");

			//	//cv::imshow("irg", infraGrey);

			//	//if (ImGui::Button("Save Infra")) OCVStuff.saveImage(1);  // saving infra image (flag == 1)
			//	ImGui::SliderFloat("irLow", &irLow, 0.0f, 65536.0f - 255.0f);
			//	if (irLow > (irHigh - 255.0f))
			//	{
			//		irHigh = irLow + 255.0f;
			//	}
			//	ImGui::SliderFloat("irHigh", &irHigh, 255.0f, 65536.0f);
			//	if (irHigh < (irLow + 255.0f))
			//	{
			//		irLow = irHigh - 255.0f;
			//	}
			//	krender.setIrBrightness(irLow, irHigh);

			//	ImGui::Separator();
			//	ImGui::Text("Calibration Misc.");
			//	if (ImGui::Button("Calibrate")) calibratingFlag ^= 1; ImGui::SameLine();
			//	ImGui::Checkbox("", &calibratingFlag);


			//	ImGui::End();

			//}

		

			krender.setRenderingOptions(showDepthFlag, showBigDepthFlag, showInfraFlag, showColorFlag, showLightFlag, showPointFlag, showFlowFlag, showEdgesFlag, showNormalFlag, showVolumeFlag, showTrackFlag);


			//ImGui::ShowTestWindow();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

			
			if (newFrame)
			{
				//krender.setColorDepthMapping(colorDepthMap);
				//krender.computeEdges();

				//krender.setRenderingOptions(showDepthFlag, showBigDepthFlag, showInfraFlag, showColorFlag, showLightFlag, showPointFlag, showFlowFlag, showEdgesFlag);

				//krender.setBuffersForRendering(depthArray, bigDepthArray, infraredArray, colorArray, flow.ptr());
				krender.setDepthImageRenderPosition(vertFov);
				krender.setRayNormImageRenderPosition(vertFov);
				krender.setTrackImageRenderPosition(vertFov);

				//krender.setInfraImageRenderPosition();
				krender.setColorImageRenderPosition(vertFov);

#ifdef USEINFRARED
				krender.setFlowImageRenderPosition(depthHeight, depthWidth, vertFov);
#else
				krender.setFlowImageRenderPosition(colorHeight, colorWidth, vertFov);

#endif
				
				
				//krender.setPointCloudRenderPosition(zModelPC_offset);
				//krender.setLightModelRenderPosition();
				krender.setVolumeSDFRenderPosition(volSlice);
				
				krender.setMarchingCubesRenderPosition(zModelPC_offset);
				krender.setViewMatrix(xRot, yRot, zRot, xTran, yTran, zTran);
				krender.setDepthTextureProjectionMatrix();


				// compute time
				//krender.filterDepth(showBigDepthFlag);
				//krender.computeDepthToVertex(showBigDepthFlag);
				//krender.computeVertexToNormal(showBigDepthFlag);
				//krender.filterGaps();

				//krender.computeBlur(showBigDepthFlag);

				//krender.integrateVolume();
				//krender.raycastVolume();

				//krender.getSlice();
				//krender.getRayCastImage();
				//krender.renderLiveVideoWindow();


				//krender.renderPointCloud(showBigDepthFlag);
				//krender.renderTSDFPointCloud();

			}
			else 
			{
				//krender.setColorDepthMapping(colorDepthMap);
				//krender.setBuffersForRendering(NULL, NULL, NULL, NULL, NULL);
				//krender.renderLiveVideoWindow();
				//krender.renderPointCloud(showBigDepthFlag);
				//krender.renderTSDFPointCloud();



			}
			if (cameraRunning)
			{
				krender.setDisplayOriSize(display2DWindow.x, display_h - display2DWindow.y - display2DWindow.h, display2DWindow.w, display2DWindow.h);
#ifdef USEINFRARED
				krender.Render(true);
#else
				krender.Render(false);
#endif
			}




			krender.setComputeWindowPosition();

			glfwSwapBuffers(window);
		}

	

	// Cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	kcamera.stop();

	//krender.cleanUp();

	return 0;
}