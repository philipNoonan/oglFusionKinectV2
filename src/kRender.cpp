#include "kRender.h"

kRender::~kRender()
{
}

void kRender::GLFWCallbackWrapper::MousePositionCallback(GLFWwindow* window, double positionX, double positionY)
{
	s_application->MousePositionCallback(window, positionX, positionY);
}

void kRender::GLFWCallbackWrapper::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	s_application->MouseButtonCallback(window, button, action, mods);
}


void kRender::GLFWCallbackWrapper::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	s_application->KeyboardCallback(window, key, scancode, action, mods);
}

void kRender::GLFWCallbackWrapper::SetApplication(kRender* application)
{
	GLFWCallbackWrapper::s_application = application;
}

kRender* kRender::GLFWCallbackWrapper::s_application = nullptr;

void kRender::MousePositionCallback(GLFWwindow* window, double positionX, double positionY)
{
	//...
	//std::cout << "mouser" << std::endl;
	m_mouse_pos_x = positionX;
	m_mouse_pos_y = positionY;
}
void kRender::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 1500.0f;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...

																		// the height of the screen at the distance of the image is 2 * halfheight
																		// to go from the middle to the top 

																		//m_model_depth = glm::translate(glm::mat4(1.0f), glm::vec3(-halfWidthAtDistance, halfHeightAtDist - m_depth_height, -zDist));

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && m_selectInitialPoseFlag == true)
	{
		if (m_mouse_pos_x > 32 && m_mouse_pos_x < m_depth_width + 32 && m_mouse_pos_y < 32 + 424 && m_mouse_pos_y > 32)
		{
			m_center_pixX = m_mouse_pos_x - 32;
			m_center_pixY = m_mouse_pos_y - 32;

			//std::cout << "x: " << m_center_pixX  << " y: " << m_center_pixY << std::endl;

			// get depth value, from texture buffer or float array???

			//// need to get depth pixel of this point
			//float x = (pixX - m_cameraParams.z) * (1.0f / m_cameraParams.x) * depth.x;
			//float y = (pixY - m_cameraParams.w) * (1.0f / m_cameraParams.y) * depth.x;
			//float z = depth.x;

			//m_cameraParams.x
		}
	}


	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// m_depthPixelPoints2D.push_back(std::make_pair(m_mouse_pos_x, m_mouse_pos_y));
		// get correct current offset and scakle for the window
		int depth_pos_x = m_mouse_pos_x / m_render_scale_width;
		int depth_pos_y = m_mouse_pos_y / m_render_scale_height;

		//std::cout <<" x: " << m_mouse_pos_x << " y: " << m_mouse_pos_y << " xS: " << m_render_scale_width << " yS: " << m_render_scale_height << std::endl;
		//std::cout << ((float)h / 424.0f) * m_mouse_pos_y << std::endl;


		if (depth_pos_x < m_depth_width && depth_pos_y < m_depth_height)
		{
			m_depthPixelPoints2D.push_back(std::make_pair(depth_pos_x, depth_pos_y));
			m_depthPointsFromBuffer.resize(m_depthPixelPoints2D.size() * 4); // for 4 floats per vertex (x,y,z, + padding)


		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (m_depthPixelPoints2D.size() > 0)
		{
			m_depthPixelPoints2D.pop_back();
			m_depthPointsFromBuffer.resize(m_depthPixelPoints2D.size() * 4); // for 4 floats per vertex (x,y,z, + padding)

		}
		// pop_back entry on vector
	}

	if (m_depthPixelPoints2D.size() > 0 && action == GLFW_PRESS)
	{
		std::cout << m_depthPixelPoints2D.size();
		for (auto i : m_depthPixelPoints2D)
		{
			//std::cout << " x: " << i.first << " y: " << i.second << std::endl;
		}
	}
	else if (m_depthPixelPoints2D.size() == 0 && action == GLFW_PRESS)
	{
		std::cout << "no entries yet, left click points on depth image" << std::endl;
	}
	//std::cout << "mouse button pressed: " << button << " " << action << " x: " <<  m_mouse_pos_x << " y: " << m_mouse_pos_y << std::endl;

}

void kRender::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//...
	//std::cout << "keyer" << std::endl;
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
		m_show_imgui = !m_show_imgui;
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		m_registration_matrix[3][0] -= 5.0f;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		m_registration_matrix[3][0] += 5.0f;
	if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS)
		m_registration_matrix[3][1] -= 5.0f;
	if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS)
		m_registration_matrix[3][1] += 5.0f;
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		m_registration_matrix[3][2] -= 5.0f;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		m_registration_matrix[3][2] += 5.0f;

	std::cout << "manual x " << m_registration_matrix[3][0] << " manual y " << m_registration_matrix[3][1] << " manual z " << m_registration_matrix[3][2] << std::endl;
}

void kRender::SetCallbackFunctions()
{
	GLFWCallbackWrapper::SetApplication(this);
	glfwSetCursorPosCallback(m_window, GLFWCallbackWrapper::MousePositionCallback);
	glfwSetKeyCallback(m_window, GLFWCallbackWrapper::KeyboardCallback);
	glfwSetMouseButtonCallback(m_window, GLFWCallbackWrapper::MouseButtonCallback);
}

GLFWwindow * kRender::loadGLFWWindow()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_REFRESH_RATE, 30);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	m_window = glfwCreateWindow(m_screen_width, m_screen_height, "oglfusion", nullptr, nullptr);

	if (m_window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return -1;
	}

	glfwMakeContextCurrent(m_window);
	//glfwSwapInterval(1); // Enable vsync
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		//return -1;
	}


	return m_window;
}

void kRender::requestShaderInfo()
{
	renderProg.printActiveUniforms();
}

void kRender::compileAndLinkShader()
{
	try {
		renderProg.compileShader("shaders/vertShader.vs");
		renderProg.compileShader("shaders/fragShader.fs");
		renderProg.link();


	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

void kRender::setLocations()
{
	m_ProjectionID = glGetUniformLocation(renderProg.getHandle(), "projection");
	m_MvpID = glGetUniformLocation(renderProg.getHandle(), "MVP");
	m_ModelID = glGetUniformLocation(renderProg.getHandle(), "model");
	m_ViewProjectionID = glGetUniformLocation(renderProg.getHandle(), "ViewProjection");
	m_sliceID = glGetUniformLocation(renderProg.getHandle(), "slice");
	m_imSizeID = glGetUniformLocation(renderProg.getHandle(), "imSize");

	m_getPositionSubroutineID = glGetSubroutineUniformLocation(renderProg.getHandle(), GL_VERTEX_SHADER, "getPositionSubroutine");
	m_fromTextureID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromTexture");
	m_fromPosition4DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromPosition4D");
	m_fromPosition2DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromPosition2D");

	m_fromStandardTextureID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromStandardTexture");

	m_colorSelectionRoutineID = glGetSubroutineUniformLocation(renderProg.getHandle(), GL_FRAGMENT_SHADER, "getColorSelection");
	m_fromDepthID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromDepth");
	m_fromColorID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromColor");
	m_fromRayNormID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromRayNorm");
	m_fromRayVertID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromRayVert");
	m_fromPointsID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromPoints");
	m_fromVolumeID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromVolume");
	m_fromTrackID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromTrack");
	m_fromFlowID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromFlow");




	//m_ambientID = glGetUniformLocation(renderProg.getHandle(), "ambient");
	//m_lightID = glGetUniformLocation(renderProg.getHandle(), "light");

	//m_irLowID = glGetUniformLocation(renderProg.getHandle(), "irLow");
	//m_irHighID = glGetUniformLocation(renderProg.getHandle(), "irHigh");

}

void kRender::updateVerts(float w, float h)
{
	std::vector<float> vertices = {
		// Positions		// Texture coords
		w / 2.0f, h / 2.0f, 0.0f, 1.0f, 1.0f, // top right
		w / 2.0f, -h / 2.0f, 0.0f, 1.0f, 0.0f, // bottom right
		-w / 2.0f, -h / 2.0f, 0.0f, 0.0f, 0.0f, // bottom left
		-w / 2.0f, h / 2.0f, 0.0f, 0.0f, 1.0f  // Top left
	};

	m_standard_verts = vertices;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Standard);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_standard_verts.size() * sizeof(float), m_standard_verts.data());

}

void kRender::setVertPositions()
{
	std::vector<float> vertices = {
		// Positions				// Texture coords
		1.0f,	1.0f,	0.0f,		1.0f, 1.0f, // top right
		1.0f,	-1.0f,	0.0f,		1.0f, 0.0f, // bottom right
		-1.0f,	-1.0f,	0.0f,		0.0f, 0.0f, // bottom left
		-1.0f,	1.0f,	0.0f,		0.0f, 1.0f  // Top left
	};

	m_standard_verts = vertices;

	std::vector<float> verticesColor = {
		// Positions				// Texture coords
		1920,	1080,	0.0f,		1.0f, 1.0f, // top right
		1920,	0,		0.0f,		1.0f, 0.0f, // bottom right
		0,		0,		0.0f,		0.0f, 0.0f, // bottom left
		0,		1080.0,	0.0f,		0.0f, 1.0f  // Top left
	};
	m_color_vert = verticesColor;

	std::vector<float> verticesDepth = {
		// Positions					// Texture coords
		512.0f,  424.0f,	0.0f,		1.0f, 1.0f, // top right
		512.0f,	 0,			0.0f,		1.0f, 0.0f, // bottom right
		0,		 0,			0.0f,		0.0f, 0.0f, // bottom left
		0,		 424.0f,	0.0f,		0.0f, 1.0f  // Top left 
	};
	m_depth_vert = verticesDepth;

	m_vertices = vertices;

	std::vector<unsigned int>  indices = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	};

	m_indices = indices;

	m_trackedPoints.resize(1000 * 1000 * 2);

	for (int i = 0; i < 2000; i += 2)
	{
		for (int j = 0; j < 1000; j++)
		{
			m_trackedPoints[j * 2000 + i] = (1920 >> 1) - 500 + (i / 2) * 10;
			m_trackedPoints[j * 2000 + i + 1] = (1080 >> 1) - 500 + j * 10;

		}
	}



}

void kRender::allocateBuffers()
{
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO_Standard);
	glGenBuffers(1, &m_VBO_Color);
	glGenBuffers(1, &m_VBO_Depth);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Color);
	glBufferData(GL_ARRAY_BUFFER, m_color_vert.size() * sizeof(float), &m_color_vert[0], GL_STATIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
	// Position attribute for Color
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute for Color
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// now go for depth
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Depth);
	glBufferData(GL_ARRAY_BUFFER, m_depth_vert.size() * sizeof(float), &m_depth_vert[0], GL_STATIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
	// Position attribute for Depth
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(2);
	//// TexCoord attribute for Depth
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(3);

	// standard verts
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Standard);
	glBufferData(GL_ARRAY_BUFFER, m_standard_verts.size() * sizeof(float), &m_standard_verts[0], GL_DYNAMIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_DYNAMIC_DRAW);
	// Position attribute for Depth
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(4);
	//// TexCoord attribute for Depth
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(5);



	glBindVertexArray(0);

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_posBufMC);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 128*128*128*4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &m_VAO_MC);
	glGenBuffers(1, &m_bufferTrackedPoints);

	glBindVertexArray(m_VAO_MC);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferTrackedPoints);
	glBufferData(GL_ARRAY_BUFFER, 100 * 100 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 0, 0); // 2  floats per vertex, x,y
	glEnableVertexAttribArray(7);

	glBindVertexArray(0);

	//glBindBuffer(GL_ARRAY_BUFFER, m_posBufMC);
	//glBufferData(GL_ARRAY_BUFFER, 128 * 128 * 128 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	//glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, 0); // 4  floats per vertex, x,y,z and 1 for padding? this is annoying...
	//glEnableVertexAttribArray(6);

	//glBindVertexArray(0);


	//glGenBuffers(1, &m_bufferTrackedPoints);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferTrackedPoints);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferTrackedPoints);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, m_trackedPoints.size() * sizeof(float), m_trackedPoints.data(), GL_DYNAMIC_DRAW);


}

void kRender::setWindowLayout()
{
	m_anchorMW = std::make_pair<int, int>(50, 50);


}

void kRender::setComputeWindowPosition()
{
	glViewport(m_anchorMW.first + m_depth_width * m_render_scale_width, m_anchorMW.second, m_depth_width * m_render_scale_width, m_depth_height * m_render_scale_height);
}

//
//void kRender::setColorDepthMapping(int* colorDepthMap)
//{
//	// 2d array index is given by
//	// p.x = idx / size.x
//	// p.y = idx % size.x
//
//	//for m_colorDepthMapping[j + 1] = y color image axis, 1 at top
//	// m_colorDepthMapping[j] = x axis, 0 on the left, 
//
//	// MAP ME¬¬¬
//	int j = 0;
//	for (int i = 0; i < (m_depth_width * m_depth_height); i++, j+=2)
//	{
//		int yCoord = colorDepthMap[i] / m_color_width;
//		int xCoord = colorDepthMap[i] % m_color_width;
//		m_colorDepthMapping[j] = ((float)xCoord) / (float)m_color_width;
//		m_colorDepthMapping[j + 1] = (1080.0f - (float)yCoord) / (float)m_color_height;
//
//
//
//	}
//
//
//
//	glBindBuffer(GL_ARRAY_BUFFER, m_buf_color_depth_map);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, m_colorDepthMapping.size() * sizeof(float), m_colorDepthMapping.data());
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//	//// Other way to copy data to buffer, taken from https://learnopengl.com/#!Advanced-OpenGL/Advanced-Data
//	//glBindBuffer(GL_ARRAY_BUFFER, m_buf_color_depth_map);
//	//void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
//	//memcpy_s(ptr, m_colorDepthMapping.size() * sizeof(float), m_colorDepthMapping.data(), m_colorDepthMapping.size() * sizeof(float));
//	//glUnmapBuffer(GL_ARRAY_BUFFER);
//
//}


void kRender::setRenderingOptions(bool showDepthFlag, bool showBigDepthFlag, bool showInfraFlag, bool showColorFlag, bool showLightFlag, bool showPointFlag, bool showFlowFlag, bool showEdgesFlag, bool showNormalFlag, bool showVolumeSDFFlag, bool showTrackFlag)
{
	m_showDepthFlag = showDepthFlag;
	m_showBigDepthFlag = showBigDepthFlag;
	m_showInfraFlag = showInfraFlag;
	m_showColorFlag = showColorFlag;
	m_showLightFlag = showLightFlag;
	m_showPointFlag = showPointFlag;
	m_showFlowFlag = showFlowFlag;
	m_showEdgesFlag = showEdgesFlag;
	m_showNormalFlag = showNormalFlag;
	m_showVolumeSDFFlag = showVolumeSDFFlag;
	m_showTrackFlag = showTrackFlag;
}

void kRender::setTextures(GLuint depthTex, GLuint colorTex, GLuint vertexTex, GLuint normalTex, GLuint volumeTex, GLuint trackTex)
{

	m_textureDepth = depthTex;
	m_textureColor = colorTex;
	m_textureVertex = vertexTex;
	m_textureNormal = normalTex;
	m_textureVolume = volumeTex;
	m_textureTrack = trackTex;

}
void kRender::setFlowTexture(GLuint flowTex)
{
	m_textureFlow = flowTex;
}

void kRender::bindTexturesForRendering()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	if (m_showDepthFlag)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	}

	if (m_showNormalFlag)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_textureNormal);
	}

	if (m_showTrackFlag)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_textureTrack);

	}

	if (m_showFlowFlag)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_textureFlow);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_textureColor);
	}


	if (m_showVolumeSDFFlag)
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_3D, m_textureVolume);
	}





}

void kRender::bindBuffersForRendering()
{
	glBindVertexArray(m_VAO_MC);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferTrackedPoints);
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(7);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_posBufMC);
	//glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0); // 4  floats per vertex, x,y,z and 1 for padding? this is annoying...
	//glEnableVertexAttribArray(4);


}



void kRender::Render(bool useInfrared)
{
	// set positions
	// set uniforms
	// render textures
	bindTexturesForRendering();
	bindBuffersForRendering();
	//setDepthImageRenderPosition();
	setNormalImageRenderPosition();
	setViewport(32, 900 - 32 - 424, 512, 424);

	renderLiveVideoWindow(useInfrared);



}




// set up the data buffers for rendering
void kRender::setBuffersForRendering(float * depthArray, float * bigDepthArray, float * infraArray, float * colorArray, unsigned char * flowPtr)
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//if (depthArray != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, m_textureDepth);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_depth_width, m_depth_height, GL_RED, GL_FLOAT, depthArray);
	//}


	//if (infraArray != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, m_textureInfra);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_depth_width, m_depth_height, GL_RED, GL_FLOAT, infraArray);
	//}


	//if (m_showColorFlag || colorArray != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE2);
	//	glBindTexture(GL_TEXTURE_2D, m_textureColor);
	//	if (colorArray != NULL)
	//	{
	//		glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_color_width, m_color_height, GL_BGRA, GL_UNSIGNED_BYTE, colorArray);
	//	}
	//}


	//if (flowPtr != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE3);
	//	glBindTexture(GL_TEXTURE_2D, m_textureFlow);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_depth_width, m_depth_height, GL_RG, GL_FLOAT, flowPtr);
	//}


	//if (flowPtr != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE3); // what number should this be FIX ME
	//	glBindTexture(GL_TEXTURE_2D, m_textureFlow);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_depth_width, m_depth_height, GL_RG, GL_FLOAT, flowPtr);
	//}


	//if (bigDepthArray != NULL)
	//{
	//	glActiveTexture(GL_TEXTURE6);
	//	glBindTexture(GL_TEXTURE_2D, m_textureBigDepth);
	//	glTexSubImage2D(GL_TEXTURE_2D, /*level*/0, /*xoffset*/0, /*yoffset*/0, m_color_width, m_color_height + 2, GL_RED, GL_FLOAT, bigDepthArray);
	//}

	//if (m_showEdgesFlag)
	//{
	//	glActiveTexture(GL_TEXTURE7);
	//	glBindTexture(GL_TEXTURE_2D, m_textureEdges);
	//}


}

void kRender::setMarchingCubesRenderPosition(float modelZ)
{

	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 0000.0f + modelZ;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...

	m_model_MC = glm::translate(glm::mat4(1.0f), glm::vec3(-500, -500, -zDist)); // not sure why we have to offset by 0.5m xy

	glm::mat4 flipYZ = glm::mat4(1);
	//flipYZ[0][0] = -1.0f;
	flipYZ[1][1] = -1.0f;
	flipYZ[2][2] = -1.0f;

	m_model_MC = flipYZ * m_model_MC;
}

void kRender::setVolumeSDFRenderPosition(float slice)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist = m_volume_size.z;
	//zDist = ((float)128 * 6) / tan(20 * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(1.f, 1.f, 1.0f);

	m_model_volume = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_volume = glm::translate(m_model_volume, glm::vec3(0, 0, -zDist));

	m_volumeSDFRenderSlice = slice / m_volume_size.z;

}

void kRender::setDepthImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)424 * 6) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_depth = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_depth = glm::translate(m_model_depth, glm::vec3(-m_depth_width / 2, -m_depth_height / 2, -zDist));


}

void kRender::setRayNormImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)m_depth_height * 6) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_raynorm = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_raynorm = glm::translate(m_model_raynorm, glm::vec3(-m_depth_width / 2, -m_depth_height / 2, -zDist + 2));


}

void kRender::setTrackImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)424 * 6) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;

	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_track = glm::scale(glm::mat4(1.0f), scaleVec);

	m_model_track = glm::translate(m_model_track, glm::vec3(-m_depth_width / 2, -m_depth_height / 2, -zDist));
}

void kRender::setNormalImageRenderPosition()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 1500.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	float zDist;
	zDist = ((float)h / 2) / tan(22.5 * M_PI / 180.0f);
	float halfHeightAtDist = (float)h / 2;
	float halfWidthAtDistance = (float)w / 2;
	m_model_normal = glm::translate(glm::mat4(1.0f), glm::vec3(-m_depth_width / 2, -m_depth_height / 2, -zDist + 5));


}

void kRender::setColorImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//// if setting z dist
	//float zDist = 8000.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	//// else if setting size on window
	float zDist;
	zDist = ((float)m_color_height * 6) / tan(vertFov * M_PI / 180.0f);
	float halfHeightAtDist = (float)h * 4;
	float halfWidthAtDistance = (float)w * 4;
	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, -halfHeightAtDist, -zDist));
	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_color = glm::scale(glm::mat4(1.0f), scaleVec);
	m_model_color = glm::translate(m_model_color, glm::vec3(-m_color_width / 2.0f, -m_color_height / 2.0f, -zDist));

	//std::cout << "zDis" << zDist << "w " << w << " h" << h << " ad " << halfWidthAtDistance << std::endl;
	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, 0.0f, -2000.0f));
}

void kRender::setInfraImageRenderPosition()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 1500.0f;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	m_model_infra = glm::translate(glm::mat4(1.0f), glm::vec3(-halfWidthAtDistance, -halfHeightAtDist, -zDist));


}

void kRender::setFlowImageRenderPosition(int height, int width, float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//// if setting z dist
	//float zDist = 8000.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	//// else if setting size on window
	float zDist;
	zDist = ((float)height * 6) / tan(vertFov * M_PI / 180.0f);
	float halfHeightAtDist = (float)h * 4;
	float halfWidthAtDistance = (float)w * 4;
	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, -halfHeightAtDist, -zDist));
	glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	m_model_flow = glm::scale(glm::mat4(1.0f), scaleVec);
	m_model_flow = glm::translate(m_model_flow, glm::vec3(-width / 2.0f, -height / 2.0f, -zDist));

	}

void kRender::setPointCloudRenderPosition(float modelZ)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 0000.0f + modelZ;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	m_model_pointcloud = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zDist));

	glm::mat4 flipYZ = glm::mat4(1);
	//flipYZ[0][0] = -1.0f;
	flipYZ[1][1] = -1.0f;
	flipYZ[2][2] = -1.0f;

	m_model_pointcloud = m_registration_matrix * flipYZ * m_model_pointcloud;

}

void kRender::setLightModelRenderPosition()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//float zDist = 3000.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...

	float zDist;
	zDist = ((float)h * 4) / tan(30.5f * M_PI / 180.0f);
	float halfHeightAtDist = (float)h * 4;
	float halfWidthAtDistance = (float)w * 4;
	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, -halfHeightAtDist, -zDist));

	if (m_showBigDepthFlag)
	{
		glm::vec3 scaleVec = glm::vec3(4.f, 4.f, 1.0f);

		m_model_lightmodel = glm::scale(glm::mat4(1.0f), scaleVec);
		m_model_lightmodel = glm::translate(m_model_lightmodel, glm::vec3(-m_color_width / 2.0f, -m_color_height / 2.0f, -zDist + 10.0f));

	}
	else
	{
		m_model_lightmodel = glm::translate(glm::mat4(1.0f), glm::vec3(halfWidthAtDistance - m_depth_width, -halfHeightAtDist, -zDist));

	}

}


void kRender::setViewMatrix(float xRot, float yRot, float zRot, float xTran, float yTran, float zTran)
{
	glm::mat4 t0, t1, r0;
	m_view = glm::mat4(1.0f);

	t0 = glm::translate(glm::mat4(1.0), glm::vec3(xTran, yTran, zTran));
	t1 = glm::translate(glm::mat4(1.0), glm::vec3(-xTran, -yTran, -zTran));

	r0 = glm::eulerAngleXYZ(glm::radians(xRot), glm::radians(yRot), glm::radians(zRot));


	m_view = t1 * r0 * t0;
	//m_view = glm::translate(m_view, glm::vec3(0.0f, 0.0f, 0.0f));

}

void kRender::setProjectionMatrix()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	m_projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 1.0f, 10000.0f); // scaling the texture to the current window size seems to work
	glViewport(0, 0, w, h);


}

void kRender::setDepthTextureProjectionMatrix()
{

	m_projection = glm::perspective(glm::radians(45.0f), (float)m_depth_width / (float)m_depth_height, 1.0f, 10000.0f); // scaling the texture to the current window size seems to work


}

void kRender::setViewport(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
}

void kRender::renderLiveVideoWindow(bool useInfrared)
{
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderProg.use();
	glm::mat4 MVP;

	//// FOR DEPTH
	if (m_showDepthFlag)
	{
		glBindVertexArray(m_VAO);
		MVP = m_projection * m_view * m_model_depth;
		glm::vec2 imageSize(m_depth_width, m_depth_height);

		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromDepthID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	//// FOR RAYNORM
	if (m_showNormalFlag)
	{
		glBindVertexArray(m_VAO);
		MVP = m_projection * m_view * m_model_raynorm;
		glm::vec2 imageSize(m_depth_width, m_depth_height);

		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromRayNormID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	if (m_showVolumeSDFFlag)
	{
		// chnage verts for size of texture

		//updateVerts(m_volume_size.x, m_volume_size.y);
		glm::vec2 imageSize(m_volume_size.x, m_volume_size.y);

		glBindImageTexture(5, m_textureVolume, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16I);

		glBindVertexArray(m_VAO);
		MVP = m_projection * m_view * m_model_volume;
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromVolumeID);
		glUniform1f(m_sliceID, m_volumeSDFRenderSlice);

		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	if (m_showTrackFlag)
	{
		glBindVertexArray(m_VAO);
		MVP = m_projection * m_view * m_model_track;
		glm::vec2 imageSize(m_depth_width, m_depth_height);

		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromTrackID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);



	}

	if (m_showFlowFlag)
	{
		glm::vec2 imageSize;
		if (useInfrared)
		{
			imageSize = glm::vec2(m_depth_width, m_depth_height);
			MVP = m_projection * m_view * m_model_depth;
		}
		else
		{
			imageSize = glm::vec2(m_color_width, m_color_height);
			MVP = m_projection * m_view * m_model_color;
		}
		glBindVertexArray(m_VAO);


		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromColorID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(m_VAO);
		MVP = glm::translate(MVP, glm::vec3(0.0f, 0.0f, 5.0f));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromFlowID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(m_VAO_MC);

	//glBindVertexArray(m_VAO_MC);
	//glm::mat4 projection = glm::perspective(glm::radians(61.087f), 1920.0f / 1080.0f, 0.1f, 10000.0f); // 61 was obtained https://codeyarns.com/2015/09/08/how-to-compute-intrinsic-camera-matrix-for-a-camera/ from fy = y / tan(FOVy / 2) , where y = halfHeight = 1080 / 2

	//MVP = m_projection * m_view * m_model_MC;
	//glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromPosition4DID);
	//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromPointsID);
	////glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
	//glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
	//glDrawArrays(GL_TRIANGLES, 0, m_numVerts);
	////glDrawArrays(GL_POINTS, 0, m_numVerts    

	glEnable(GL_PROGRAM_POINT_SIZE);

	MVP = m_projection * m_view * m_model_flow;
	//MVP = m_projection * m_view * m_model_color;
	glm::vec2 imageSize;
	if (useInfrared)
	{
		imageSize = glm::vec2(m_depth_width, m_depth_height);
	}
	else
	{
		imageSize = glm::vec2(m_color_width, m_color_height);
	}
	glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

	MVP = glm::translate(MVP, glm::vec3(0.0f, 0.0f, 10.0f));
	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromPosition2DID);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromPointsID);
	//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
	glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_POINTS, 0, m_trackedPoints.size() / 2);


	glBindVertexArray(0);


}


