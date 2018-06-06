#include "interface.h"

//#include <libfreenect2/packet_pipeline.h>




void Freenect2Camera::start()
{
	if (m_status == STOPPED)
	{
		m_status = CAPTURING;
		m_thread = new std::thread(&Freenect2Camera::captureLoop, this);
	}
}

void Freenect2Camera::stop()
{
	if (m_status == CAPTURING)
	{
		m_status = STOPPED;

		if (m_thread->joinable())
		{
			m_thread->join();
		}

		m_thread = nullptr;
	}
}

void Freenect2Camera::frames(unsigned char * colorAray, float * bigDepthArray)
{
	//m_mtx.lock();

	//memcpy_S the arrays to the pointers passed in here

	memcpy_s(colorAray, 1920 * 1080 * 4, m_rawColor, 1920 * 1080 * 4);
	memcpy_s(bigDepthArray, 1920 * 1082 * 4, m_rawBigDepth, 1920 * 1082 * 4);



	//m_mtx.unlock();
	m_frames_ready = false;
}

// get all the frames available, pass a null pointer in for each array you dont want back
void Freenect2Camera::frames(unsigned char * colorArray, float * depthArray, float * infraredArray, float * bigDepthArray, int * colorDepthMapping)
{
	//m_mtx.lock();

	//memcpy_S the arrays to the pointers passed in here

		if (colorArray != NULL)
		{
			memcpy_s(colorArray, 1920 * 1080 * 4, m_rawColor, 1920 * 1080 * 4);
		}

		if (depthArray != NULL)
		{
			memcpy_s(depthArray, 512 * 424 * 4, m_depth_frame, 512 * 424 * 4);
		}

		if (infraredArray != NULL)
		{
			memcpy_s(infraredArray, 512 * 424 * 4, m_infra_frame, 512 * 424 * 4);
		}

		if (bigDepthArray != NULL)
		{
			memcpy_s(bigDepthArray, 1920 * 1082 * 4, m_rawBigDepth, 1920 * 1082 * 4);
		}
		if (colorDepthMapping != NULL)
		{
			memcpy_s(colorDepthMapping, 512 * 424 * 4, m_color_Depth_Map, 512 * 424 * 4);
		}






	//m_mtx.unlock();
	m_frames_ready = false;
}


//
//
//void Freenect2Camera::frames(cv::Mat &color, cv::Mat &depth, cv::Mat &infra, float &fullcolor)
//{
//	m_mtx.lock();
//
//	//cv::cvtColor(m_color_frame, color, CV_BGRA2BGR);
//
//	m_depth_frame.convertTo(depth, CV_16UC1);
//	m_infra_frame.convertTo(infra, CV_16UC1);
//
//	color = cv::Mat(1080, 1920, CV_8UC4, rawColor);
//
//	//fullcolor = **rawColor;
//
//	m_mtx.unlock();
//
//	//cv::Mat newColor = cv::Mat(1080, 1920, CV_8UC4, rawColor);
//	//cv::imshow("col", newColor);
//	//cv::waitKey(1);
//
//	m_frames_ready = false;
//}
//
//void Freenect2Camera::frames(float * color, float * depth, float * infra)
//{
//	//m_mtx.lock();
//
//	//cv::cvtColor(m_color_frame, color, CV_BGRA2BGR);
//	//color = cv::Mat(1080, 1920, CV_8UC4, rawColor);
//
//	//m_depth_frame.convertTo(depth, CV_16UC1);
//	//m_infra_frame.convertTo(infra, CV_16UC1);
//
//	//m_mtx.unlock();
//	memcpy_s(color, m_frame_width * m_frame_height * 4, m_color_frame, m_frame_width * m_frame_height * 4);
//	memcpy_s(depth, m_frame_width * m_frame_height * 4, m_depth_frame, m_frame_width * m_frame_height * 4);
//	memcpy_s(infra, m_frame_width * m_frame_height * 4, m_infra_frame, m_frame_width * m_frame_height * 4);
//
//	m_frames_ready = false;
//}
//
//void Freenect2Camera::frames(cv::Mat &color, cv::Mat &depth)
//{
//	m_mtx.lock();
//
//	cv::cvtColor(m_color_frame, color, CV_BGRA2BGR);
//	m_depth_frame.convertTo(depth, CV_16UC1);
//
//	m_mtx.unlock();
//
//	m_frames_ready = false;
//}

std::vector<float> Freenect2Camera::getColorCameraParameters()
{
	std::vector<float> camPams;

	camPams.push_back(m_colorCamPams.fx);
	camPams.push_back(m_colorCamPams.fy);
	camPams.push_back(m_colorCamPams.cx);
	camPams.push_back(m_colorCamPams.cy);
	camPams.push_back(m_colorCamPams.mx_x3y0);
	camPams.push_back(m_colorCamPams.mx_x0y3);
	camPams.push_back(m_colorCamPams.mx_x2y1);
	camPams.push_back(m_colorCamPams.mx_x1y2);
	camPams.push_back(m_colorCamPams.mx_x2y0);
	camPams.push_back(m_colorCamPams.mx_x0y2);
	camPams.push_back(m_colorCamPams.mx_x1y1);
	camPams.push_back(m_colorCamPams.mx_x1y0);
	camPams.push_back(m_colorCamPams.mx_x0y1);
	camPams.push_back(m_colorCamPams.mx_x0y0);
	camPams.push_back(m_colorCamPams.my_x3y0);
	camPams.push_back(m_colorCamPams.my_x0y3);
	camPams.push_back(m_colorCamPams.my_x2y1);
	camPams.push_back(m_colorCamPams.my_x1y2);
	camPams.push_back(m_colorCamPams.my_x2y0);
	camPams.push_back(m_colorCamPams.my_x0y2);
	camPams.push_back(m_colorCamPams.my_x1y1);
	camPams.push_back(m_colorCamPams.my_x1y0);
	camPams.push_back(m_colorCamPams.my_x0y1);
	camPams.push_back(m_colorCamPams.my_x0y0);
	camPams.push_back(m_colorCamPams.shift_d);
	camPams.push_back(m_colorCamPams.shift_m);

	return camPams;
}

void Freenect2Camera::captureLoop()
{
	m_frame_width = 512;
	m_frame_height = 424;

	m_rawColor = new float[1920 * 1080];
	m_rawBigDepth = new float[1920 * 1082];

	m_color_frame = new float[m_frame_width * m_frame_height];
	m_depth_frame = new float[m_frame_width * m_frame_height];
	m_infra_frame = new float[m_frame_width * m_frame_height];
	m_color_Depth_Map = new int[m_frame_width * m_frame_height];


	//cv::Size2i frame_size(m_frame_width, m_frame_height);

	libfreenect2::Freenect2 freenect2;
	libfreenect2::Freenect2Device *dev = 0;
	libfreenect2::PacketPipeline *pipeline = 0;




	libfreenect2::Freenect2Device::Config configNearMode;
	configNearMode.MinDepth = 0.005f;
	configNearMode.MaxDepth = 4.5f;


	//pipeline = new libfreenect2::CpuPacketPipeline();
	//pipeline = new libfreenect2::OpenGLPacketPipeline();
	pipeline = new libfreenect2::OpenCLPacketPipeline(0);
	//pipeline = new libfreenect2::CudaPacketPipeline(0);


	if (freenect2.enumerateDevices() == 0)
	{
		std::cout << "no device connected!" << std::endl;
		return;
	}

	std::string serial = freenect2.getDefaultDeviceSerialNumber();
	dev = freenect2.openDevice(serial, pipeline);

	if (dev == 0)
	{
		std::cout << "failure opening device!" << std::endl;
		return;
	}

	int types = libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
	libfreenect2::SyncMultiFrameListener listener(types);
	libfreenect2::FrameMap frames;

	dev->setColorFrameListener(&listener);
	dev->setIrAndDepthFrameListener(&listener);

	dev->setConfiguration(configNearMode, 0.1322581f, 1.0f, 0.1612903f);

	if (!dev->start())
		return;



	libfreenect2::Freenect2Device::IrCameraParams irCamParams;// = dev->getIrCameraParams();
	//irCamParams.fx = 3.607533433165361e+02;
	//irCamParams.fy = 3.613185655370914e+02;
	//irCamParams.cx = 2.582765413544986e+02;
	//irCamParams.cy = 2.030852224480832e+02;
	//irCamParams.k1 = 0.101243877185196;
	//irCamParams.k2 = -0.279311690736655;
	//irCamParams.k3 = 0.073644459910773;
	//irCamParams.p1 = -0.003461526071644;
	//irCamParams.p2 = -9.965536968853084e-04;

	irCamParams.fx = m_depthCamPams.fx;
	irCamParams.fy = m_depthCamPams.fy;
	irCamParams.cx = m_depthCamPams.ppx;
	irCamParams.cy = m_depthCamPams.ppy;
	irCamParams.k1 = m_depthCamPams.k1;
	irCamParams.k2 = m_depthCamPams.k2;
	irCamParams.k3 = m_depthCamPams.k3;
	irCamParams.p1 = m_depthCamPams.p1;
	irCamParams.p2 = m_depthCamPams.p2;


	//irCamParams.fx = 1.532509370e+003;
	//irCamParams.fy = 1.53012236359e+003;
	//irCamParams.cx = 2.476569e+002;
	//irCamParams.cy = 1.865447e+002;
	//irCamParams.k1 = 6.43e-2;
	//irCamParams.k2 = -5.2153;
	//irCamParams.k3 = 6.84454e1;
	//irCamParams.p1 = -1.9e-3;
	//irCamParams.p2 = -4.1e-3;


	dev->setIrCameraParams(irCamParams);



	m_colorCamPams = dev->getColorCameraParams();

	m_color_fx = m_colorCamPams.fx;
	m_color_fy = m_colorCamPams.fy;
	m_color_ppx = m_colorCamPams.cx;
	m_color_ppy = m_colorCamPams.cy;

	//m_colorCamPams.fx = 9.8886340346057432e+02;
	//m_colorCamPams.fy = 9.1514224089519803e+02;
	//m_colorCamPams.cx = 1.0211721350442493e+03;
	//m_colorCamPams.cy = 5.0197760905458745e+02;

	//dev->setColorCameraParams(m_colorCamPams);

	libfreenect2::Registration* registration = new libfreenect2::Registration(dev->getIrCameraParams(), m_colorCamPams);
	libfreenect2::Frame undistorted(m_frame_width, m_frame_height, 4), registered(m_frame_width, m_frame_height, 4), bigDepth(1920, 1082, 4);
	int colorDepthIndex[512 * 424];


	while (m_status == CAPTURING)
	{
		//m_frames_ready = false;
		//std::cout << "waiting for frame" << std::endl;

		if (!listener.waitForNewFrame(frames, 10 * 1000)) // 10 sconds
		{
			std::cout << "timeout!" << std::endl;
			return;
		}
		libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
		libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
		libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];
		
		//std::cout << "obtained frame" << std::endl;

		registration->apply(rgb, depth, &undistorted, &registered, true, &bigDepth, colorDepthIndex);
		//registration->apply(rgb, depth, &undistorted, &registered);

		//m_mtx.lock();
		//cv::Mat temp1 = cv::Mat(1082, 1920, CV_32F, bigDepth.data);
		//cv::Mat temp2;
		//temp1.convertTo(temp2, CV_16UC1);


		//cv::imshow("rgb1", 50 * temp2);
		//cv::waitKey(1);

		

		memcpy_s(m_rawColor, 1920 * 1080 * 4, rgb->data, 1920 * 1080 * 4);
		memcpy_s(m_rawBigDepth, 1920 * 1082 * 4, bigDepth.data, 1920 * 1082 * 4);

		memcpy_s(m_color_frame, m_frame_width * m_frame_height * 4, registered.data, m_frame_width * m_frame_height * 4);
		memcpy_s(m_depth_frame, m_frame_width * m_frame_height * 4, undistorted.data, m_frame_width * m_frame_height * 4);
		memcpy_s(m_infra_frame, m_frame_width * m_frame_height * 4, ir->data, m_frame_width * m_frame_height * 4);
		memcpy_s(m_color_Depth_Map, m_frame_width * m_frame_height * 4, colorDepthIndex, m_frame_width * m_frame_height * 4);

		//m_mtx.unlock();

		m_frames_ready = true;

		listener.release(frames);


		//std::cout << "released frame" << std::endl;


	}

	dev->stop();
	dev->close();

	delete registration;
	delete m_rawColor;
	delete m_rawBigDepth;

	delete m_color_frame;
	delete m_depth_frame;
	delete m_infra_frame;

	delete m_color_Depth_Map;

}
