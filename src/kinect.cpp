#include "kinect.h"

#define USEINFRARED
//#define USEWEBCAM
//#define USEIMAGES
//#define USEVIDEO
//#define USETESTIMAGE

//
//#include "opencv2/core/utility.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"

// To split this up we will have just rendering calls in 
// kRender - rendering calls
// gFusion - raycasting, integrating, set, reset, getPose, setPose, depth to vert, vert to normals, 

// memory bank
// volume - 3D Texture short2 RG16I, 256x256x256
// depth - 2D Texture float RED, 512x424 mipmapped
// color - 2D Texture unsigned byte RGBA, 1920x1080
// vertex - 2D Texture float RGBA32F, vector of textures mipmapped
// normal - 2D Texture float RGBA32F, vector of textures mipmapped



// Doesnt work :( cant seem to easily get laptop to use integrated GPU from openGL context. 
//#include <windows.h>
//extern "C" {
//	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
//}







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
	mcconfig.gridSize = gconfig.volumeSize;
	mcconfig.gridSizeMask = glm::uvec3(mcconfig.gridSize.x - 1, mcconfig.gridSize.y - 1, mcconfig.gridSize.z - 1);
	mcconfig.gridSizeShift = glm::uvec3(0, log2(mcconfig.gridSize.x), log2(mcconfig.gridSize.y) + log2(mcconfig.gridSize.z));
	mcconfig.numVoxels = mcconfig.gridSize.x * mcconfig.gridSize.y * mcconfig.gridSize.z;
	mcconfig.voxelSize = glm::vec3(gconfig.volumeDimensions.x * 1000.0f / gconfig.volumeSize.x, gconfig.volumeDimensions.y * 1000.0f / gconfig.volumeSize.y, gconfig.volumeDimensions.z * 1000.0f / gconfig.volumeSize.z);
	mcconfig.maxVerts = std::min(mcconfig.gridSize.x * mcconfig.gridSize.y * 128, uint32_t(128 * 128 * 128));

	gfusion.setMcConfig(mcconfig);

}



int main(int, char**)
{


	int display_w, display_h;
	// load openGL window
	window = krender.loadGLFWWindow();

	glfwGetFramebufferSize(window, &display_w, &display_h);
	// Setup ImGui binding
	ImGui::CreateContext();

	ImGui_ImplGlfwGL3_Init(window, true);
	ImVec4 clear_color = ImColor(114, 144, 154);

	gFusionInit();
	mCubeInit();
	//std::cout << "fusion and mcube init" << std::endl;

	krender.setBuffersFromMarchingCubes(gfusion.getVertsMC(), gfusion.getNormsMC(), gfusion.getNumVerts());
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

	//std::cout << "flow set" << std::endl;

	kRenderInit();
	//std::cout << "render init" << std::endl;



	//gfusion.queryWorkgroupSizes();




	//cv::Ptr<cv::DenseOpticalFlow> algorithm = cv::optflow::createOptFlow_DIS(cv::optflow::DISOpticalFlow::PRESET_MEDIUM);
	//
	//cv::Mat prevgray, gray, graySmall, rgb, frame;
	//cv::Mat I0x = cv::Mat(1080, 1920, CV_16SC1);
	//cv::Mat I0y = cv::Mat(1080, 1920, CV_16SC1);
	//cv::Mat flow, flow_uv[2];
	//cv::Mat mag, ang;
	//cv::Mat hsv_split[3], hsv;
	//char ret;

	//cv::namedWindow("flow", 1);
	//cv::namedWindow("orig", 1);

	const int samples = 50;
	float time[samples];
	int index = 0;

	kcamera.start();

	while (!kcamera.ready())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	//std::cout << "camera ready" << std::endl;

	bool newFrame = false;
	bool show_slider_graph = true;

	float midDepth1 = 0.0f;
	float midDepth2 = 0.0f;
	float midDepth3 = 0.0f;
	//OCVStuff.setupAruco();

	//cv::Mat irCamPams = cv::Mat::eye(3, 3, CV_32F);
	//cv::Mat irCamDist = cv::Mat(5, 1, CV_32F);

	//irCamPams.at<float>(0, 0) = kcamera.fx();
	//irCamPams.at<float>(1, 1) = kcamera.fy();
	//irCamPams.at<float>(0, 2) = 512.0f - kcamera.ppx();
	//irCamPams.at<float>(1, 2) = 424.0f - kcamera.ppy();

	//irCamDist.at<float>(0, 0) = kcamera.k1();
	//irCamDist.at<float>(1, 0) = kcamera.k2();
	//irCamDist.at<float>(2, 0) = kcamera.p1(); // k3 not used in aruco
	//irCamDist.at<float>(3, 0) = kcamera.p2();

	//cv::Mat newcol = cv::Mat(1080, 1920, CV_8UC4, colorArray);




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

		krender.setCameraParams(glm::vec4(kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()), glm::vec4(kcamera.fx_col(), kcamera.fx_col(), kcamera.ppx_col(), kcamera.ppy_col())); // FIX ME
		gfusion.setCameraParams(glm::vec4(kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()), glm::vec4(kcamera.fx_col(), kcamera.fx_col(), kcamera.ppx_col(), kcamera.ppy_col())); // FIX ME

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

			if (graphPoints.size() > 900)
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


			// GRAPHS
			// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
			{

				ImGui::SetNextWindowSize(ImVec2(display_w - (2 * 32), 390), ImGuiSetCond_Always);
				ImGuiWindowFlags window_flags = 0;
				window_flags |= ImGuiWindowFlags_NoTitleBar;
				//window_flags |= ImGuiWindowFlags_ShowBorders;
				window_flags |= ImGuiWindowFlags_NoResize;
				window_flags |= ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoCollapse;

				ImGui::Begin("Slider Graph", &show_slider_graph, window_flags);
				ImGui::PushItemWidth(-krender.guiPadding().first);
				ImGui::SetWindowPos(ImVec2(32, 900 - 32 - 390));
				ImGui::PlotLines("X", &arrayX[0], 900, 0, "", minmaxX.first, minmaxX.second, ImVec2(0, 80));
				ImGui::PlotLines("Y", &arrayY[0], 900, 0, "", minmaxY.first, minmaxY.second, ImVec2(0, 80));
				ImGui::PlotLines("Z", &arrayZ[0], 900, 0, "", minmaxZ.first, minmaxZ.second, ImVec2(0, 80));

				ImGui::End();

			}


			// MAIN VIDEO
			bool show_main_video = true;
			{
				ImGui::SetNextWindowPos(ImVec2(32, 32));
				ImGui::SetNextWindowSize(ImVec2(512, 424), ImGuiSetCond_Always);

				ImGuiWindowFlags window_flags = 0;
				window_flags |= ImGuiWindowFlags_NoTitleBar;
				//window_flags |= ImGuiWindowFlags_ShowBorders;
				window_flags |= ImGuiWindowFlags_NoResize;
				window_flags |= ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoCollapse;

				ImGui::Begin("Video ", &show_main_video, window_flags);

				ImGui::End();
			}

			//// THUMBNAILS
			bool show_thumbnails = true;
			{
				ImGui::SetNextWindowPos(ImVec2(32 + 512 + 32, 32));
				ImGui::SetNextWindowSize(ImVec2(1600 - 32 - 512 - 32 - 32 - 32 - 528 - 150, 424), ImGuiSetCond_Always);

				ImGuiWindowFlags window_flags = 0;
				window_flags |= ImGuiWindowFlags_NoTitleBar;
				//window_flags |= ImGuiWindowFlags_ShowBorders;
				window_flags |= ImGuiWindowFlags_NoResize;
				window_flags |= ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoCollapse;

				ImGui::Begin("Video Sources", &show_thumbnails, window_flags);

				ImGui::End();
			}


			if (krender.showImgui())
			{
				ImGui::SetNextWindowPos(ImVec2(1600 - 32 - 528 - 150, 32));
				ImGui::SetNextWindowSize(ImVec2(528 + 150, 424), ImGuiSetCond_Always);
				ImGuiWindowFlags window_flags = 0;
				window_flags |= ImGuiWindowFlags_NoTitleBar;
				//window_flags |= ImGuiWindowFlags_ShowBorders;
				window_flags |= ImGuiWindowFlags_NoResize;
				window_flags |= ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoCollapse;

				float arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				arr[0] = gdisoptflow.getTimeElapsed();
				gfusion.getTimes(arr);
				arr[8] = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];

				ImGui::Begin("Menu", &show_slider_graph, window_flags);
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);

				//ImGui::PushItemWidth(-krender.guiPadding().first);
				//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
				ImGui::Text("Help menu - press 'H' to hide");

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

					iOff = initOffset(krender.getCenterPixX(), krender.getCenterPixY());

					glm::mat4 initPose = glm::translate(glm::mat4(1.0f), glm::vec3(-iOff.x + gconfig.volumeDimensions.x / 2.0f, -iOff.y + gconfig.volumeDimensions.y / 2.0f, -iOff.z + 0.05f));


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
				if (ImGui::Button("save stl")) gfusion.exportSurfaceAsStlBinary();

				ImGui::PlotHistogram("Timing", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 33.0f, ImVec2(0, 80));


				ImGui::PushItemWidth(-1);
				float avail_width = ImGui::CalcItemWidth();
				float label_width = ImGui::CalcTextSize(" X ").x;
				ImGui::PopItemWidth();
				ImGui::PushItemWidth((avail_width / 3) - label_width);
				ImGui::Combo("X  ", &sizeX, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
				ImGui::Combo("Y  ", &sizeY, sizes, IM_ARRAYSIZE(sizes)); ImGui::SameLine(0.0f, 0.0f);
				ImGui::Combo("Z  ", &sizeZ, sizes, IM_ARRAYSIZE(sizes));

				ImGui::PopItemWidth();

				ImGui::SliderFloat("dim", &dimension, 0.05f, 4.0f);

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
#ifdef USEINFRARED
			krender.Render(true);
#else
			krender.Render(false);
#endif



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