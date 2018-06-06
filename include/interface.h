#pragma once
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <iostream>

#include <thread>
#include <mutex>
#include <vector>
//#include "opencv2/opencv.hpp"

struct camPams
{
	float fx;
	float fy;
	float ppx;
	float ppy;

	float k1;
	float k2;
	float p1;
	float p2;
	float k3;
};


class Freenect2Camera
{
	enum Status
	{
		CAPTURING,
		STOPPED
	};

public:
	Freenect2Camera()
		: m_color_frame()
		, m_depth_frame()
		, m_infra_frame()
		, m_rawColor()
		, m_rawBigDepth()
		, m_frame_width(0)
		, m_frame_height(0)
		, m_frames_ready(false)
		, m_thread(nullptr)
		, m_mtx()
		, m_status(STOPPED)
	{
	}

	~Freenect2Camera()
	{
		if (m_status == CAPTURING)
		{
			stop();
		}
	}

	void start();

	void stop();

	bool ready()
	{
		return m_frames_ready;
	}

	std::vector<float> getColorCameraParameters();

	void frames(unsigned char * colorArray, float * bigDepthArray);

	void frames(unsigned char * colorArray, float * depthArray, float * infraredArray, float * bigDepthArray, int * colorDepthMapping);

	//void frames(cv::Mat &color, cv::Mat &depth, cv::Mat &infra, float & fullColor);

	//void frames(cv::Mat &color, cv::Mat &depth, cv::Mat &infra);

	//void frames(cv::Mat &color, cv::Mat &depth);

	int width()
	{
		return m_frame_width;
	}

	int height()
	{
		return m_frame_height;
	}

	camPams getDepthCamPams()
	{
		return m_depthCamPams;
	}

	void setDepthCamPams(camPams dcp)
	{
		m_depthCamPams = dcp;
	}

	/*camPams getColorCamPams()
	{
		return m_colorCamPams;
	}*/

	/*void setColorCamPams(camPams ccp)
	{
		m_colorCamPams = ccp;
	}
*/




	float fx_col()
	{
		return m_color_fx;
	}

	float fy_col()
	{
		return m_color_fy;
	}

	float ppx_col()
	{
		return m_color_ppx;
	}

	float ppy_col()
	{
		return m_color_ppy;
	}


private:
	void captureLoop();

	float* m_color_frame;
	float* m_depth_frame;
	float* m_infra_frame;
	int * m_color_Depth_Map;

	float *m_rawColor;
	float *m_rawBigDepth;

	int m_frame_width;
	int m_frame_height;

	camPams m_depthCamPams;
	//camPams m_colorCamPams;

	/*float m_depth_fx;
	float m_depth_fy;
	float m_depth_ppx;
	float m_depth_ppy;*/

	float m_color_fx;
	float m_color_fy;
	float m_color_ppx;
	float m_color_ppy;

	//float m_depth_k1;
	//float m_depth_k2;
	//float m_depth_k3;
	//float m_depth_p1;
	//float m_depth_p2;

	Status m_status;
	bool m_frames_ready;
	std::thread *m_thread;
	std::mutex m_mtx;
	libfreenect2::Freenect2Device::ColorCameraParams m_colorCamPams;
};
