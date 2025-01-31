#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <gl/GL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "WindowManager.h"
#include "Logger.h"
#include "Timer.h"
#include "camera.h"
#include "Shader.h"
#include "model.h"



//*** Globle Function Declarations ***
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
unsigned int loadTexture(const char* path, bool gammaCorrection);
void renderQuad();
void renderCube();

//*** Global Variable Declaration ***
WindowManager* pWindow = NULL;
Camera* camera = NULL;
float lastX = WindowManager::SCR_WIDTH / 2.0f;
float lastY = WindowManager::SCR_HEIGHT / 2.0f;
bool firstMouse = true;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


///==================== OpenGL Variables =======================///
const glm::vec3 LIGHT_COLOR = glm::vec3(1.0f, 0.0f, 0.0f); // CHANGE AREA LIGHT COLOR HERE!
bool keys[1024]; // activated keys
glm::vec3 areaLightTranslate;
Shader* ltcShaderPtr;


struct VertexAL {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
};

const GLfloat psize = 10.0f;
VertexAL planeVertices[6] = {
	{ {-psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
	{ {-psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
	{ { psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
	{ {-psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
	{ { psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
	{ { psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} }
};
VertexAL areaLightVertices[6] = {
	{ {-8.0f, 2.4f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, // 0 1 5 4
	{ {-8.0f, 2.4f,  1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
	{ {-8.0f, 0.4f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
	{ {-8.0f, 2.4f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
	{ {-8.0f, 0.4f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
	{ {-8.0f, 0.4f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} }
};

GLuint planeVBO, planeVAO;
GLuint areaLightVBO, areaLightVAO;

void configureMockupData()
{
	// PLANE
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);

	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
		(GLvoid*)0);
	glEnableVertexAttribArray(0);

	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
		(GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// texcoord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
		(GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// AREA LIGHT
	glGenVertexArrays(1, &areaLightVAO);
	glBindVertexArray(areaLightVAO);

	glGenBuffers(1, &areaLightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, areaLightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(areaLightVertices), areaLightVertices, GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
		(GLvoid*)0);
	glEnableVertexAttribArray(0);

	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
		(GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// texcoord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
		(GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	glBindVertexArray(0);
}

void renderPlane()
{
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void renderAreaLight()
{
	glBindVertexArray(areaLightVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}



struct LTC_matrices {
	GLuint mat1;
	GLuint mat2;
};

GLuint loadMTexture()
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
		0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

GLuint loadLUTTexture()
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
		0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}



void incrementRoughness(float step)
{
	static glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f);
	static float roughness = 0.5f;
	roughness += step;
	roughness = glm::clamp(roughness, 0.0f, 1.0f);
	//std::cout << "roughness: " << roughness << '\n';
	ltcShaderPtr->use();
	ltcShaderPtr->setVec4("material.albedoRoughness", glm::vec4(color, roughness));
	glUseProgram(0);
}

void incrementLightIntensity(float step)
{
	static float intensity = 4.0f;
	intensity += step;
	intensity = glm::clamp(intensity, 0.0f, 10.0f);
	//std::cout << "intensity: " << intensity << '\n';
	ltcShaderPtr->use();
	ltcShaderPtr->setFloat("areaLight.intensity", intensity);
	glUseProgram(0);
}

void switchTwoSided(bool doSwitch)
{
	static bool twoSided = true;
	if (doSwitch) twoSided = !twoSided;
	//std::cout << "twoSided: " << std::boolalpha << twoSided << '\n';
	ltcShaderPtr->use();
	ltcShaderPtr->setFloat("areaLight.twoSided", twoSided);
	glUseProgram(0);
}





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	pWindow = new WindowManager();
	MSG msg = { 0 };
	camera = new Camera();

	Logger::Init();

	TIMER_INIT("Window");
	pWindow->initialize();
	TIMER_END(); 
	LOG_INFO("Window Initialized in %.6f seconds", TIMER_GET("Window"));



	///======================== OpenGL INIT ==============================///
	glEnable(GL_DEPTH_TEST);

	// LUT textures
	LTC_matrices mLTC;
	mLTC.mat1 = loadMTexture();
	mLTC.mat2 = loadLUTTexture();

	// SHADERS
	Shader shaderLTC("shaders/area_light.vs", "shaders/area_light.fs");
	ltcShaderPtr = &shaderLTC;
	Shader shaderLightPlane("shaders/light_plane.vs", "shaders/light_plane.fs");

	// TEXTURES
	unsigned int concreteTexture = loadTexture("resources/textures/concreteTexture.png", true);

	// SHADER CONFIGURATION
	shaderLTC.use();
	shaderLTC.setVec3("areaLight.points[0]", areaLightVertices[0].position);
	shaderLTC.setVec3("areaLight.points[1]", areaLightVertices[1].position);
	shaderLTC.setVec3("areaLight.points[2]", areaLightVertices[4].position);
	shaderLTC.setVec3("areaLight.points[3]", areaLightVertices[5].position);
	shaderLTC.setVec3("areaLight.color", LIGHT_COLOR);
	shaderLTC.setInt("LTC1", 0);
	shaderLTC.setInt("LTC2", 1);
	shaderLTC.setInt("material.diffuse", 2);
	incrementRoughness(0.0f);
	incrementLightIntensity(0.0f);
	switchTwoSided(false);
	glUseProgram(0);

	shaderLightPlane.use();
	{
		glm::mat4 model(1.0f);
		shaderLightPlane.setMat4("model", model);
	}
	shaderLightPlane.setVec3("lightColor", LIGHT_COLOR);
	glUseProgram(0);

	// 3D OBJECTS
	configureMockupData();
	areaLightTranslate = glm::vec3(0.0f, 0.0f, 0.0f);





	//*** Game LOOP ***
	while (pWindow->isRunning == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				pWindow->isRunning = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			///=========================== DISPLAY ==================================//
			// per-frame time logic
			// --------------------
			float currentFrame = static_cast<float>(Timer::getAppRunTime() * 0.05f);
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;


			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shaderLTC.use();
			glm::mat4 model(1.0f);
			glm::mat3 normalMatrix = glm::mat3(model);
			shaderLTC.setMat4("model", model);
			shaderLTC.setMat3("normalMatrix", normalMatrix);
			glm::mat4 view = camera->GetViewMatrix();
			shaderLTC.setMat4("view", view);
			glm::mat4 projection = glm::perspective(
				glm::radians(camera->Zoom), (float)WindowManager::SCR_WIDTH / (float)WindowManager::SCR_HEIGHT, 0.1f, 100.0f);
			shaderLTC.setMat4("projection", projection);
			shaderLTC.setVec3("viewPosition", camera->Position);
			shaderLTC.setVec3("areaLightTranslate", areaLightTranslate);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mLTC.mat1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mLTC.mat2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, concreteTexture);
			renderPlane();
			glUseProgram(0);

			shaderLightPlane.use();
			model = glm::translate(model, areaLightTranslate);
			shaderLightPlane.setMat4("model", model);
			shaderLightPlane.setMat4("view", view);
			shaderLightPlane.setMat4("projection", projection);
			renderAreaLight();
			glUseProgram(0);


			pWindow->swapDisplayBuffer();

			///================== UPDATE =======================//
			//angleCube = angleCube + 0.02f;

		}
	}


	return((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//*** Function Declaration ***
	void ToggleFullscreen(void);
	void resize(int, int);


	//*** Code ***
	switch (iMsg)
	{
	case WM_SETFOCUS:
		break;

	case WM_KILLFOCUS:
		break;

	case WM_SIZE:
        glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_MOUSEMOVE:
		{
			float xpos = static_cast<float>(GET_X_LPARAM(lParam));
			float ypos = static_cast<float>(GET_Y_LPARAM(lParam));

			if (firstMouse)
			{
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

			lastX = xpos;
			lastY = ypos;
			camera->ProcessMouseMovement(xoffset, yoffset);
		}
		
		break;

	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_CHAR:
		switch (LOWORD(wParam))
		{
		case 'F':
		case 'f':
			
			break;

		case 'W':
		case 'w':
			camera->ProcessKeyboard(FORWARD, deltaTime);
			break;

		case 'A':
		case 'a':
			camera->ProcessKeyboard(LEFT, deltaTime);
			break;

		case 'S':
		case 's':
			camera->ProcessKeyboard(BACKWARD, deltaTime);
			break;

		case 'D':
		case 'd':
			camera->ProcessKeyboard(RIGHT, deltaTime);
			break;

		
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}


	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}


unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadTexture(char const* path, bool gammaCorrection)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1)
		{
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
			 // bottom face
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 // top face
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
			  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

