#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "glutils.h"
#include "glslprogram.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>

//#include "opencv2/core/utility.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"
// 
//#include <opencv2/optflow.hpp>


class gDisOptFlow
{
public:
	gDisOptFlow() {};
	~gDisOptFlow() {};

	void compileAndLinkShader();
	void setLocations();
	void allocateBuffers();
	void allocateTextures(bool useInfrared);

	void setTextureParameters(int width, int height) { m_texture_width = width;  m_texture_height = height; }
	void setNumLevels(int width)
	{
		m_numLevels = (int)(log((2 * width) / (4.0 * m_patch_size)) / log(2.0) + 0.5) - 1;
	}
	//void setTexture(GLuint texID) { m_textureI1 = texID; }
	void setTexture(unsigned char * imageArray);
	void setTexture(float * imageArray);

	void computeSobel(int level, bool useInfrared);
	void makePatches(int level);
	//bool precomputeStructureTensor();
	bool calc(bool useInfrared);
	bool track();
	bool densification(int level);
	void wipeFlow();
	void clearPoints();

	//void resizeFlow(int level);
	bool patchInverseSearch(int level, bool useInfrared);
	void variationalRefinement(int level);
	void variRef(int level);

	void swapTextures();
	GLuint getFlowTexture()
	{
		return m_textureU_x_y;
	}
	GLuint getColorTexture()
	{
		return m_textureI1;
	}
	GLuint getTrackedPointsBuffer()
	{
		return m_trackedPointsBuffer;
	}
	double getTimeElapsed()
	{
		return m_timeElapsed;
	}
	void setTrackedPoint(float x, float y)
	{
		//m_trackedPoint = cv::Point2f(x, y);
	}

	bool firstFrame = true;

private:

	//cv::Mat I0im, I1im;

	//cv::Point2f m_trackedPoint = cv::Point2f(1920 >> 1, 1080 >> 1);
	std::vector<float> m_trackedPoints;

	GLuint m_trackedPointsBuffer;


	int variational_refinement_iter;
	float variational_refinement_alpha;
	float variational_refinement_gamma;
	float variational_refinement_delta;
	int getVariationalRefinementIterations() const { return variational_refinement_iter; }
	void setVariationalRefinementIterations(int val) { variational_refinement_iter = val; }
	float getVariationalRefinementAlpha() const { return variational_refinement_alpha; }
	void setVariationalRefinementAlpha(float val) { variational_refinement_alpha = val; }
	float getVariationalRefinementDelta() const { return variational_refinement_delta; }
	void setVariationalRefinementDelta(float val) { variational_refinement_delta = val; }
	float getVariationalRefinementGamma() const { return variational_refinement_gamma; }
	void setVariationalRefinementGamma(float val) { variational_refinement_gamma = val; }
	//std::vector<cv::Ptr<cv::optflow::VariationalRefinement> > variational_refinement_processors;

	GLuint timeQuery[1];
	double m_timeElapsed = 0.0;

	inline int divup(int a, int b) { return (a % b != 0) ? (a / b + 1) : (a / b); }

	GLSLProgram disFlowProg;
	GLSLProgram sobelProg;
	GLSLProgram variRefineProg;

	//Locations
	/* subroutines */
	GLuint m_subroutine_SobelID;
	GLuint m_getGradientsID;
	GLuint m_getSmoothnessID;

	GLuint m_subroutine_DISflowID;
	GLuint m_makePatchesID;

	GLuint m_makePatchesHorID;
	GLuint m_makePatchesVerID;


	GLuint m_patchInverseSearchID;
	GLuint m_patchInverseSearchDescentID;
	GLuint m_densificationID;
	GLuint m_resizeID;
	GLuint m_trackID;
	GLuint m_prefixSum2D_HorID, m_prefixSum2D_VerID;
	GLuint m_patch_sizeID;
	GLuint m_patch_strideID;
	GLuint m_trackWidthID;

	GLuint m_subroutine_variRefineID;
	GLuint m_prepareBuffersID;
	GLuint m_computeDataTermID;
	GLuint m_computeSmoothnessTermID;
	GLuint m_computeSORID;

	/* uniforms */
	GLuint m_level_cov_ID;
	GLuint m_imageType_cov_ID;
	GLuint m_imageType_dis_ID;
	GLuint m_level_dis_ID;
	GLuint m_iter_dis_ID;
	GLuint m_level_var_ID;
	GLuint m_flipflopID;
	GLuint m_iter_var_ID;
	GLuint m_texSizeID;

	//Buffers
	std::vector<float> m_refinementDataTerms;
	GLuint m_buffer_refinement_data_terms;

	//GLuint m_bufferU; //!< a buffer for the merged flow

	//GLuint m_textureSx; //!< intermediate sparse flow representation (x component)
	//GLuint m_textureSy; //!< intermediate sparse flow representation (y component)
	GLuint m_textureS_x_y;
	GLuint m_textureI1_s_ext;

	GLuint m_textureTest;



	// variational refineing textures
	GLuint m_textureI_mix_diff_warp; //  averageI, Iz, warpedI
	GLuint m_textureI_grads_mix_diff_x_y; // Ix, Iy, Ixz, Iyz
	GLuint m_textureI_second_grads_mix_diff_x_y; // Ixx, Ixy, Iyy
	GLuint m_texture_dup_dvp;
	GLuint m_texture_total_flow;
	GLuint m_texture_smoothness_terms;
	GLuint m_texture_smoothness_weight;

	//Textures
	GLuint createTexture(GLenum target, int levels, int w, int h, int d, GLuint internalformat);

	GLuint m_textureI0_prod_xx_yy_xy;
	GLuint m_textureI0_sum_x_y;
	GLuint m_textureI0_grad_x_y;
	GLuint m_textureI0_prod_xx_yy_xy_aux;
	GLuint m_textureI0_sum_x_y_aux;


	GLuint m_textureI0;
	GLuint m_textureI1;
	GLuint m_textureU;

	GLuint m_textureWarp_I1;
	//GLuint m_textureI0s;
	//GLuint m_textureI1s;
	GLuint m_textureI0s_I1s;

	GLuint m_textureI0_xs_ys;
	//GLuint m_textureI0ys;

	GLuint m_textureU_x_y;
	GLuint m_texture_init_U_x_y;
	GLuint m_texture_previous_U_x_y;

	//GLuint m_textureUy;

	GLuint m_textureUx_initial;
	GLuint m_textureUy_initial;

	GLuint m_texture_temp;
	GLuint m_texture_temp1;


	// parameters
	int m_texture_width;
	int m_texture_height;
	int m_patch_size = 8;
	int m_patch_stride = 4;
	int m_border_size = 16;
	int m_ws; // 1 + (width - patch_size) / patch_stride
	
	
	int swapCounter = 0;
	bool flipflop = false;

	int m_numLevels;// = (int)(log((2 * 1920) / (4.0 * m_patch_size)) / log(2.0) + 0.5) - 1;



	std::vector<float> zeroValues = std::vector<float>(1920 * 1080 * 4, 0.0f);




};