#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#include <GL/glew.h>
#include "Model.h"
#include <filesystem>
#include <glfw3.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shader.h"

#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "glfw3dll.lib")

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, bool& day, std::vector<std::string>& faces, std::string& textureFolder, unsigned int& cubemapTexture);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadCubemap(std::vector<std::string> faces);
void setFaces(bool& day, std::vector<std::string>& faces, std::string& textureFolder, unsigned int& cubemapTexture);

//Camera camera(glm::vec3(0.0f, 3.0f, -36.0f));
Camera camera(glm::vec3(-40.0f, 10.0f, 10.0f),glm::vec3(0.0f,1.0f,0.0f),0.0f,-10.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cameraLock = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D ourTexture;\n"
"void main()\n"
"{\n"
"FragColor = texture(ourTexture, TexCoord);\n"
"}\0";

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"out vec3 ourColor;\n"
"out vec2 TexCoord;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"   ourColor = aColor;\n"
"   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
"}\0";

enum Movement {
	Move,
	Stop,
	Reset
};

Movement moveTrain = Movement::Reset;
float trainPosition = -15.0f;
bool canGo = false;

float incrementNumber() {
	if (trainPosition >= -20.0f)
		return 0.06f;
	if (trainPosition >= -350.5f && trainPosition <= -350.4f && moveTrain == Movement::Move && canGo == false)
	{
		moveTrain = Movement::Stop;
		return 0.00f;
	}
	if (trainPosition <= -692.4f)
	{
		moveTrain = Movement::Stop;
		return 0.00f;
	}
	if (trainPosition >= -350.5f && trainPosition <= -349.5f && canGo == true)
	{
		canGo = false;
		moveTrain = Movement::Move;
		return 0.06f;
	}
	return 0.09f;
}

void moveTrainFunction() {
	if (moveTrain == Movement::Stop)
		return;
	float fIncrement = incrementNumber();
	switch (moveTrain)
	{
	case Move:
		trainPosition -= fIncrement;
		break;
	case Stop:

		break;
	case Reset:
		trainPosition = -0.7f;
		break;
	default:
		break;
	}
}


int main(int argc, char** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Train Project", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	glEnable(GL_DEPTH_TEST);

	// build and compile shaders

	Shader skyboxShader("skybox.vs", "skybox.fs");

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	std::filesystem::path localPath = std::filesystem::current_path();
	std::string textureFolder = localPath.string() + "/Resources/Textures";

	std::vector<std::string> faces
	{
		textureFolder + "/_right.png",
		textureFolder + "/_left.png",
		textureFolder + "/_top.png",
		textureFolder + "/_bottom.png",
		textureFolder + "/_front.png",
		textureFolder + "/_back.png"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	Model trainModel(localPath.string() + "/Resources/train/electrictrain.obj");
	Model railsModel(localPath.string() + "/Resources/train/tracks.obj");
	Model stationModel(localPath.string() + "/Resources/train/House.obj");
	Model fieldModel(localPath.string() + "/Resources/train/field.obj");
	Model brasovSignModel(localPath.string() + "/Resources/train/ExitSign_HiPoly.obj");
	Model ploiestiSignModel(localPath.string() + "/Resources/train/ExitSign_HiPoly -Ploiesti.obj");
	Model bucurestSignModel(localPath.string() + "/Resources/train/ExitSign_HiPoly -Bucuresti.obj");
	Model men(localPath.string() + "/Resources/train/Humano_01Business_01_30K.obj");
	//Model women(localPath.string() + "/Resources/woman/source/ExitSign_HiPoly -Bucuresti.obj");
	Model benchModel(localPath.string() + "/Resources/train/Bench_HighRes.obj");
	Model podeaModel(localPath.string() + "/Resources/train/plank 1 model.obj");
	Model copac(localPath.string() + "/Resources/train/Tree.obj");

	vector<Model> sign = vector<Model>{ brasovSignModel,ploiestiSignModel,bucurestSignModel };
	vector<glm::vec3>persons = vector<glm::vec3>{
		glm::vec3(3.0f, 2.5f, -5.5f),
		glm::vec3(2.9f, 2.5f, -7.2f),
		glm::vec3(1.2f, 2.5f, -5.0f),
		glm::vec3(2.2f, 2.5f, -6.5f)
	};


	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//works for multiple objects
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);


	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	bool day = true;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window, day, faces, textureFolder, cubemapTexture);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		view = camera.GetViewMatrix();

		glUseProgram(shaderProgram);

		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
		unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
		// pass them to the shaders (3 different ways)
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		glm::mat4 modelTrain = glm::mat4(1.0f);
		modelTrain = glm::translate(modelTrain, glm::vec3(0.0f, -0.7f, trainPosition)); 
		modelTrain = glm::scale(modelTrain, glm::vec3(1.0f, 1.0f, 1.0f));	
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTrain));
		trainModel.Draw(shaderProgram);

		for (int i = 1; i < 25; i++)
		{
			glm::mat4 modelCopac = glm::mat4(1.0f);
			modelCopac = glm::translate(modelCopac, glm::vec3(10.0f, -0.7f, i*-30.0f));
			modelCopac = glm::scale(modelCopac, glm::vec3(0.4f, 0.4f, 0.4f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCopac));
			copac.Draw(shaderProgram);
		}

		glm::mat4 bench = glm::mat4(1.0f);
		bench = glm::rotate(bench, (float)glm::radians(180.0f), glm::vec3(0.0f, 90.0f, 0.0));
		bench = glm::translate(bench, glm::vec3(-14.5f, -0.7f, 5.8f));
		bench = glm::scale(bench, glm::vec3(0.02f, 0.02f, 0.02f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bench));
		benchModel.Draw(shaderProgram);

		glm::mat4 modelPodea = glm::mat4(1.0f);
		modelPodea = glm::translate(modelPodea, glm::vec3(0.0f, 1.0f, trainPosition)); 
		modelPodea = glm::scale(modelPodea, glm::vec3(2.1f, 0.0f, 4.2f));	
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPodea));
		podeaModel.Draw(shaderProgram);

		for (int i = 0; i < persons.size(); i++)
		{
			glm::mat4 person = glm::mat4(1.0f);
			person = glm::rotate(person, (float)glm::radians(270.0f), glm::vec3(15.0f, 90.0f, 0.0f));
			person = glm::translate(person, persons[i]); 
			person = glm::scale(person, glm::vec3(0.01f, 0.01f, 0.01f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(person));
			men.Draw(shaderProgram);
		}



		float x = -50.0f;
		for (int i = 0; i < 5; i++)
		{
			glm::mat4 tracksModel = glm::mat4(1.0f);
			tracksModel = glm::translate(tracksModel, glm::vec3(0.0f, -0.9f, x));
			tracksModel = glm::scale(tracksModel, glm::vec3(0.002f, 0.001f, 0.005f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tracksModel));
			railsModel.Draw(shaderProgram);
			x -= 150.0f;
		}

		x = 0.0f;
		float signX = .3f;
		float position = 0.0f;
		float signPosition = 7.0f;
		for (int i = 0; i < 3; i++) {
			glm::mat4 buildingModel = glm::mat4(1.0f);
			buildingModel = glm::translate(buildingModel, glm::vec3(20.0f, position, x));
			buildingModel = glm::scale(buildingModel, glm::vec3(1.0f, 1.0f, 1.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(buildingModel));
			stationModel.Draw(shaderProgram);

			glm::mat4 signBrasovModel = glm::mat4(1.0f);
			//glMatrixMode(GL_MODELVIEW);
			signBrasovModel = glm::rotate(signBrasovModel, (float)glm::radians(270.0f), glm::vec3(0.0f, 90.0f, 0.0));
			signBrasovModel = glm::translate(signBrasovModel, glm::vec3(signX, signPosition, -12.0f));
			signBrasovModel = glm::scale(signBrasovModel, glm::vec3(2.0f, 1.0f, 1.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(signBrasovModel));
			sign[i].Draw(shaderProgram);

			x -= 350.0f;
			position -= 0.4f;
			signX -= 349.95f;
			signPosition -= 0.4f;
		}

		float xField = -400.0f, yField = 100.0f;
		for (int i = 0; i < 2; i++) {
			glm::mat4 modelField = glm::mat4(1.0f);
			modelField = glm::rotate(modelField, (float)glm::radians(91.0575f), glm::vec3(5.0f, 0.8f, -0.7f));
			modelField = glm::translate(modelField, glm::vec3(yField, xField, -13.0f));
			modelField = glm::scale(modelField, glm::vec3(16.0f, 16.0f, 0.1f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelField));
			fieldModel.Draw(shaderProgram);

			xField -= 460.0;
			yField += 160.0f;
		}
		

		moveTrainFunction();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


void processInput(GLFWwindow* window, bool& day, std::vector<std::string>& faces, std::string& textureFolder, unsigned int& cubemapTexture)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		day = true;
		setFaces(day, faces, textureFolder, cubemapTexture);
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		day = false;
		setFaces(day, faces, textureFolder, cubemapTexture);
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		moveTrain = Movement::Move;
		canGo = true;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		moveTrain = Movement::Stop;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		cameraLock = true;
		camera.SetCameraPosition(glm::vec3(0.0f, 3.0f, -36.0f));
	}

}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void setFaces(bool& day, std::vector<std::string>& faces, std::string& textureFolder, unsigned int& cubemapTexture) {
	if (day)
	{
		faces =
		{
		   textureFolder + "/_right.png",
		textureFolder + "/_left.png",
		textureFolder + "/_top.png",
		textureFolder + "/_bottom.png",
		textureFolder + "/_front.png",
		textureFolder + "/_back.png"
		};
	}
	else
	{
		faces =
		{
		   textureFolder + "/_right_night.jpg",
		textureFolder + "/_left_night.jpg",
		textureFolder + "/_top_night.jpg",
		textureFolder + "/_bottom_night.jpg",
		textureFolder + "/_front_night.jpg",
		textureFolder + "/_back_night.jpg"
		};
	}
	cubemapTexture = loadCubemap(faces);
}