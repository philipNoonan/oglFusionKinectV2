


#include "gFusion.h"

gFusion::~gFusion()
{
}

void gFusion::queryDeviceLimits()
{
	GLint sizeSSBO;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &sizeSSBO);
	std::cout << "GL_MAX_SHADER_STORAGE_BLOCK_SIZE is " << sizeSSBO << " bytes." << std::endl;

	GLint sizeCWGI;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &sizeCWGI);
	std::cout << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS is " << sizeCWGI << " invocations." << std::endl;

	GLint size3D;
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &size3D);
	std::cout << "GL_MAX_3D_TEXTURE_SIZE is " << size3D << " texels." << std::endl;

}

void gFusion::compileAndLinkShader()
{
	try {
		depthToVertProg.compileShader("shaders/depthToVert.cs");
		depthToVertProg.link();
		
		vertToNormProg.compileShader("shaders/vertToNorm.cs");
		vertToNormProg.link();

		trackProg.compileShader("shaders/track.cs");
		trackProg.link();

		reduceProg.compileShader("shaders/reduce.cs");
		reduceProg.link();

		integrateProg.compileShader("shaders/integrate.cs");
		integrateProg.link();

		raycastProg.compileShader("shaders/raycast.cs");
		raycastProg.link();

		marchingCubesProg.compileShader("shaders/marchingCubes.cs");
		marchingCubesProg.link();

		prefixSumProg.compileShader("shaders/prefixSum.cs");
		prefixSumProg.link();

		trackSDFProg.compileShader("shaders/trackSDF.cs");
		trackSDFProg.link();

		reduceSDFProg.compileShader("shaders/reduceSDF.cs");
		reduceSDFProg.link();

		mipProg.compileShader("shaders/mip.cs");
		mipProg.link();
		
		
		helpersProg.compileShader("shaders/helpers.cs");
		helpersProg.link();


		
	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	//glGenQueries(7, query);

}

void gFusion::setLocations()
{
	m_invkID = glGetUniformLocation(depthToVertProg.getHandle(), "invK");
	m_camPamsID = glGetUniformLocation(depthToVertProg.getHandle(), "camPams");

	m_viewID_t = glGetUniformLocation(trackProg.getHandle(), "view");
	m_TtrackID = glGetUniformLocation(trackProg.getHandle(), "Ttrack");
	m_distThresh_t = glGetUniformLocation(trackProg.getHandle(), "dist_threshold");
	m_normThresh_t = glGetUniformLocation(trackProg.getHandle(), "normal_threshold");

	m_imageSizeID = glGetUniformLocation(reduceProg.getHandle(), "imageSize");

	m_invTrackID = glGetUniformLocation(integrateProg.getHandle(), "invTrack");
	m_KID = glGetUniformLocation(integrateProg.getHandle(), "K");
	m_muID = glGetUniformLocation(integrateProg.getHandle(), "mu");
	m_maxWeightID = glGetUniformLocation(integrateProg.getHandle(), "maxWeight");
	m_volDimID = glGetUniformLocation(integrateProg.getHandle(), "volDim");
	m_volSizeID = glGetUniformLocation(integrateProg.getHandle(), "volSize");

	m_viewID_r = glGetUniformLocation(raycastProg.getHandle(), "view");
	m_nearPlaneID = glGetUniformLocation(raycastProg.getHandle(), "nearPlane");
	m_farPlaneID = glGetUniformLocation(raycastProg.getHandle(), "farPlane");
	m_stepID = glGetUniformLocation(raycastProg.getHandle(), "step");
	m_largeStepID = glGetUniformLocation(raycastProg.getHandle(), "largeStep");
	m_volDimID_r = glGetUniformLocation(raycastProg.getHandle(), "volDim");
	m_volSizeID_r = glGetUniformLocation(raycastProg.getHandle(), "volSize");

	//HELPERS
	m_helpersSubroutineID = glGetSubroutineUniformLocation(helpersProg.getHandle(), GL_COMPUTE_SHADER, "performHelperFunction");
	m_resetVolumeID = glGetSubroutineIndex(helpersProg.getHandle(), GL_COMPUTE_SHADER, "resetVolume");
	m_trackPointsToVertsID = glGetSubroutineIndex(helpersProg.getHandle(), GL_COMPUTE_SHADER, "trackPointsToVerts");
	m_volSizeID_h = glGetUniformLocation(helpersProg.getHandle(), "volSize");
	m_buffer2DWidthID = glGetUniformLocation(helpersProg.getHandle(), "buffer2DWidth");
	m_invKID_h = glGetUniformLocation(helpersProg.getHandle(), "invK");

	// PREFIX SUMS
	m_prefixSumSubroutineID = glGetSubroutineUniformLocation(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "getPrefixSum");
	m_resetSumsArrayID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "resetSumsArray");

	m_forEachGroupID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forEachGroup");
	m_forEveryGroupID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forEveryGroup");
	m_forFinalIncrementalSumID = glGetSubroutineIndex(prefixSumProg.getHandle(), GL_COMPUTE_SHADER, "forFinalIncrementalSum");

	// MARCHING CUBES
	m_marchingCubesSubroutineID = glGetSubroutineUniformLocation(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "doMarchingCubes");
	m_classifyVoxelID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchClassifyVoxel");
	m_compactVoxelsID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchCompactVoxels");
	m_generateTrianglesID = glGetSubroutineIndex(marchingCubesProg.getHandle(), GL_COMPUTE_SHADER, "launchGenerateTriangles");

	m_gridSizeID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSize");
	m_gridSizeShiftID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSizeShift");
	m_gridSizeMaskID = glGetUniformLocation(marchingCubesProg.getHandle(), "gridSizeMask");
	m_isoValueID = glGetUniformLocation(marchingCubesProg.getHandle(), "isoValue");
	m_numVoxelsID = glGetUniformLocation(marchingCubesProg.getHandle(), "numVoxels");
	m_activeVoxelsID = glGetUniformLocation(marchingCubesProg.getHandle(), "activeVoxels");
	m_maxVertsID = glGetUniformLocation(marchingCubesProg.getHandle(), "maxVerts");
	m_voxelSizeID = glGetUniformLocation(marchingCubesProg.getHandle(), "voxelSize");

	// TRACKSDF
	m_TtrackID_t = glGetUniformLocation(trackSDFProg.getHandle(), "Ttrack");
	m_volDimID_t = glGetUniformLocation(trackSDFProg.getHandle(), "volDim");
	m_volSizeID_t = glGetUniformLocation(trackSDFProg.getHandle(), "volSize");
	m_cID = glGetUniformLocation(trackSDFProg.getHandle(), "c");
	m_epsID = glGetUniformLocation(trackSDFProg.getHandle(), "eps");
	m_imageSizeID_t_sdf = glGetUniformLocation(trackSDFProg.getHandle(), "imageSize");
	// REDUCE SDF
	m_imageSizeID_sdf = glGetUniformLocation(reduceSDFProg.getHandle(), "imageSize");

	//INTENSITY PROJECTION
	m_viewID_m = glGetUniformLocation(mipProg.getHandle(), "view");
	m_nearPlaneID_m = glGetUniformLocation(mipProg.getHandle(), "nearPlane");
	m_farPlaneID_m = glGetUniformLocation(mipProg.getHandle(), "farPlane");
	m_stepID_m = glGetUniformLocation(mipProg.getHandle(), "step");
	m_largeStepID_m = glGetUniformLocation(mipProg.getHandle(), "largeStep");
	m_volDimID_m = glGetUniformLocation(mipProg.getHandle(), "volDim");
	m_volSizeID_m = glGetUniformLocation(mipProg.getHandle(), "volSize");

	//listmode.resize(402653184, 0);
	//glGenBuffers(1, &m_lmbuff_0);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lmbuff_0);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, 402653184 * sizeof(int), &listmode[0], GL_DYNAMIC_STORAGE_BIT);

	//glGenBuffers(1, &m_lmbuff_1);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lmbuff_1);
	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, 402653184 * sizeof(int) / 2, &listmode[0], GL_DYNAMIC_STORAGE_BIT);

	//glBufferData(GL_SHADER_STORAGE_BUFFER, 402653184 * sizeof(int), &listmode[0], GL_DYNAMIC_DRAW);




}

GLuint gFusion::createTexture(GLenum target, int levels, int w, int h, int d, GLint internalformat)
{
	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(target, texid);

	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// https://stackoverflow.com/questions/15405869/is-gltexstorage2d-imperative-when-auto-generating-mipmaps
	//glTexImage2D(target, 0, internalformat, w, h, 0, format, type, 0); // cretes mutable storage that requires glTexImage2D
	
	if (target == GL_TEXTURE_1D)
	{
		glTexStorage1D(target, levels, internalformat, w);
	}
	else if (target == GL_TEXTURE_2D)
	{
		glTexStorage2D(target, levels, internalformat, w, h); // creates immutable storage and requires glTexSubImage2D
	}
	else if (target == GL_TEXTURE_3D)
	{
		glTexStorage3D(target, levels, internalformat, w, h, d);
	}
	return texid;
}

void gFusion::initTextures()
{
	m_textureDepth = createTexture(GL_TEXTURE_2D, 3, m_depth_width, m_depth_height, 1, GL_R32F);
	m_textureColor = createTexture(GL_TEXTURE_2D, 1, m_color_width, m_color_height, 1, GL_RGBA8UI);
	m_textureVertex = createTexture(GL_TEXTURE_2D, 3, m_depth_width, m_depth_height, 1, GL_RGBA32F);
	m_textureNormal = createTexture(GL_TEXTURE_2D, 3, m_depth_width, m_depth_height, 1, GL_RGBA32F);
	m_textureReferenceVertex = createTexture(GL_TEXTURE_2D, 1, m_depth_width, m_depth_height, 1, GL_RGBA32F);
	m_textureReferenceNormal = createTexture(GL_TEXTURE_2D, 1, m_depth_width, m_depth_height, 1, GL_RGBA32F);
	m_textureDifferenceVertex = createTexture(GL_TEXTURE_2D, 1, m_depth_width, m_depth_height, 1, GL_R32F);
	m_textureTestImage = createTexture(GL_TEXTURE_2D, 1, m_depth_width, m_depth_height, 1, GL_RGBA32F);
	m_textureTrackImage = createTexture(GL_TEXTURE_2D, 1, m_depth_width, m_depth_height, 1, GL_RGBA32F);
	m_textureTest = createTexture(GL_TEXTURE_2D, 1, m_depth_width, m_depth_height, 1, GL_RGBA32F);
	m_textureMip = createTexture(GL_TEXTURE_2D, 1, m_depth_width, m_depth_height, 1, GL_RGBA32F);

	// MARCHING CUBES TEXTURES
	m_textureEdgeTable = createTexture(GL_TEXTURE_1D, 1, 256, 1, 1, GL_R32F);
	m_textureTriTex = createTexture(GL_TEXTURE_1D, 1, 256 * 16, 1, 1, GL_R32F);
	m_textureNumVertsTable = createTexture(GL_TEXTURE_1D, 1, 256, 1, 1, GL_R32F);
	setTablesForMarchingCubes();
}

void gFusion::initVolume()
{
	m_textureVolume = createTexture(GL_TEXTURE_3D, 1, configuration.volumeSize.x, configuration.volumeSize.y, configuration.volumeSize.z, GL_RG16I);
}

void gFusion::Reset(glm::mat4 pose, bool deleteFlag)
{
	if (deleteFlag)
	{
		glDeleteTextures(1, &m_textureVolume);
		initVolume();
	}
	else
	{
		resetVolume();
	}

	poseLibrary.resize(0);
	resetPose(pose);
}

void gFusion::resetVolume()
{
	helpersProg.use();

	int compWidth;
	int compHeight;

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16I);
	//glBindImageTexture(1, m_textureTestImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resetVolumeID);
	glUniform3fv(m_volSizeID_h, 1, glm::value_ptr(configuration.volumeSize));


	compWidth = divup(configuration.volumeSize.x, 32);
	compHeight = divup(configuration.volumeSize.y, 32);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//cv::Mat test = cv::Mat(configuration.volumeSize.x, configuration.volumeSize.y, CV_32FC1);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureTestImage);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, test.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(0);

	//cv::imshow("test", test);


}

void gFusion::resetPose(glm::mat4 pose)
{
	m_pose = pose;

	m_pose_eig = Eigen::MatrixXf::Identity(4, 4);

	m_pose_eig(0, 3) = pose[3][0];
	m_pose_eig(1, 3) = pose[3][1];
	m_pose_eig(2, 3) = pose[3][2];

	m_cumTwist << 0, 0, 0, 0, 0, 0;

}


void gFusion::allocateBuffers()
{

	// REDUCTION BUFFER OBJECT
	m_reduction.resize(m_depth_height * m_depth_width);
	size_t reductionSize = m_depth_height * m_depth_width * (sizeof(GLint) + (sizeof(GLfloat) * 7)); // this is the size of one reduction element 1 int + 1 float + 6 float

	glGenBuffers(1, &m_buffer_reduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer_reduction);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reductionSize, &m_reduction[0], GL_STATIC_DRAW);

	// OUTPUT DATA FROM REDUCTION BUFFER OBJECT
	m_outputdata.resize(32 * 8);
	glGenBuffers(1, &m_buffer_outputdata);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffer_outputdata);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * 8 * sizeof(float), &m_outputdata[0], GL_STATIC_DRAW);

	// MARCHING CUBES BUFFERS
	size_t memSize = sizeof(GLuint) * mcubeConfiguration.numVoxels;
	size_t memSizeVec4 = sizeof(float) * 4 * mcubeConfiguration.maxVerts;

	glGenBuffers(1, &m_bufferVoxelVerts);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferVoxelVerts);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);
	
	glGenBuffers(1, &m_bufferVoxelOccupied);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_bufferVoxelOccupiedScan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);
	
	glGenBuffers(1, &m_bufferCompactedVoxelArray);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);
	
	glGenBuffers(1, &m_bufferVoxelVertsScan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_bufferVoxelVertsScan);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_bufferPos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferPos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSizeVec4, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_bufferNorm);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_bufferNorm);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSizeVec4, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &m_bufferPrefixSumByGroup);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_bufferPrefixSumByGroup);
	glBufferData(GL_SHADER_STORAGE_BUFFER, memSize / 1024, NULL, GL_DYNAMIC_DRAW);



	size_t reductionSDFSize = m_depth_height * m_depth_width * (sizeof(GLfloat) * 8); // this is the size of one reduction element 1 float + 1 float + 6 float
	glGenBuffers(1, &m_bufferSDFReduction);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bufferSDFReduction);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reductionSDFSize, NULL, GL_STATIC_DRAW);

	// OUTPUT DATA FROM REDUCTION BUFFER OBJECT
	glGenBuffers(1, &m_bufferSDFoutputdata);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, m_bufferSDFoutputdata);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 32 * 8 * sizeof(float), NULL, GL_STATIC_DRAW);



	m_trackedPoints3D.resize(m_depth_height * m_depth_height * 3);
	int xOffTrack = m_depth_height / 2;
	int yOffTrack = m_depth_height / 2;
	int xSpacing = 2 * xOffTrack / m_depth_height;
	int ySpacing = 2 * yOffTrack / m_depth_height;

	for (int i = 0; i < m_depth_height * 3; i += 3)
	{
		for (int j = 0; j < m_depth_height; j++)
		{
			m_trackedPoints3D[j * m_depth_height * 2 + i] = (m_depth_width >> 1) - xOffTrack + (i / 3) * xSpacing;
			m_trackedPoints3D[j * m_depth_height * 2 + i + 1] = (m_depth_width >> 1) - yOffTrack + j * ySpacing;
			m_trackedPoints3D[j * m_depth_height * 2 + i + 2] = 0.0f;


		}
	}

	glGenBuffers(1, &m_trackedPoints3DBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_trackedPoints3DBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_trackedPoints3D.size() * sizeof(float), m_trackedPoints3D.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, m_trackedPoints3DBuffer);




}

void gFusion::getTimes(float arr[])
{
	arr[1] = raycastTime;
	arr[2] = marchingCubesTime;
	arr[3] = trackTime;
	arr[4] = reduceTime;
	arr[5] = integrateTime;
	arr[6] = trackSDFTime;
	arr[7] = reduceSDFTime;

}

void gFusion::printTimes()
{


	std::cout << "raycast " << raycastTime << " ms " << std::endl;
	std::cout << "marchingcubes " << marchingCubesTime << " ms " << std::endl;
	std::cout << "track " << trackTime << " ms " << std::endl;
	std::cout << "reduce " << reduceTime << " ms " << std::endl;
	std::cout << "integrate " << integrateTime << " ms " << std::endl;

}

void gFusion::setTablesForMarchingCubes()
{
	edgeTable.resize(256);
	edgeTable = { 0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
					0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
					0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
					0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
					0x230, 0x339, 0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
					0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
					0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
					0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
					0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c,
					0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
					0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff, 0x3f5, 0x2fc,
					0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
					0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
					0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
					0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
					0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
					0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
					0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
					0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
					0x15c, 0x55, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
					0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
					0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
					0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
					0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460,
					0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
					0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa, 0x1a3, 0x2a9, 0x3a0,
					0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
					0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
					0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
					0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190,
					0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
					0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };

#define X 255
	triTex.resize(256 * 16);
	triTex = {
		X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		0, 8, 3, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		0, 1, 9, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		1, 8, 3, 9, 8, 1, X, X, X, X, X, X, X, X, X, X  ,
		1, 2, 10, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		0, 8, 3, 1, 2, 10, X, X, X, X, X, X, X, X, X, X  ,
		9, 2, 10, 0, 2, 9, X, X, X, X, X, X, X, X, X, X  ,
		2, 8, 3, 2, 10, 8, 10, 9, 8, X, X, X, X, X, X, X  ,
		3, 11, 2, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		0, 11, 2, 8, 11, 0, X, X, X, X, X, X, X, X, X, X  ,
		1, 9, 0, 2, 3, 11, X, X, X, X, X, X, X, X, X, X  ,
		1, 11, 2, 1, 9, 11, 9, 8, 11, X, X, X, X, X, X, X  ,
		3, 10, 1, 11, 10, 3, X, X, X, X, X, X, X, X, X, X  ,
		0, 10, 1, 0, 8, 10, 8, 11, 10, X, X, X, X, X, X, X  ,
		3, 9, 0, 3, 11, 9, 11, 10, 9, X, X, X, X, X, X, X  ,
		9, 8, 10, 10, 8, 11, X, X, X, X, X, X, X, X, X, X  ,
		4, 7, 8, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		4, 3, 0, 7, 3, 4, X, X, X, X, X, X, X, X, X, X  ,
		0, 1, 9, 8, 4, 7, X, X, X, X, X, X, X, X, X, X  ,
		4, 1, 9, 4, 7, 1, 7, 3, 1, X, X, X, X, X, X, X  ,
		1, 2, 10, 8, 4, 7, X, X, X, X, X, X, X, X, X, X  ,
		3, 4, 7, 3, 0, 4, 1, 2, 10, X, X, X, X, X, X, X  ,
		9, 2, 10, 9, 0, 2, 8, 4, 7, X, X, X, X, X, X, X  ,
		2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, X, X, X, X  ,
		8, 4, 7, 3, 11, 2, X, X, X, X, X, X, X, X, X, X  ,
		11, 4, 7, 11, 2, 4, 2, 0, 4, X, X, X, X, X, X, X  ,
		9, 0, 1, 8, 4, 7, 2, 3, 11, X, X, X, X, X, X, X  ,
		4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, X, X, X, X  ,
		3, 10, 1, 3, 11, 10, 7, 8, 4, X, X, X, X, X, X, X  ,
		1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, X, X, X, X  ,
		4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, X, X, X, X  ,
		4, 7, 11, 4, 11, 9, 9, 11, 10, X, X, X, X, X, X, X  ,
		9, 5, 4, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		9, 5, 4, 0, 8, 3, X, X, X, X, X, X, X, X, X, X  ,
		0, 5, 4, 1, 5, 0, X, X, X, X, X, X, X, X, X, X  ,
		8, 5, 4, 8, 3, 5, 3, 1, 5, X, X, X, X, X, X, X  ,
		1, 2, 10, 9, 5, 4, X, X, X, X, X, X, X, X, X, X  ,
		3, 0, 8, 1, 2, 10, 4, 9, 5, X, X, X, X, X, X, X  ,
		5, 2, 10, 5, 4, 2, 4, 0, 2, X, X, X, X, X, X, X  ,
		2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, X, X, X, X  ,
		9, 5, 4, 2, 3, 11, X, X, X, X, X, X, X, X, X, X  ,
		0, 11, 2, 0, 8, 11, 4, 9, 5, X, X, X, X, X, X, X  ,
		0, 5, 4, 0, 1, 5, 2, 3, 11, X, X, X, X, X, X, X  ,
		2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, X, X, X, X  ,
		10, 3, 11, 10, 1, 3, 9, 5, 4, X, X, X, X, X, X, X  ,
		4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, X, X, X, X  ,
		5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, X, X, X, X  ,
		5, 4, 8, 5, 8, 10, 10, 8, 11, X, X, X, X, X, X, X  ,
		9, 7, 8, 5, 7, 9, X, X, X, X, X, X, X, X, X, X  ,
		9, 3, 0, 9, 5, 3, 5, 7, 3, X, X, X, X, X, X, X  ,
		0, 7, 8, 0, 1, 7, 1, 5, 7, X, X, X, X, X, X, X  ,
		1, 5, 3, 3, 5, 7, X, X, X, X, X, X, X, X, X, X  ,
		9, 7, 8, 9, 5, 7, 10, 1, 2, X, X, X, X, X, X, X  ,
		10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, X, X, X, X  ,
		8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, X, X, X, X  ,
		2, 10, 5, 2, 5, 3, 3, 5, 7, X, X, X, X, X, X, X  ,
		7, 9, 5, 7, 8, 9, 3, 11, 2, X, X, X, X, X, X, X  ,
		9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, X, X, X, X  ,
		2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, X, X, X, X  ,
		11, 2, 1, 11, 1, 7, 7, 1, 5, X, X, X, X, X, X, X  ,
		9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, X, X, X, X  ,
		5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, X  ,
		11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, X  ,
		11, 10, 5, 7, 11, 5, X, X, X, X, X, X, X, X, X, X  ,
		10, 6, 5, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		0, 8, 3, 5, 10, 6, X, X, X, X, X, X, X, X, X, X  ,
		9, 0, 1, 5, 10, 6, X, X, X, X, X, X, X, X, X, X  ,
		1, 8, 3, 1, 9, 8, 5, 10, 6, X, X, X, X, X, X, X  ,
		1, 6, 5, 2, 6, 1, X, X, X, X, X, X, X, X, X, X  ,
		1, 6, 5, 1, 2, 6, 3, 0, 8, X, X, X, X, X, X, X  ,
		9, 6, 5, 9, 0, 6, 0, 2, 6, X, X, X, X, X, X, X  ,
		5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, X, X, X, X  ,
		2, 3, 11, 10, 6, 5, X, X, X, X, X, X, X, X, X, X  ,
		11, 0, 8, 11, 2, 0, 10, 6, 5, X, X, X, X, X, X, X  ,
		0, 1, 9, 2, 3, 11, 5, 10, 6, X, X, X, X, X, X, X  ,
		5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, X, X, X, X  ,
		6, 3, 11, 6, 5, 3, 5, 1, 3, X, X, X, X, X, X, X  ,
		0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, X, X, X, X  ,
		3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, X, X, X, X  ,
		6, 5, 9, 6, 9, 11, 11, 9, 8, X, X, X, X, X, X, X  ,
		5, 10, 6, 4, 7, 8, X, X, X, X, X, X, X, X, X, X  ,
		4, 3, 0, 4, 7, 3, 6, 5, 10, X, X, X, X, X, X, X  ,
		1, 9, 0, 5, 10, 6, 8, 4, 7, X, X, X, X, X, X, X  ,
		10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, X, X, X, X  ,
		6, 1, 2, 6, 5, 1, 4, 7, 8, X, X, X, X, X, X, X  ,
		1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, X, X, X, X  ,
		8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, X, X, X, X  ,
		7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, X  ,
		3, 11, 2, 7, 8, 4, 10, 6, 5, X, X, X, X, X, X, X  ,
		5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, X, X, X, X  ,
		0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, X, X, X, X  ,
		9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, X  ,
		8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, X, X, X, X  ,
		5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, X  ,
		0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, X  ,
		6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, X, X, X, X  ,
		10, 4, 9, 6, 4, 10, X, X, X, X, X, X, X, X, X, X  ,
		4, 10, 6, 4, 9, 10, 0, 8, 3, X, X, X, X, X, X, X  ,
		10, 0, 1, 10, 6, 0, 6, 4, 0, X, X, X, X, X, X, X  ,
		8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, X, X, X, X  ,
		1, 4, 9, 1, 2, 4, 2, 6, 4, X, X, X, X, X, X, X  ,
		3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, X, X, X, X  ,
		0, 2, 4, 4, 2, 6, X, X, X, X, X, X, X, X, X, X  ,
		8, 3, 2, 8, 2, 4, 4, 2, 6, X, X, X, X, X, X, X  ,
		10, 4, 9, 10, 6, 4, 11, 2, 3, X, X, X, X, X, X, X  ,
		0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, X, X, X, X  ,
		3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, X, X, X, X  ,
		6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, X  ,
		9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, X, X, X, X  ,
		8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, X  ,
		3, 11, 6, 3, 6, 0, 0, 6, 4, X, X, X, X, X, X, X  ,
		6, 4, 8, 11, 6, 8, X, X, X, X, X, X, X, X, X, X  ,
		7, 10, 6, 7, 8, 10, 8, 9, 10, X, X, X, X, X, X, X  ,
		0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, X, X, X, X  ,
		10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, X, X, X, X  ,
		10, 6, 7, 10, 7, 1, 1, 7, 3, X, X, X, X, X, X, X  ,
		1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, X, X, X, X  ,
		2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, X  ,
		7, 8, 0, 7, 0, 6, 6, 0, 2, X, X, X, X, X, X, X  ,
		7, 3, 2, 6, 7, 2, X, X, X, X, X, X, X, X, X, X  ,
		2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, X, X, X, X  ,
		2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, X  ,
		1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, X  ,
		11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, X, X, X, X  ,
		8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, X  ,
		0, 9, 1, 11, 6, 7, X, X, X, X, X, X, X, X, X, X  ,
		7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, X, X, X, X  ,
		7, 11, 6, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		7, 6, 11, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		3, 0, 8, 11, 7, 6, X, X, X, X, X, X, X, X, X, X  ,
		0, 1, 9, 11, 7, 6, X, X, X, X, X, X, X, X, X, X  ,
		8, 1, 9, 8, 3, 1, 11, 7, 6, X, X, X, X, X, X, X  ,
		10, 1, 2, 6, 11, 7, X, X, X, X, X, X, X, X, X, X  ,
		1, 2, 10, 3, 0, 8, 6, 11, 7, X, X, X, X, X, X, X  ,
		2, 9, 0, 2, 10, 9, 6, 11, 7, X, X, X, X, X, X, X  ,
		6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, X, X, X, X  ,
		7, 2, 3, 6, 2, 7, X, X, X, X, X, X, X, X, X, X  ,
		7, 0, 8, 7, 6, 0, 6, 2, 0, X, X, X, X, X, X, X  ,
		2, 7, 6, 2, 3, 7, 0, 1, 9, X, X, X, X, X, X, X  ,
		1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, X, X, X, X  ,
		10, 7, 6, 10, 1, 7, 1, 3, 7, X, X, X, X, X, X, X  ,
		10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, X, X, X, X  ,
		0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, X, X, X, X  ,
		7, 6, 10, 7, 10, 8, 8, 10, 9, X, X, X, X, X, X, X  ,
		6, 8, 4, 11, 8, 6, X, X, X, X, X, X, X, X, X, X  ,
		3, 6, 11, 3, 0, 6, 0, 4, 6, X, X, X, X, X, X, X  ,
		8, 6, 11, 8, 4, 6, 9, 0, 1, X, X, X, X, X, X, X  ,
		9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, X, X, X, X  ,
		6, 8, 4, 6, 11, 8, 2, 10, 1, X, X, X, X, X, X, X  ,
		1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, X, X, X, X  ,
		4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, X, X, X, X  ,
		10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, X  ,
		8, 2, 3, 8, 4, 2, 4, 6, 2, X, X, X, X, X, X, X  ,
		0, 4, 2, 4, 6, 2, X, X, X, X, X, X, X, X, X, X  ,
		1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, X, X, X, X  ,
		1, 9, 4, 1, 4, 2, 2, 4, 6, X, X, X, X, X, X, X  ,
		8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, X, X, X, X  ,
		10, 1, 0, 10, 0, 6, 6, 0, 4, X, X, X, X, X, X, X  ,
		4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, X  ,
		10, 9, 4, 6, 10, 4, X, X, X, X, X, X, X, X, X, X  ,
		4, 9, 5, 7, 6, 11, X, X, X, X, X, X, X, X, X, X  ,
		0, 8, 3, 4, 9, 5, 11, 7, 6, X, X, X, X, X, X, X  ,
		5, 0, 1, 5, 4, 0, 7, 6, 11, X, X, X, X, X, X, X  ,
		11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, X, X, X, X  ,
		9, 5, 4, 10, 1, 2, 7, 6, 11, X, X, X, X, X, X, X  ,
		6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, X, X, X, X  ,
		7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, X, X, X, X  ,
		3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, X  ,
		7, 2, 3, 7, 6, 2, 5, 4, 9, X, X, X, X, X, X, X  ,
		9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, X, X, X, X  ,
		3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, X, X, X, X  ,
		6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, X  ,
		9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, X, X, X, X  ,
		1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, X  ,
		4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, X  ,
		7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, X, X, X, X  ,
		6, 9, 5, 6, 11, 9, 11, 8, 9, X, X, X, X, X, X, X  ,
		3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, X, X, X, X  ,
		0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, X, X, X, X  ,
		6, 11, 3, 6, 3, 5, 5, 3, 1, X, X, X, X, X, X, X  ,
		1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, X, X, X, X  ,
		0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, X  ,
		11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, X  ,
		6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, X, X, X, X  ,
		5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, X, X, X, X  ,
		9, 5, 6, 9, 6, 0, 0, 6, 2, X, X, X, X, X, X, X  ,
		1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, X  ,
		1, 5, 6, 2, 1, 6, X, X, X, X, X, X, X, X, X, X  ,
		1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, X  ,
		10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, X, X, X, X  ,
		0, 3, 8, 5, 6, 10, X, X, X, X, X, X, X, X, X, X  ,
		10, 5, 6, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		11, 5, 10, 7, 5, 11, X, X, X, X, X, X, X, X, X, X  ,
		11, 5, 10, 11, 7, 5, 8, 3, 0, X, X, X, X, X, X, X  ,
		5, 11, 7, 5, 10, 11, 1, 9, 0, X, X, X, X, X, X, X  ,
		10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, X, X, X, X  ,
		11, 1, 2, 11, 7, 1, 7, 5, 1, X, X, X, X, X, X, X  ,
		0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, X, X, X, X  ,
		9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, X, X, X, X  ,
		7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, X  ,
		2, 5, 10, 2, 3, 5, 3, 7, 5, X, X, X, X, X, X, X  ,
		8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, X, X, X, X  ,
		9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, X, X, X, X  ,
		9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, X  ,
		1, 3, 5, 3, 7, 5, X, X, X, X, X, X, X, X, X, X  ,
		0, 8, 7, 0, 7, 1, 1, 7, 5, X, X, X, X, X, X, X  ,
		9, 0, 3, 9, 3, 5, 5, 3, 7, X, X, X, X, X, X, X  ,
		9, 8, 7, 5, 9, 7, X, X, X, X, X, X, X, X, X, X  ,
		5, 8, 4, 5, 10, 8, 10, 11, 8, X, X, X, X, X, X, X  ,
		5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, X, X, X, X  ,
		0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, X, X, X, X  ,
		10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, X  ,
		2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, X, X, X, X  ,
		0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, X  ,
		0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, X  ,
		9, 4, 5, 2, 11, 3, X, X, X, X, X, X, X, X, X, X  ,
		2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, X, X, X, X  ,
		5, 10, 2, 5, 2, 4, 4, 2, 0, X, X, X, X, X, X, X  ,
		3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, X  ,
		5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, X, X, X, X  ,
		8, 4, 5, 8, 5, 3, 3, 5, 1, X, X, X, X, X, X, X  ,
		0, 4, 5, 1, 0, 5, X, X, X, X, X, X, X, X, X, X  ,
		8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, X, X, X, X  ,
		9, 4, 5, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		4, 11, 7, 4, 9, 11, 9, 10, 11, X, X, X, X, X, X, X  ,
		0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, X, X, X, X  ,
		1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, X, X, X, X  ,
		3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, X  ,
		4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, X, X, X, X  ,
		9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, X  ,
		11, 7, 4, 11, 4, 2, 2, 4, 0, X, X, X, X, X, X, X  ,
		11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, X, X, X, X  ,
		2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, X, X, X, X  ,
		9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, X  ,
		3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, X  ,
		1, 10, 2, 8, 7, 4, X, X, X, X, X, X, X, X, X, X  ,
		4, 9, 1, 4, 1, 7, 7, 1, 3, X, X, X, X, X, X, X  ,
		4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, X, X, X, X  ,
		4, 0, 3, 7, 4, 3, X, X, X, X, X, X, X, X, X, X  ,
		4, 8, 7, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		9, 10, 8, 10, 11, 8, X, X, X, X, X, X, X, X, X, X  ,
		3, 0, 9, 3, 9, 11, 11, 9, 10, X, X, X, X, X, X, X  ,
		0, 1, 10, 0, 10, 8, 8, 10, 11, X, X, X, X, X, X, X  ,
		3, 1, 10, 11, 3, 10, X, X, X, X, X, X, X, X, X, X  ,
		1, 2, 11, 1, 11, 9, 9, 11, 8, X, X, X, X, X, X, X  ,
		3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, X, X, X, X  ,
		0, 2, 11, 8, 0, 11, X, X, X, X, X, X, X, X, X, X  ,
		3, 2, 11, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		2, 3, 8, 2, 8, 10, 10, 8, 9, X, X, X, X, X, X, X  ,
		9, 10, 2, 0, 9, 2, X, X, X, X, X, X, X, X, X, X  ,
		2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, X, X, X, X  ,
		1, 10, 2, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		1, 3, 8, 9, 1, 8, X, X, X, X, X, X, X, X, X, X  ,
		0, 9, 1, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		0, 3, 8, X, X, X, X, X, X, X, X, X, X, X, X, X  ,
		X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X
	};
#undef X

	std::vector<float> transposedTriTable;
	
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			transposedTriTable.push_back(triTex[j * 16 + i]);
		}
	}
	numVertsTable.resize(256);
	numVertsTable = { 0,
		3,
		3,
		6,
		3,
		6,
		6,
		9,
		3,
		6,
		6,
		9,
		6,
		9,
		9,
		6,
		3,
		6,
		6,
		9,
		6,
		9,
		9,
		12,
		6,
		9,
		9,
		12,
		9,
		12,
		12,
		9,
		3,
		6,
		6,
		9,
		6,
		9,
		9,
		12,
		6,
		9,
		9,
		12,
		9,
		12,
		12,
		9,
		6,
		9,
		9,
		6,
		9,
		12,
		12,
		9,
		9,
		12,
		12,
		9,
		12,
		15,
		15,
		6,
		3,
		6,
		6,
		9,
		6,
		9,
		9,
		12,
		6,
		9,
		9,
		12,
		9,
		12,
		12,
		9,
		6,
		9,
		9,
		12,
		9,
		12,
		12,
		15,
		9,
		12,
		12,
		15,
		12,
		15,
		15,
		12,
		6,
		9,
		9,
		12,
		9,
		12,
		6,
		9,
		9,
		12,
		12,
		15,
		12,
		15,
		9,
		6,
		9,
		12,
		12,
		9,
		12,
		15,
		9,
		6,
		12,
		15,
		15,
		12,
		15,
		6,
		12,
		3,
		3,
		6,
		6,
		9,
		6,
		9,
		9,
		12,
		6,
		9,
		9,
		12,
		9,
		12,
		12,
		9,
		6,
		9,
		9,
		12,
		9,
		12,
		12,
		15,
		9,
		6,
		12,
		9,
		12,
		9,
		15,
		6,
		6,
		9,
		9,
		12,
		9,
		12,
		12,
		15,
		9,
		12,
		12,
		15,
		12,
		15,
		15,
		12,
		9,
		12,
		12,
		9,
		12,
		15,
		15,
		12,
		12,
		9,
		15,
		6,
		15,
		12,
		6,
		3,
		6,
		9,
		9,
		12,
		9,
		12,
		12,
		15,
		9,
		12,
		12,
		15,
		6,
		9,
		9,
		6,
		9,
		12,
		12,
		15,
		12,
		15,
		15,
		6,
		12,
		9,
		15,
		12,
		9,
		6,
		12,
		3,
		9,
		12,
		12,
		15,
		12,
		15,
		9,
		12,
		12,
		15,
		15,
		6,
		9,
		12,
		6,
		3,
		6,
		9,
		9,
		6,
		9,
		12,
		6,
		3,
		9,
		6,
		12,
		3,
		6,
		3,
		3,
		0, };

		GLenum err = glGetError();
		uint32_t temp = 1;

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_1D, m_textureEdgeTable);

		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RED, GL_FLOAT, edgeTable.data());
		err = glGetError();




		//glBindTexture(GL_TEXTURE_1D, 0);
		//glActiveTexture(0);

		glBindTexture(GL_TEXTURE_1D, m_textureTriTex);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256 * 16, GL_RED, GL_FLOAT, triTex.data());

		glBindTexture(GL_TEXTURE_1D, m_textureNumVertsTable);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RED, GL_FLOAT, numVertsTable.data());

		std::vector<float> tableOut;
		tableOut.resize(256, 1);

		glGetTexImage(GL_TEXTURE_1D, 0, GL_RED, GL_FLOAT, tableOut.data());
		err = glGetError();

}

void gFusion::testPrefixSum()
{
	//std::vector<float> testInput, testOutput, testMidput;
	//testInput.resize(1024 * 8 * 2, 1);
	//testOutput.resize(1024 * 8 * 2);
	//testMidput.resize(1024);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_buffer_testInput);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_buffer_testOutput);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * 8 * 2 * sizeof(float), &testOutput[0], GL_STATIC_DRAW);

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_buffer_testMidput);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * sizeof(float), &testMidput[0], GL_STATIC_DRAW);

	//GLuint query[1];
	//glGenQueries(1, query);

	prefixSumProg.use();
	//glBeginQuery(GL_TIME_ELAPSED, query[0]);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resetSumsArrayID);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEachGroupID);
	glDispatchCompute(8, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//glEndQuery(GL_TIME_ELAPSED);


	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEveryGroupID);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forFinalIncrementalSumID);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	std::vector<uint32_t> outputPrefixSum, midPrefixSum;
	outputPrefixSum.resize(1024 * 8 * 2);
	midPrefixSum.resize(1024);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_testOutput);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputPrefixSum.data(), outputPrefixSum.size() * sizeof(uint32_t), ptr, outputPrefixSum.size() * sizeof(uint32_t));

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);



	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_testMidput);
	void *ptr1 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(midPrefixSum.data(), midPrefixSum.size() * sizeof(uint32_t), ptr1, midPrefixSum.size() * sizeof(uint32_t));

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	std::cout << "midprefix " << std::endl;
	for (auto i : midPrefixSum)
		std::cout << i << " ";
	std::cout << std::endl;

	int j = 0, k = -1;
	std::cout << "outprefix " << std::endl;

	for (auto i : outputPrefixSum)
	{
		std::cout << i << " ";
	}
	std::cout << std::endl;
	
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[0], GL_QUERY_RESULT_AVAILABLE, &available);
	//}

	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[0], GL_QUERY_RESULT, &elapsed);

	//double seconds = elapsed / 1000000000.0;

	//std::cout << "time elapsed " << seconds << " sec" << std::endl;

}

void gFusion::depthToVertex(float * depthArray)
{
	int compWidth;
	int compHeight; 

	// gltexsubimage2d  
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	if (depthArray != NULL)
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_depth_width, m_depth_height, GL_RED, GL_FLOAT, depthArray);
	}
	glGenerateMipmap(GL_TEXTURE_2D);

	depthToVertProg.use();

	for (int i = 0; i < 3; i++) // here 3 is the number of mipmap levels
	{
		compWidth = divup(m_depth_width >> i, 32);
		compHeight = divup(m_depth_height >> i, 32);

		glm::mat4 invK = getInverseCameraMatrix(m_camPamsDepth / float(1 << i));
		glBindImageTexture(0, m_textureDepth, i, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
		glBindImageTexture(1, m_textureVertex, i, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glUniformMatrix4fv(m_invkID, 1, GL_FALSE, glm::value_ptr(invK));
		glUniform4fv(m_camPamsID, 1, glm::value_ptr(m_camPamsDepth));

		glDispatchCompute(compWidth, compHeight, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
}

void gFusion::vertexToNormal()
{
	int compWidth;
	int compHeight; 

	vertToNormProg.use();

	for (int i = 0; i < 3; i++) // here 3 is the number of mipmap levels
	{
		glBindImageTexture(0, m_textureVertex, i, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(1, m_textureNormal, i, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		compWidth = divup(m_depth_width >> i, 32);
		compHeight = divup(m_depth_height >> i, 32);

		glDispatchCompute(compWidth, compHeight, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

	}
}

void gFusion::showNormals()
{
	cv::Mat lvl0 = cv::Mat(424, 512, CV_32FC4);
	cv::Mat lvl1 = cv::Mat(424 / 2, 512 / 2, CV_32FC4);
	cv::Mat lvl2 = cv::Mat(424 / 4, 512 / 4, CV_32FC4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureNormal);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, lvl0.data);
	glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_FLOAT, lvl1.data);
	glGetTexImage(GL_TEXTURE_2D, 2, GL_RGBA, GL_FLOAT, lvl2.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(0);

	cv::imshow("lvl0!", lvl0);
	cv::imshow("lvl1!", lvl1);
	cv::imshow("lvl2!", lvl2);



	lvl0 = cv::Mat(424, 512, CV_32FC4);
	lvl1 = cv::Mat(424 / 2, 512 / 2, CV_32FC4);
	lvl2 = cv::Mat(424 / 4, 512 / 4, CV_32FC4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureVertex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, lvl0.data);
	glGetTexImage(GL_TEXTURE_2D, 1, GL_RGBA, GL_FLOAT, lvl1.data);
	glGetTexImage(GL_TEXTURE_2D, 2, GL_RGBA, GL_FLOAT, lvl2.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(0);

	cv::Mat image0[4];
	cv::split(lvl0, image0);
	cv::Mat image1[4];
	cv::split(lvl1, image1);
	cv::Mat image2[4];
	cv::split(lvl2, image2);

	cv::imshow("lvl0", image0[2]);
	cv::imshow("lvl1", image1[2]);
	cv::imshow("lvl2", image2[2]);

}

void gFusion::showRaycast()
{
	cv::Mat points = cv::Mat(424, 512, CV_32FC4);
	cv::Mat norms = cv::Mat(424, 512, CV_32FC4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureReferenceVertex);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, points.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(0);

	cv::Mat pointsDepth[4];
	cv::split(points, pointsDepth);
	cv::imshow("points", pointsDepth[2]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureReferenceNormal);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, norms.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(0);

	cv::imshow("normals", norms);


}
//
//void gFusion::showDifference()
//{
//	cv::Mat dif = cv::Mat(424, 512, CV_32FC1);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, m_textureDifferenceVertex);
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, dif.data);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glActiveTexture(0);
//
//	cv::imshow("difs", dif* 10.0f);
//}
//
//void gFusion::showTrack()
//{
//	cv::Mat trk = cv::Mat(424, 512, CV_32FC4);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, m_textureTrackImage);
//	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, trk.data);
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glActiveTexture(0);
//
//	cv::cvtColor(trk, trk, CV_RGBA2BGRA);
//
//	cv::imshow("trk", trk);
//}

bool gFusion::Track()
{
	//std::cout << "inside track" << std::endl;
	//glBeginQuery(GL_TIME_ELAPSED, query[0]);

	bool tracked = false;
	glm::mat4 oldPose = m_pose;

	float alignmentEnergy;

	//std::cout << "inside time que" << std::endl;

	// here we will loop through the layers and number of iterations per layer
	for (int level = configuration.iterations.size() - 1; level >= 0; --level)
	{
		//std::cout << "inside iter loop" << std::endl;

		//int level = 0;
		for (int iteration = 0; iteration < configuration.iterations[level]; iteration++)
		{
			//std::cout << "inside for loop" << std::endl;

			std::vector<float> b;
			std::vector<float> C;

			track(level);
			//std::cout << "done track" << std::endl;

			reduce(level);
			//std::cout << "done reduce" << std::endl;

			getReduction(b, C, alignmentEnergy);
			//std::cout << "done reduction" << std::endl;


			//std::cout << "level " << level << " iteration " << iteration << " AE:" << alignmentEnergy << std::endl;

			Eigen::Matrix<float, 6, 1> b_icp(b.data());
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> C_icp(C.data());

			Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dC_icp = C_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();


			//result = dC_icp.ldlt().solve(db_icp);
			//result = dC_icp.fullPivLu().solve(db_icp);

			Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?


			glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			delta[3][0] = result(0);
			delta[3][1] = result(1);
			delta[3][2] = result(2);

			m_pose = delta * m_pose;

			if (result.norm() < 1e-5 && result.norm() != 0)
				break;
		}
	}


	//std::cout << "out of loop" << std::endl;

	if (alignmentEnergy > 0.5f || alignmentEnergy == 0)
	{
		m_pose = oldPose;
	}
	else
	{
		tracked = true;
		updatePoseFinder();
	}
	//tracked = true;

	//m_pose = oldPose;

	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[0], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[0], GL_QUERY_RESULT, &elapsed);
	//trackTime = elapsed / 1000000.0;
	//std::cout << "after times" << std::endl;

	return tracked;
}

void gFusion::track(int layer)
{

	glm::mat4 invK = getInverseCameraMatrix(m_camPamsDepth);
	glm::mat4 oldPose = pose;
	glm::mat4 projectReference = getCameraMatrix(m_camPamsDepth) * glm::inverse(m_pose);

	int compWidth;
	int compHeight;

	trackProg.use();

	glUniformMatrix4fv(m_viewID_t, 1, GL_FALSE, glm::value_ptr(projectReference));
	glUniformMatrix4fv(m_TtrackID, 1, GL_FALSE, glm::value_ptr(m_pose));
	glUniform1f(m_distThresh_t, configuration.dist_threshold);
	glUniform1f(m_normThresh_t, configuration.normal_threshold);

	glBindImageTexture(0, m_textureVertex, layer, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, m_textureNormal, layer, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindImageTexture(2, m_textureReferenceVertex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // the raycasted images do not need to be scaled, since the mipmaped verts are projected back to 512*424 image space resolution in the shader
	glBindImageTexture(3, m_textureReferenceNormal, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	
	glBindImageTexture(4, m_textureDifferenceVertex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	glBindImageTexture(5, m_textureTrackImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	
	compWidth = divup(m_depth_width >> layer, 32); // right bitshift does division by powers of 2
	compHeight = divup(m_depth_height >> layer, 32);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);



}

void gFusion::reduce(int layer)
{


	//glBeginQuery(GL_TIME_ELAPSED, query[1]);

	glm::ivec2 imageSize = glm::ivec2(m_depth_width >> layer, m_depth_height >> layer);

	//std::cout << "image size " << m_imageSizeID << " " << imageSize.x << " " << imageSize.y << std::endl;
	reduceProg.use();

	glUniform2iv(m_imageSizeID, 1, glm::value_ptr(imageSize));
	//glBindImageTexture(0, m_textureOutputData, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	glDispatchCompute(8, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//std::cout << "done compute reduce " << std::endl;
	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[1], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[1], GL_QUERY_RESULT, &elapsed);
	//reduceTime = elapsed / 1000000.0;

}

Eigen::Matrix4f Twist(const Eigen::Matrix<double, 6, 1> &xi)
{
	//glm::mat4 M;
	//M[0][0] = 0.0f;		M[1][0] = -xi(2);	M[2][0] = xi(1);	M[3][0] = xi(3);
	//M[0][1] = xi(2);	M[1][1] = 0.0f;		M[2][1] = -xi(1);	M[3][1] = xi(4);
	//M[0][2] = -xi(1);	M[1][2] = xi(0);	M[2][2] = 0.0f;		M[3][2] = xi(5);
	//M[0][3] = 0.0f;		M[1][3] = 0.0f;		M[2][3] = 0.0f;		M[3][3] = 0.0f;

	Eigen::Matrix4f M;
	//M << 0.0, -xi(2), xi(1), xi(3),
	//	xi(2), 0.0, -xi(0), xi(4),
	//	-xi(1), xi(0), 0.0, xi(5),
	//	0.0, 0.0, 0.0, 0.0;

	M << 0.0, -xi(2), xi(1), xi(3),
		xi(2), 0.0, -xi(0), xi(4),
		-xi(1), xi(0), 0.0, xi(5),
		0.0, 0.0, 0.0, 0.0;

	//Eigen::Matrix4f Mt = M.transpose();

	return M;
};

bool gFusion::TrackSDF() {



	bool tracked = false;
	glm::mat4 oldPose = m_pose;

	glm::mat4 sdfPose = m_pose;

	Eigen::Matrix4f m_pose_eig_prev = m_pose_eig;

	Eigen::Vector3d trans;
	Eigen::Matrix3d rot;

	float alignmentEnergy;

	Eigen::Matrix<double, 6, 1> result;
	result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
	//glm::mat4 twistMat = Twist(result);
	Eigen::Matrix<double, 6, 1> result_prev = result;
	//Eigen::Matrix4d twistMat = Twist(result);


	// here we will loop through the layers and number of iterations per layer
	for (int level = configuration.iterations.size() - 1; level >= 0; --level)
	{
	//	if (tracked == true)
	//		break;
	//	int level = 0;
		for (int iteration = 0; iteration < configuration.iterations[level]; iteration++)
		{
			Eigen::Matrix4f camToWorld = Twist(result).exp() * m_pose_eig_prev;
			//std::cout << "level " << level << " iter " << iteration << " result " << result.transpose() << std::endl;
			std::vector<float> b;
			std::vector<float> C;

			trackSDF(level, camToWorld);
			reduceSDF(level);

			Eigen::Matrix<double, 6, 6> A0 = Eigen::Matrix<double, 6, 6>::Zero();
			Eigen::Matrix<double, 6, 1> b0 = Eigen::Matrix<double, 6, 1>::Zero();

			//getPreRedu(A0, b0);

			getSDFReduction(b, C, alignmentEnergy);

			//std::cout << alignmentEnergy << std::endl;

			Eigen::Matrix<float, 6, 1> b_icp(b.data());
			Eigen::Matrix<float, 6, 6, Eigen::RowMajor> C_icp(C.data());

			//Eigen::Matrix<double, 6, 1> result;
			Eigen::Matrix<double, 6, 6, Eigen::RowMajor> dC_icp = C_icp.cast<double>();
			Eigen::Matrix<double, 6, 1> db_icp = b_icp.cast<double>();

			double scaling = 1 / dC_icp.maxCoeff();

			dC_icp *= scaling;
			db_icp *= scaling;

			dC_icp = dC_icp + (double(iteration) * 0.01)*Eigen::MatrixXd::Identity(6, 6);

			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?
			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = result - svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?

			result = result - dC_icp.ldlt().solve(db_icp);

			//std::cout  std::endl;

			Eigen::Matrix<double, 6, 1> Change = result - result_prev;
			double Cnorm = Change.norm();

			result_prev = result;
			//std::cout << "AE: " << alignmentEnergy << " snorm : " << Cnorm << " vec " << result.transpose() << std::endl;

			//std::cout << "cnrom :" << Cnorm << std::endl;
			if (alignmentEnergy != 0 && alignmentEnergy < 1e-3 || Cnorm < 1e-3)
			{
				//std::cout << "tracked!!! iteration " << iteration << " level " << level << " AE: " << alignmentEnergy << " snorm : " << Cnorm << " vec " << result.transpose() << std::endl;

				//std::cout << "tracked!!! iteration " << iteration << std::endl;
				tracked = true;
				break;
			}


			//m_pose_eig = Twist(result).exp() * m_pose_eig;




			//result = dC_icp.inverse() * db_icp;

			//Eigen::Affine3d aff = eigen_utils::direct_exponential_map(result, 1.0);




			//rot <<	m_pose[0][0], m_pose[1][0], m_pose[2][0],
			//		m_pose[0][1], m_pose[1][1], m_pose[2][1],
			//		m_pose[0][2], m_pose[1][2], m_pose[2][2];

			//trans << m_pose[3][0], m_pose[3][1], m_pose[3][2];

			//Eigen::Matrix3d postRot = aff.rotation().transpose() * rot;
			//Eigen::Vector3d postTrans = trans - aff.rotation().transpose() * aff.translation();

		//	std::cout << postRot << std::endl;
		//	std::cout << postTrans << std::endl;
			//Eigen::JacobiSVD<Eigen::MatrixXd> svd(dC_icp, Eigen::ComputeFullU | Eigen::ComputeFullV);
			//result = svd.solve(db_icp); // TODO CHECK THIS WORKS, SHOULD WE MAKE A BACK SUB SOLVER?
			//result = dC_icp.ldlt().solve(db_icp);

			//glm::mat4 delta = glm::eulerAngleXYZ(result(3), result(4), result(5));
			//delta[3][0] = result(0);
			//delta[3][1] = result(1);
			//delta[3][2] = result(2);

			//sdfPose = delta * sdfPose;

			/*m_pose[0][0] = postRot(0, 0); m_pose[1][0] = postRot(0, 1); m_pose[2][0] = postRot(0, 2); m_pose[3][0] = postTrans(0);
			m_pose[0][1] = postRot(1, 0); m_pose[1][1] = postRot(1, 1); m_pose[2][1] = postRot(1, 2); m_pose[3][1] = postTrans(1);
			m_pose[0][2] = postRot(2, 0); m_pose[1][2] = postRot(2, 1); m_pose[2][2] = postRot(2, 2); m_pose[3][2] = postTrans(2);
			m_pose[0][3] = 0;		  m_pose[1][3] = 0;			m_pose[2][3] = 0;	      m_pose[3][3] = 1.0f;

			if (result(0, 0) < 0.001
				&& result(1, 0) < 0.001
				&& result(2, 0) < 0.001
				&& result(3, 0) < 0.001
				&& result(4, 0) < 0.001
				&& result(5, 0) < 0.001) {
				std::cout << "STOP Gauss Newton at level: " << level << " it: " << iteration << std::endl;
				tracked = true;
				break;

			}*/

			//m_pose = sdfPose;

		//	std::cout << result.norm() << std::endl;
			//if (result.norm() < 1e-2 && result.norm() != 0)
			//{
			//	//std::cout << "converged" << std::endl;
			//	tracked = true;
			//	break;

			//}



			}

		}

		if (std::isnan(result.sum())) result << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;


		if (tracked)
		{
			m_pose_eig = Twist(result).exp() * m_pose_eig_prev;

			m_pose[0][0] = m_pose_eig(0, 0); m_pose[1][0] = m_pose_eig(0, 1); m_pose[2][0] = m_pose_eig(0, 2); m_pose[3][0] = m_pose_eig(0, 3);
			m_pose[0][1] = m_pose_eig(1, 0); m_pose[1][1] = m_pose_eig(1, 1); m_pose[2][1] = m_pose_eig(1, 2); m_pose[3][1] = m_pose_eig(1, 3);
			m_pose[0][2] = m_pose_eig(2, 0); m_pose[1][2] = m_pose_eig(2, 1); m_pose[2][2] = m_pose_eig(2, 2); m_pose[3][2] = m_pose_eig(2, 3);
			m_pose[0][3] = 0;				 m_pose[1][3] = 0;				  m_pose[2][3] = 0;				   m_pose[3][3] = 1.0f;

			//updatePoseFinder();

			m_cumTwist += result;
		}
		else
		{

			m_pose_eig = m_pose_eig_prev;
			m_pose = oldPose;
		}


	//std::cout << " m_pose " << std::endl;
	//std::cout << m_pose[0][0] << " " << m_pose[1][0] << " " << m_pose[2][0] << " " << m_pose[3][0] << std::endl;
	//std::cout << m_pose[0][1] << " " << m_pose[1][1] << " " << m_pose[2][1] << " " << m_pose[3][1] << std::endl;
	//std::cout << m_pose[0][2] << " " << m_pose[1][2] << " " << m_pose[2][2] << " " << m_pose[3][2] << std::endl;
	//std::cout << m_pose[0][3] << " " << m_pose[1][3] << " " << m_pose[2][3] << " " << m_pose[3][3] << std::endl;


	

	//if (alignmentEnergy > 0.5f || alignmentEnergy == 0)
	//{
	//	m_pose = oldPose;
	//}
	//else
	//{
	//	tracked = true;
	//	updatePoseFinder();
	//}




	return tracked;
}

void gFusion::trackSDF(int layer, Eigen::Matrix4f camToWorld)
{
	//glBeginQuery(GL_TIME_ELAPSED, query[5]);


	trackSDFProg.use();

	int xthreads, ythreads;
	xthreads = divup(m_depth_width >> layer, 32); // right bitshift does division by powers of 2
	ythreads = divup(m_depth_height >> layer, 32);
	glm::ivec2 imageSize = glm::ivec2(m_depth_width >> layer, m_depth_height >> layer);

	// bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16I);
	glBindImageTexture(1, m_textureVertex, layer, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, m_textureTest, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// set uniforms
	//glUniformMatrix4fv(m_TtrackID_t, 1, GL_FALSE, glm::value_ptr(m_pose));
	glUniformMatrix4fv(m_TtrackID_t, 1, GL_FALSE, camToWorld.data());
	glUniform2iv(m_imageSizeID_t_sdf, 1, glm::value_ptr(imageSize));

	glUniform1f(m_cID, 0.02f * 0.1f);
	glUniform1f(m_epsID, 10e-9);
	glUniform3fv(m_volDimID_t, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_t, 1, glm::value_ptr(configuration.volumeSize));

	glDispatchCompute(xthreads, ythreads, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);

	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[5], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[5], GL_QUERY_RESULT, &elapsed);
	//trackSDFTime = elapsed / 1000000.0;



	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureTest);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testIm.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(0);

	//cv::imshow("testim1", testIm);





}

void gFusion::reduceSDF(int layer)
{
	//glBeginQuery(GL_TIME_ELAPSED, query[6]);

	glm::ivec2 imageSize = glm::ivec2(m_depth_width >> layer, m_depth_height >> layer);

	reduceSDFProg.use();

	glUniform2iv(m_imageSizeID_sdf, 1, glm::value_ptr(imageSize));
	//glBindImageTexture(0, m_textureOutputData, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	glDispatchCompute(8, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);

	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[6], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[6], GL_QUERY_RESULT, &elapsed);
	//reduceSDFTime = elapsed / 1000000.0;

}

void gFusion::getReduction(std::vector<float>& b, std::vector<float>& C, float &alignmentEnergy)
{
	outputData.resize(32 * 8);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_outputdata);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//for (auto i : outputData)
	//	std::cout << i << " ";

	//std::cout << std::endl;
	// output data has 32 cols and 8 rows
	// sum the 8 rows up to the top row

	for (int row = 1; row < 8; row++)
	{
		for (int col = 0; col < 32; col++)
		{
			outputData[col + 0 * 32] = outputData[col + row * 32];
		}
	}


	std::vector<float>::const_iterator first0 = outputData.begin() + 1;
	std::vector<float>::const_iterator last0 = outputData.begin() + 28;

	std::vector<float> vals(first0, last0);

	std::vector<float>::const_iterator first1 = vals.begin();
	std::vector<float>::const_iterator last1 = vals.begin() + 6;
	std::vector<float>::const_iterator last2 = vals.begin() + 6 + 21;

	std::vector<float> bee(first1, last1);
	std::vector<float> Cee = makeJTJ(std::vector<float>(last1, last2));

	b = bee;
	C = Cee;

	alignmentEnergy = sqrt(outputData[0] / outputData[28]);
}

void gFusion::getPreRedu(Eigen::Matrix<double, 6, 6> &A, Eigen::Matrix<double, 6, 1> &b)
{
	std::vector<float> outputSDFData;
	size_t reductionSDFSize = m_depth_height * m_depth_width * 8; // this is the size of one reduction element 1 float + 1 float + 6 float
	outputSDFData.resize(reductionSDFSize);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferSDFReduction);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputSDFData.data(), reductionSDFSize * sizeof(GLfloat), ptr, reductionSDFSize * sizeof(GLfloat));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//Eigen::Matrix<double, 6, 6> A = Eigen::Matrix<double, 6, 6>::Zero();
	//Eigen::Matrix<double, 6, 1> b = Eigen::Matrix<double, 6, 1>::Zero();
	Eigen::Matrix<double, 6, 1> SDF_derivative = Eigen::Matrix<double, 6, 1>::Zero();

	for (int i = 0; i < 512 * 424 * 8; i += 8)
	{
		SDF_derivative << outputSDFData[i + 2], outputSDFData[i + 3], outputSDFData[i + 4], outputSDFData[i + 5], outputSDFData[i + 6], outputSDFData[i + 7];
		A = A + SDF_derivative * SDF_derivative.transpose();
		b = b + (outputSDFData[i + 1] * SDF_derivative);
	}



}

void gFusion::getSDFReduction(std::vector<float>& b, std::vector<float>& C, float &alignmentEnergy)
{
	std::vector<float> outputSDFData;
	outputSDFData.resize(32 * 8);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferSDFoutputdata);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputSDFData.data(), outputSDFData.size() * sizeof(float), ptr, outputSDFData.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


	for (int row = 1; row < 8; row++)
	{
		for (int col = 0; col < 32; col++)
		{
			outputSDFData[col + 0 * 32] = outputSDFData[col + row * 32];
		}
	}

	std::vector<float>::const_iterator first0 = outputSDFData.begin() + 1;
	std::vector<float>::const_iterator last0 = outputSDFData.begin() + 28;

	std::vector<float> vals(first0, last0);

	std::vector<float>::const_iterator first1 = vals.begin();
	std::vector<float>::const_iterator last1 = vals.begin() + 6;
	std::vector<float>::const_iterator last2 = vals.begin() + 6 + 21;

	std::vector<float> bee(first1, last1);
	std::vector<float> Cee = makeJTJ(std::vector<float>(last1, last2));

	b = bee;
	C = Cee;

	alignmentEnergy = sqrt(outputSDFData[0] / outputSDFData[28]);


}

void gFusion::integrate()
{


	//glBeginQuery(GL_TIME_ELAPSED, query[2]);

	integrateProg.use();

	glm::mat4 integratePose = glm::inverse(m_pose);
	glm::mat4 K = getCameraMatrix(m_camPamsDepth); // make sure im set

	// bind uniforms
	glUniformMatrix4fv(m_invTrackID, 1, GL_FALSE, glm::value_ptr(integratePose));
	glUniformMatrix4fv(m_KID, 1, GL_FALSE, glm::value_ptr(K));
	glUniform1f(m_muID, configuration.mu);
	glUniform1f(m_maxWeightID, configuration.maxWeight);
	glUniform3fv(m_volDimID, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID, 1, glm::value_ptr(configuration.volumeSize));

	//bind image textures
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16I);
	glBindImageTexture(1, m_textureDepth, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);


	int xWidth;
	int yWidth;


	xWidth = divup(configuration.volumeSize.x, 32);
	yWidth = divup(configuration.volumeSize.y, 32);



	glDispatchCompute(xWidth, yWidth, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);


	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[2], GL_QUERY_RESULT_AVAILABLE, &available);
	//}

	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[2], GL_QUERY_RESULT, &elapsed);

	//integrateTime = elapsed / 1000000.0;

	


}

void gFusion::raycast()
{


	//glBeginQuery(GL_TIME_ELAPSED, query[3]);

	raycastProg.use();

	glm::mat4 invK = getInverseCameraMatrix(m_camPamsDepth);
	glm::mat4 view = m_pose * invK;

	float step = configuration.stepSize();

	// bind uniforms
	glUniformMatrix4fv(m_viewID_r, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(m_nearPlaneID, configuration.nearPlane);
	glUniform1f(m_farPlaneID, configuration.farPlane);
	glUniform1f(m_stepID, step);
	glUniform1f(m_largeStepID, 0.75f * configuration.mu);
	glUniform3fv(m_volDimID_r, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_r, 1, glm::value_ptr(configuration.volumeSize));

	//bind image textures
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16I);
	glBindImageTexture(1, m_textureReferenceVertex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(2, m_textureReferenceNormal, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	//glBindImageTexture(3, m_textureTestImage, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	

	//glBindImageTexture(1, m_textureVolumeSliceNorm, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(2, m_textureVolumeColor, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	// bind the volume texture for 3D sampling
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, m_textureVolume);

	/*glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, m_textureVolumeColor);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_textureColor);*/

	int xWidth;
	int yWidth;


	xWidth = divup(m_depth_width, 32);
	yWidth = divup(m_depth_height, 32);


	glDispatchCompute(xWidth, yWidth, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);
	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[3], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[3], GL_QUERY_RESULT, &elapsed);
	//raycastTime = elapsed / 1000000.0;

	//std::cout << raycastTime << std::endl;

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_textureTestImage);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, testIm2.data);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glActiveTexture(0);

	//cv::imshow("testim2", testIm2);



}

void gFusion::intensityProjection()
{
	mipProg.use();

	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	glBindImageTexture(1, m_textureMip, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	glm::mat4 invK = getInverseCameraMatrix(m_camPamsDepth);
	glm::mat4 view = m_pose * invK;

	float step = configuration.stepSize();

	// bind uniforms
	glUniformMatrix4fv(m_viewID_m, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(m_nearPlaneID_m, configuration.nearPlane);
	glUniform1f(m_farPlaneID_m, configuration.farPlane);
	glUniform1f(m_stepID_m, step);
	glUniform1f(m_largeStepID_m, 0.75f * configuration.mu);
	glUniform3fv(m_volDimID_m, 1, glm::value_ptr(configuration.volumeDimensions));
	glUniform3fv(m_volSizeID_m, 1, glm::value_ptr(configuration.volumeSize));


	int xWidth;
	int yWidth;


	xWidth = divup(m_depth_width, 32);
	yWidth = divup(m_depth_height, 32);


	glDispatchCompute(xWidth, yWidth, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	/*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureMip);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, mipMat.data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(0);

	cv::imshow("ints", mipMat * 10.0f);*/

}

GLuint gFusion::prefixSum(GLuint inputBuffer, GLuint outputBuffer)
{
	// reduction sum
	prefixSumProg.use();
	int xthreads = divup(mcubeConfiguration.numVoxels, 1024); // 1024 is the localworkgroupsize inside the shader

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, inputBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, outputBuffer);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_resetSumsArrayID);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEachGroupID);
	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	int xthreads2 = divup(xthreads, 1024);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forEveryGroupID);
	glDispatchCompute(xthreads2, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_forFinalIncrementalSumID);
	glDispatchCompute(xthreads, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	uint32_t lastElement, lastScanElement;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (mcubeConfiguration.numVoxels - 1) * sizeof(uint32_t), sizeof(uint32_t), &lastScanElement);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (mcubeConfiguration.numVoxels - 1) * sizeof(uint32_t), sizeof(uint32_t), &lastElement);

	return lastElement + lastScanElement;

}

void gFusion::marchingCubes()
{


	//glBeginQuery(GL_TIME_ELAPSED, query[4]);

	int threads = 128;

	// CLASSIFY VOXEL
	marchingCubesProg.use();
	// BIND TEXTURES
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	glBindImageTexture(1, m_textureEdgeTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(2, m_textureTriTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(3, m_textureNumVertsTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	// BIND BUFFERS
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bufferVoxelVerts);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	// SET UNIFORMS
	glUniform3uiv(m_gridSizeID, 1, glm::value_ptr(mcubeConfiguration.gridSize));
	glUniform3uiv(m_gridSizeShiftID, 1, glm::value_ptr(mcubeConfiguration.gridSizeShift));
	glUniform3uiv(m_gridSizeMaskID, 1, glm::value_ptr(mcubeConfiguration.gridSizeMask));
	glUniform1f(m_isoValueID, mcubeConfiguration.isoValue);
	glUniform1ui(m_numVoxelsID, mcubeConfiguration.numVoxels);

	int xthreads = divup(mcubeConfiguration.numVoxels, threads);
	int ythreads = 1;
	if (xthreads > 65535)
	{
		ythreads = xthreads / 32768;
		xthreads = 32768;
	}
	// LAUNCH SHADER
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_classifyVoxelID);
	glDispatchCompute(xthreads, ythreads, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// PREFIX SUM
	mcubeConfiguration.activeVoxels = prefixSum(m_bufferVoxelOccupied, m_bufferVoxelOccupiedScan);
	//std::cout << "active voxels " << mcubeConfiguration.activeVoxels << std::endl;

	// COMPACT VOXELS
	marchingCubesProg.use();
	// BIND TEXTURES
	// BIND BUFFERS
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_bufferVoxelOccupied);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_bufferVoxelOccupiedScan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	// BIND UNIFORMS
	glUniform1ui(m_numVoxelsID, mcubeConfiguration.numVoxels);

	xthreads = divup(mcubeConfiguration.numVoxels, threads);
	ythreads = 1;
	if (xthreads > 65535)
	{
		ythreads = xthreads / 32768;
		xthreads = 32768;
	}
	// LAUNCH SHADER
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_compactVoxelsID);
	glDispatchCompute(xthreads, ythreads, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// PREFIX SUM
	m_totalVerts = prefixSum(m_bufferVoxelVerts, m_bufferVoxelVertsScan);
	//std::cout << "total verts " << m_totalVerts << std::endl;

	// GENERATE TRIANGLES
	marchingCubesProg.use();
	// BIND TEXTURES
	glBindImageTexture(0, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);
	glBindImageTexture(1, m_textureEdgeTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(2, m_textureTriTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(3, m_textureNumVertsTable, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	// BIND BUFFERS
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bufferCompactedVoxelArray);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_bufferVoxelVertsScan);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferPos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_bufferNorm);
	// BIND UNIFORMS
	glUniform3uiv(m_gridSizeID, 1, glm::value_ptr(mcubeConfiguration.gridSize));
	glUniform3uiv(m_gridSizeShiftID, 1, glm::value_ptr(mcubeConfiguration.gridSizeShift));
	glUniform3uiv(m_gridSizeMaskID, 1, glm::value_ptr(mcubeConfiguration.gridSizeMask));
	glUniform1f(m_isoValueID, mcubeConfiguration.isoValue);
	glUniform1ui(m_numVoxelsID, mcubeConfiguration.numVoxels);
	glUniform1ui(m_activeVoxelsID, mcubeConfiguration.activeVoxels);
	glUniform1ui(m_maxVertsID, mcubeConfiguration.maxVerts);
	glUniform3fv(m_voxelSizeID, 1, glm::value_ptr(mcubeConfiguration.voxelSize));
	
	xthreads = divup(mcubeConfiguration.activeVoxels, threads);
	ythreads = 1;
	while (xthreads > 65535)
	{
		xthreads /= 2;
		ythreads *= 2;
	}
	// LAUNCH SHADER
	glUniformSubroutinesuiv(GL_COMPUTE_SHADER, 1, &m_generateTrianglesID);
	glDispatchCompute(xthreads, ythreads, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//glEndQuery(GL_TIME_ELAPSED);


	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(query[4], GL_QUERY_RESULT_AVAILABLE, &available);
	//}

	//// elapsed time in nanoseconds
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(query[4], GL_QUERY_RESULT, &elapsed);

	//marchingCubesTime = elapsed / 1000000.0;

	//std::cout << "time elapsed " << marchingCubesTime << " ms" << std::endl;


}

void gFusion::updatePoseFinder()
{
	// if tracking is success, see if the glm pose is norm distance away from any in the library, if so, a new posepair should be added to the library
	int currentLibrarySize = poseLibrary.size();

	if (currentLibrarySize == 0)
	{
		addPoseToLibrary();
	}
	else
	{
		for (int i = 0; i < currentLibrarySize; i++)
		{
			glm::vec3 libTran = glm::vec3(poseLibrary[i].pose[3][0], poseLibrary[i].pose[3][1], poseLibrary[i].pose[3][2]);
			glm::vec3 curTran = glm::vec3(m_pose[3][0], m_pose[3][1], m_pose[3][2]);

			float theNorm = glm::l2Norm(libTran, curTran);

			if (theNorm < 0.2f)
			{
				return;
			}
		}
		// if here, then no matching pose was found in library
		addPoseToLibrary();
	}

}

void gFusion::addPoseToLibrary()
{

	gPosePair newPose;
	GLuint new_textureID;
	new_textureID = createTexture(GL_TEXTURE_2D, 0, m_depth_width, m_depth_height, 1, GL_R32F);

	glCopyImageSubData(m_textureDepth, GL_TEXTURE_2D, 0, 0, 0, 0,
		new_textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_depth_width, m_depth_height, 1);

	newPose.textureID = new_textureID;
	newPose.pose = m_pose;
	poseLibrary.push_back(newPose);

}
/// Function called when tracking is lost
bool gFusion::recoverPose()
{
	bool foundPose = false;
	int currentLibrarySize = poseLibrary.size();
	std::cout << currentLibrarySize << std::endl;
	if (currentLibrarySize == 0)
	{
		std::cout << "You should not have arrived here" << std::endl;
		return foundPose;
	}
	else
	{
		for (int i = 0; i < currentLibrarySize; i++)
		{
			m_pose = poseLibrary[i].pose;
			/*glCopyImageSubData(poseLibrary[i].textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
				m_textureDepth, GL_TEXTURE_2D, 0, 0, 0, 0,
				m_depth_width, m_depth_height, 1);*/
			//raycast();
			//depthToVertex(NULL);
			//vertexToNormal();
			//showNormals();
			//foundPose = Track();
			foundPose = TrackSDF();
			if (foundPose)
			{
				std::cout << "found a pose " << std::endl;
				break;
			}

		}



	}

	return foundPose;
}

void gFusion::trackPoints3D(GLuint trackedPoints2Dbuffer)
{

	int compWidth;
	int compHeight;

	helpersProg.use();

	glm::mat4 invK = getInverseCameraMatrix(m_camPamsDepth);
	glUniformMatrix4fv(m_invKID_h, 1, GL_FALSE, glm::value_ptr(invK));
	glUniform1i(m_buffer2DWidthID, m_depth_height * 2);

	// BIND IMAGE
	glBindImageTexture(1, m_textureDepth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	// BIND BUFFERS
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, trackedPoints2Dbuffer);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_trackedPoints3DBuffer);

	compWidth = divup(m_depth_width, 32);
	compHeight = divup(m_depth_height, 32);

	glDispatchCompute(compWidth, compHeight, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
/*

	std::vector<float> outputData(m_depth_height * m_depth_height *2);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, trackedPoints2Dbuffer);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(outputData.data(), outputData.size() * sizeof(float), ptr, outputData.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	cv::Mat blank = cv::Mat(424, 512, CV_8UC4);
	for (int i = 0; i < m_depth_height * m_depth_height * 2; i += 2)
	{

		cv::circle(blank, cv::Point2f(outputData[i], outputData[i + 1]), 2, cv::Scalar(255, 128, 128, 255));

	}    

	cv::imshow("tracawked", blank);*/



}





void gFusion::exportSurfaceAsStlBinary()
{

	std::string modelFileName = "data/meshes/marchingCubesBin.stl";

	std::ofstream outFile(modelFileName, std::ios::out | std::ios::binary);

	if (!outFile)
	{
		//cerr << "Error opening output file: " << FileName << "!" << endl;
		printf("Error opening output file: %s!\n", modelFileName);
		exit(1);
	}

	// copy cuda device to host


	////
	// Header
	////

	char hdr[80];

	const int pointNum = mcubeConfiguration.maxVerts;
	uint32_t NumTri = mcubeConfiguration.maxVerts / 3;
	uint32_t attributeByteCount = 0;

	//outFile.write(hdr, 80);
	//outFile.write((char*)&NumTri, sizeof(uint));

	// h_data is the posVbo, i.e. the array of verts of length maxVerts, sparse
	// h_compVoxelArray is 
	// h_voxelVerts is the number of verts in each voxel, i.e. an array of length volume height * width * depth with ints inside saying how many verts are inside each voxel. should this be 1 if one vox only contains one vert

	std::vector<uint32_t> h_compVoxelArray, h_voxelVertsScan, h_voxelVerts;
	h_compVoxelArray.resize(mcubeConfiguration.numVoxels);
	h_voxelVertsScan.resize(mcubeConfiguration.numVoxels);
	h_voxelVerts.resize(mcubeConfiguration.numVoxels);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferCompactedVoxelArray);
	void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(h_compVoxelArray.data(), h_compVoxelArray.size() * sizeof(uint32_t), ptr, h_compVoxelArray.size() * sizeof(uint32_t));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferVoxelVertsScan);
	void *ptr1 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(h_voxelVertsScan.data(), h_voxelVertsScan.size() * sizeof(uint32_t), ptr1, h_voxelVertsScan.size() * sizeof(uint32_t));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferVoxelVerts);
	void *ptr2 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(h_voxelVerts.data(), h_voxelVerts.size() * sizeof(uint32_t), ptr2, h_voxelVerts.size() * sizeof(uint32_t));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	std::vector<float> hData, hDataNorm;
	hData.resize(mcubeConfiguration.maxVerts * 4);
	hDataNorm.resize(mcubeConfiguration.maxVerts * 4);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferPos);
	void *ptr3 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(hData.data(), hData.size() * sizeof(float), ptr3, hData.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferNorm);
	void *ptr4 = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy_s(hDataNorm.data(), hDataNorm.size() * sizeof(float), ptr4, hDataNorm.size() * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	outFile << "solid phils stler" << std::endl;

	//std::cout << " verts ";
	//for (int i = 0; i < m_totalVerts * 4; i++)
	//	std::cout << hData[i] << " ";

	for (int i = 0; i < m_totalVerts; i++) 
	{
		uint32_t voxel = h_compVoxelArray[i];
		uint32_t index = h_voxelVertsScan[voxel]; // index is the start of the array from where your triangles are for current voxel, you need to go from this point to index + numvertinbox * 4
		uint32_t numVertInVox = h_voxelVerts[voxel];

		// each vertex is separated in hData by 4
		//	vertex1 = hData[0] hData[1] hData[2]
		//	vertex2 = hData[4] hData[5] hData[6]
		//	vertex3 = hData[8] hData[9] hData[10]
		// the next triangle starts at hData[12]
			





		for (int j = 0; j < numVertInVox * 4; j += 12) // j is an incrementer of vertexes, previously we used a float 4
		{
			outFile << "facet normal " << hDataNorm[index * 4 + j + 0] * -1.0f << " " << hDataNorm[index * 4 + j + 1] * -1.0f << " " << hDataNorm[index * 4 + j + 2] * -1.0f << std::endl;
			outFile << "outer loop" << std::endl;
			outFile << "vertex " << hData[index * 4 + j + 0] << " " << hData[index * 4 + j + 1] << " " << hData[index * 4 + j + 2] << std::endl;
			outFile << "vertex " << hData[index * 4 + j + 4] << " " << hData[index * 4 + j + 5] << " " << hData[index * 4 + j + 6] << std::endl;
			outFile << "vertex " << hData[index * 4 + j + 8] << " " << hData[index * 4 + j + 9] << " " << hData[index * 4 + j + 10] << std::endl;
			outFile << "endloop" << std::endl;
			outFile << "endfacet" << std::endl;

		}

		//for (int j = 0; j < numVertInVox; j += 3) // oh yeah
		//{


		//	outFile.write((char*)&hDataNorm[index + (j * 3) + 0], sizeof(float));
		//	outFile.write((char*)&hDataNorm[index + (j * 3) + 1], sizeof(float));
		//	outFile.write((char*)&hDataNorm[index + (j * 3) + 2], sizeof(float));

		//	outFile.write((char*)&hData[index + (j * 3) + 0], sizeof(float));
		//	outFile.write((char*)&hData[index + (j * 3) + 0], sizeof(float));
		//	outFile.write((char*)&hData[index + (j * 3) + 0], sizeof(float));

		//	outFile.write((char*)&hData[index + (j * 3) + 1], sizeof(float));
		//	outFile.write((char*)&hData[index + (j * 3) + 1], sizeof(float));
		//	outFile.write((char*)&hData[index + (j * 3) + 1], sizeof(float));

		//	outFile.write((char*)&hData[index + (j * 3) + 2], sizeof(float));
		//	outFile.write((char*)&hData[index + (j * 3) + 2], sizeof(float));
		//	outFile.write((char*)&hData[index + (j * 3) + 2], sizeof(float));

		//	outFile.write((char*)&attributeByteCount, sizeof(uint));

		//}

	}




	outFile.close();



}

void gFusion::testLargeUpload()
{
	//GLuint queryUpload[1];
	//glGenQueries(1, queryUpload);


	//glBeginQuery(GL_TIME_ELAPSED, queryUpload[0]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lmbuff_0);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 402653184 * sizeof(int), &listmode[0]);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_lmbuff_1);
	//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 402653184 * sizeof(int) / 2, &listmode[402653184 / 2]);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);


	//glEndQuery(GL_TIME_ELAPSED);

	//GLuint available = 0;
	//while (!available) {
	//	glGetQueryObjectuiv(queryUpload[0], GL_QUERY_RESULT_AVAILABLE, &available);
	//}
	//// elapsed time in milliseconds?
	//GLuint64 elapsed;
	//glGetQueryObjectui64vEXT(queryUpload[0], GL_QUERY_RESULT, &elapsed);
	//double uploadTime = elapsed / 1000000.0;
	//std::cout << "uptime " << uploadTime << std::endl;

}