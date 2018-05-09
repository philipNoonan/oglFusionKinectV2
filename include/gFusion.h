#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>


//#include "opencv2/core/utility.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"


#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <unsupported/Eigen/MatrixFunctions>
#include "eigen_utils.h"



#include <iostream>
#include <fstream>

#include <vector>
#include <list>
#include <numeric>

#include "glutils.h"
#include "glslprogram.h"

#define _USE_MATH_DEFINES
#include <math.h>

struct gFusionConfig
{
	glm::vec3 volumeSize; // size in voxels
	glm::vec3 volumeDimensions; // size in meters
	float mu;
	float maxWeight;
	float nearPlane;
	float farPlane;
	float dist_threshold;
	float normal_threshold;
	std::vector<int> iterations;
	float track_threshold;

	gFusionConfig() 
	{
		volumeSize = glm::vec3(256);
		volumeDimensions = glm::vec3(1.0f);
		//cameraParameters = glm::vec4(256.0f, 212.0f, );
		mu = 0.05f;
		maxWeight = 100.0f;
		nearPlane = 0.1f;
		farPlane = 3.0f;
		dist_threshold = 0.05f;
		normal_threshold = 0.8f;
		iterations.push_back(5);
		iterations.push_back(5);
		iterations.push_back(5);
		track_threshold = 0.5f;
	}

	float stepSize()
	{
		float minxy = glm::min(volumeDimensions.x, volumeDimensions.y);
		float minxyz = glm::min(minxy, volumeDimensions.z);

		float maxxy = glm::max(volumeSize.x, volumeSize.y);
		float maxxyz = glm::max(maxxy, volumeSize.z);

		return minxyz / maxxyz;
	}
};

struct mCubeConfig
{
	glm::uvec3 gridSize;
	glm::uvec3 gridSizeMask;
	glm::uvec3 gridSizeShift;
	GLuint numVoxels;
	glm::vec3 voxelSize;
	float isoValue;
	GLuint maxVerts;
	float activeVoxels;

	mCubeConfig()
	{
		gridSize = glm::uvec3(128, 128, 128);
		gridSizeMask = glm::uvec3(gridSize.x - 1, gridSize.y - 1, gridSize.z - 1);
		gridSizeShift = glm::uvec3(0, log2(gridSize.x), log2(gridSize.y) + log2(gridSize.z));
		numVoxels = gridSize.x * gridSize.y * gridSize.z;
		voxelSize = glm::vec3(1.0f * 1000.0f / 128.0f, 1.0f * 1000.0f / 128.0f, 1.0f * 1000.0f / 128.0f);
		isoValue = 0.0f;
		maxVerts = 128 * 128 * 128;
		
	}

};

struct gPosePair
{
	GLuint textureID;
	glm::mat4 pose;
};


struct gTrackData
{
	GLint result;
	GLfloat error;
	GLfloat J[6];
};


class gFusion
{
public:
	gFusion() {}
	~gFusion();

	gFusionConfig configuration;
	mCubeConfig mcubeConfiguration;
	glm::mat4 pose, raycastPose;

	void queryDeviceLimits();
	// load and link shaders
	void compileAndLinkShader();
	// set shader locations
	void setLocations();
	// texture setup
	void initTextures();
	// buffers setup
	void initVolume();
	// reset functions
	void Reset(glm::mat4 pose, bool deleteFlag);
	void resetVolume();
	void resetPose(glm::mat4 pose);
	void allocateBuffers();
	// depth functions
	void depthToVertex(float * depthArray);
	void vertexToNormal();
	bool Track();
	bool TrackSDF();

	// volume functions
	void integrate();
	void raycast();
	void marchingCubes();
	void intensityProjection();

	// fusion functions
	void track(int layer);
	void reduce(int layer);
	void getReduction(std::vector<float>& b, std::vector<float>& C, float & alignmentEnergy);

	// fusion sdf functions
	void trackSDF(int layer, Eigen::Matrix4f camToWorld);
	void reduceSDF(int layer);
	void getSDFReduction(std::vector<float>& b, std::vector<float>& C, float &alignmentEnergy);

	// tracking lost functions
	void updatePoseFinder();
	void addPoseToLibrary();
	bool recoverPose();

	void trackPoints3D(GLuint trackedPoints2D);



	// tracking sets and gets
	glm::mat4 getPose()
	{
		return m_pose;
	}
	void setPose(glm::mat4 pose)
	{
		m_pose = pose;
	}
	// camera parameters
	void setCameraParams(glm::vec4 camPams, glm::vec4 camPamsColor)
	{
		m_camPamsDepth = camPams;
		m_camPamsColor = camPamsColor;
	}
	void setConfig(gFusionConfig config)
	{
		configuration = config;
	}
	void setMcConfig(mCubeConfig config)
	{
		mcubeConfiguration = config;
	}
	GLuint getDepthImage()
	{
		return m_textureDepth;
	}
	GLuint getColorImage()
	{
		return m_textureColor;
	}
	GLuint getVerts()
	{
		//return m_textureVertex;

		return m_textureReferenceVertex;

	}
	GLuint getNorms()
	{
		// return m_textureNormal;

		return m_textureReferenceNormal;
	}
	GLuint getVertsMC()
	{
		return m_bufferPos;
	}
	GLuint getNormsMC()
	{
		return m_bufferNorm;
	}
	GLuint getVolume()
	{
		return m_textureVolume;
	}
	GLuint getTrackImage()
	{
		return m_textureTrackImage;
	}
	size_t getNumVerts()
	{
		return m_totalVerts;

	}


	void showNormals();
	void showRaycast();
	void showDifference();
	void showTrack();

	void testPrefixSum();
	void exportSurfaceAsStlBinary();
	void printTimes();
	void getTimes(float arr[]);

	glm::vec3 getInitialPose(float pixX, float pixY);

	void testLargeUpload();

	std::vector<float> getCumTwist()
	{
		Eigen::MatrixXf f = m_cumTwist.cast <float>();
		std::vector<float> vec(f.data(), f.data() + f.rows() * f.cols());
		return vec;
	}

	std::vector<float> getTransPose()
	{
		std::vector<float> vec(3);
		vec[0] = m_pose[3][0];
		vec[1] = m_pose[3][1];
		vec[2] = m_pose[3][2];
		return vec;
	}




private:

	// PROGRAMS
	GLSLProgram depthToVertProg;
	GLSLProgram vertToNormProg;
	GLSLProgram raycastProg;
	GLSLProgram integrateProg;
	GLSLProgram trackProg;
	GLSLProgram reduceProg;
	GLSLProgram helpersProg;
	GLSLProgram prefixSumProg;
	GLSLProgram marchingCubesProg;
	GLSLProgram trackSDFProg;
	GLSLProgram reduceSDFProg;
	GLSLProgram mipProg;

	// LOCATIONS ID
	// depthtovert
	GLuint m_invkID;
	GLuint m_camPamsID;
	// track
	GLuint m_viewID_t;
	GLuint m_TtrackID;
	GLuint m_distThresh_t;
	GLuint m_normThresh_t;
	// reduce
	GLuint m_imageSizeID;
	// integrate
	GLuint m_invTrackID;
	GLuint m_KID;
	GLuint m_muID;
	GLuint m_maxWeightID;
	GLuint m_volDimID;
	GLuint m_volSizeID;
	// raycast
	GLuint m_viewID_r;
	GLuint m_nearPlaneID;
	GLuint m_farPlaneID;
	GLuint m_stepID;
	GLuint m_largeStepID;
	GLuint m_volDimID_r;
	GLuint m_volSizeID_r;
	GLuint m_helpersSubroutineID;

	// Helpers
	GLuint m_resetVolumeID;
	GLuint m_trackPointsToVertsID;
	GLuint m_volSizeID_h;
	GLuint m_buffer2DWidthID;
	GLuint m_invKID_h;

	GLuint m_prefixSumSubroutineID;
	GLuint m_resetSumsArrayID;
	GLuint m_forEachGroupID;
	GLuint m_forEveryGroupID;
	GLuint m_forFinalIncrementalSumID;

	// marching cubes
	GLuint m_marchingCubesSubroutineID;
	GLuint m_classifyVoxelID;
	GLuint m_compactVoxelsID;
	GLuint m_generateTrianglesID;
	
	GLuint m_gridSizeID;
	GLuint m_gridSizeShiftID;
	GLuint m_gridSizeMaskID;
	GLuint m_isoValueID;
	GLuint m_numVoxelsID;
	GLuint m_activeVoxelsID;
	GLuint m_maxVertsID;
	GLuint m_voxelSizeID;

	// track sdf
	GLuint m_TtrackID_t;
	GLuint m_volDimID_t;
	GLuint m_volSizeID_t;
	GLuint m_cID;
	GLuint m_epsID;

	// track sdf
	GLuint m_imageSizeID_t_sdf;
	//reduce sdf
	GLuint m_imageSizeID_sdf;

	// intensity propjection
	GLuint m_viewID_m;
	GLuint m_nearPlaneID_m;
	GLuint m_farPlaneID_m;
	GLuint m_stepID_m;
	GLuint m_largeStepID_m;
	GLuint m_volDimID_m;
	GLuint m_volSizeID_m;

	// TEXTURES
	GLuint createTexture(GLenum target, int levels, int w, int h, int d, GLint internalformat);
	GLuint m_textureDepth;
	GLuint m_textureColor;
	GLuint m_textureVertex;
	GLuint m_textureNormal;
	GLuint m_textureReferenceVertex;
	GLuint m_textureReferenceNormal;
	GLuint m_textureOutputData;
	GLuint m_textureDifferenceVertex;
	GLuint m_textureTestImage;
	GLuint m_textureTrackImage;

	GLuint m_textureEdgeTable;
	GLuint m_textureTriTex;
	GLuint m_textureNumVertsTable;

	GLuint m_textureVolume;

	GLuint m_textureTest;

	GLuint m_textureMip;
	// BUFFERS
	std::vector<short> m_volume;
	std::vector<gTrackData> m_reduction;
	GLuint m_buffer_reduction;
	GLuint m_bufferSDFReduction;
	GLuint m_bufferSDFoutputdata;

	std::vector<float> m_outputdata;
	GLuint m_buffer_outputdata;

	GLuint m_bufferVoxelVerts;
	GLuint m_bufferVoxelVertsScan;
	GLuint m_bufferVoxelOccupied;
	GLuint m_bufferVoxelOccupiedScan;
	GLuint m_bufferCompactedVoxelArray;
	GLuint m_bufferPos;
	GLuint m_bufferNorm;

	GLuint m_buffer_testInput;
	GLuint m_buffer_testOutput;
	GLuint m_buffer_testMidput;

	GLuint m_bufferPrefixSumByGroup;

	// TRACKED POINTS
	std::vector<float> m_trackedPoints3D;
	GLuint m_trackedPoints3DBuffer;


	// POSE RECOVERY
	std::vector<gPosePair> poseLibrary;

	Eigen::Matrix<double, 6, 1> m_cumTwist;




	int m_depth_height = 424; // set these properly from main?
	int m_depth_width = 512;
	int m_color_height = 1080;
	int m_color_width = 1920;


	// TRACKING DATA
	// glm::vec3 m_volSize = glm::vec3(256,256,256);
	glm::mat4 m_K;
	glm::mat4 m_invK;
	glm::vec4 m_camPamsDepth;
	glm::vec4 m_camPamsColor;
	glm::mat4 m_pose;

	Eigen::Matrix4f m_pose_eig;// = Eigen::MatrixXf::Identity(4, 4);





	glm::mat4 getInverseCameraMatrix(const glm::vec4 & k) { // [col][row]
		glm::mat4 invK;
		invK[0][0] = 1.0f / k.x;	invK[1][0] = 0;				invK[2][0] = -k.z / k.x;	invK[3][0] = 0;
		invK[0][1] = 0;				invK[1][1] = 1.0f / k.y;	invK[2][1] = -k.w / k.y;	invK[3][1] = 0;
		invK[0][2] = 0;				invK[1][2] = 0;				invK[2][2] = 1;				invK[3][2] = 0;
		invK[0][3] = 0;				invK[1][3] = 0;				invK[2][3] = 0;				invK[3][3] = 1;

		return invK;
	}

	glm::mat4 getCameraMatrix(const glm::vec4 & k) {
		glm::mat4 K;

		K[0][0] = k.x;	K[1][0] = 0;	K[2][0] = k.z;	K[3][0] = 0;
		K[0][1] = 0;	K[1][1] = k.y;	K[2][1] = k.w;	K[3][1] = 0;
		K[0][2] = 0;	K[1][2] = 0;	K[2][2] = 1;	K[3][2] = 0;
		K[0][3] = 0;	K[1][3] = 0;	K[2][3] = 0;	K[3][3] = 1;

		return K;
	}

	std::vector<float> makeJTJ(std::vector<float> v)
	{
		// C is a 6 x 6 matrix (essentially)
		// here we copy the triangluar matrix v into C corner, but we do the mirror in here at the same time, because we can??
		std::vector<float> C;
		C.resize(6 * 6);

		C[0] = v[0];	C[1] = v[1];	C[2] = v[2];	C[3] = v[3];	C[4] = v[4];	C[5] = v[5];
		C[6] = v[1];	C[7] = v[6];	C[8] = v[7];	C[9] = v[8];	C[10] = v[9];	C[11] = v[10];
		C[12] = v[2];	C[13] = v[7];	C[14] = v[11];	C[15] = v[12];	C[16] = v[13];	C[17] = v[14];
		C[18] = v[3];	C[19] = v[8];	C[20] = v[12];	C[21] = v[15];	C[22] = v[16];	C[23] = v[17];
		C[24] = v[4];	C[25] = v[9];	C[26] = v[13];	C[27] = v[16];	C[28] = v[18];	C[29] = v[19];
		C[30] = v[5];	C[31] = v[10];	C[32] = v[14];	C[33] = v[17];	C[34] = v[19];	C[35] = v[20];

		return C;
	}

	// NOT USED
	static inline Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rodrigues(const Eigen::Vector3d & src)
	{
		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> dst = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Identity();

		double rx, ry, rz, theta;

		rx = src(0);
		ry = src(1);
		rz = src(2);

		theta = src.norm();

		if (theta >= DBL_EPSILON)
		{
			const double I[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

			double c = cos(theta);
			double s = sin(theta);
			double c1 = 1. - c;
			double itheta = theta ? 1. / theta : 0.;

			rx *= itheta; ry *= itheta; rz *= itheta;

			double rrt[] = { rx*rx, rx*ry, rx*rz, rx*ry, ry*ry, ry*rz, rx*rz, ry*rz, rz*rz };
			double _r_x_[] = { 0, -rz, ry, rz, 0, -rx, -ry, rx, 0 };
			double R[9];

			for (int k = 0; k < 9; k++)
			{
				R[k] = c*I[k] + c1*rrt[k] + s*_r_x_[k];
			}

			memcpy(dst.data(), &R[0], sizeof(Eigen::Matrix<double, 3, 3, Eigen::RowMajor>));
		}

		return dst;
	}

	// NOT USED
	static inline void computeUpdateSE3(Eigen::Matrix<double, 4, 4, Eigen::RowMajor> & resultRt, const Eigen::Matrix<double, 6, 1> & result, Eigen::Isometry3f & rgbOdom)
	{
		// for infinitesimal transformation
		Eigen::Matrix<double, 4, 4, Eigen::RowMajor> Rt = Eigen::Matrix<double, 4, 4, Eigen::RowMajor>::Identity();

		Eigen::Vector3d rvec(result(3), result(4), result(5));

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> R = rodrigues(rvec);

		Rt.topLeftCorner(3, 3) = R;
		Rt(0, 3) = result(0);
		Rt(1, 3) = result(1);
		Rt(2, 3) = result(2);

		resultRt = Rt * resultRt;

		Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rotation = resultRt.topLeftCorner(3, 3);
		rgbOdom.setIdentity();
		rgbOdom.rotate(rotation.cast<float>().eval());
		rgbOdom.translation() = resultRt.cast<float>().eval().topRightCorner(3, 1);

	}

	inline int divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }




	std::vector<float> edgeTable;
	std::vector<float> triTex;
	std::vector<float> numVertsTable;
	
	uint32_t m_totalVerts;

	




	// MARCHING CUBES
	// SET UP TABLES TEXTURES
	void setTablesForMarchingCubes();
	/*{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, m_textureEdgeTable);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RED, GL_UNSIGNED_BYTE, &edgeTable[0]);

		glBindTexture(GL_TEXTURE_1D, m_textureTriTex);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256 * 16, GL_RED, GL_UNSIGNED_BYTE, &triTable[0]);

		glBindTexture(GL_TEXTURE_1D, m_textureNumVertsTable);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RED, GL_UNSIGNED_BYTE, &numVertsTable[0]);

		std::vector<uint> tableOut;
		tableOut.resize(256);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureNumVertsTable);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_R8UI, tableOut.data());
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(0);


	}*/

	std::vector<uint32_t> debugArray;

	GLuint  prefixSum(GLuint inputBuffer, GLuint outputBuffer);


	double raycastTime;
	double marchingCubesTime;
	double trackTime;
	double reduceTime;
	double integrateTime;
	double trackSDFTime;
	double reduceSDFTime;

	// output from get reudction
	std::vector<float> outputData;

	GLuint query[7];

	void getPreRedu(Eigen::Matrix<double, 6, 6> &A, Eigen::Matrix<double, 6, 1> &b);

	//cv::Mat testIm = cv::Mat(424, 512, CV_32FC4);
	//cv::Mat testIm2 = cv::Mat(424, 512, CV_32FC4);

	
	GLuint m_lmbuff_0;

	GLuint m_lmbuff_1;

	std::vector<int> listmode;
	//cv::Mat mipMat = cv::Mat(424,512, CV_32FC4);

};
