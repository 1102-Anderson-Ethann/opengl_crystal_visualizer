#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stack>
#include <fstream>
#include <string>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "XRDPoints.h"
#include "CIFParser.h"
#include "AtomData.h"
#include "tinyfiledialogs.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

#define numVAOs 1

float camAzimuth = 0.0f;
float camElevation = 0.3f;
float camRadius = 25.0f;
bool isDragging = false;
double lastMouseX = 0.0, lastMouseY = 0.0;

GLuint renderingProgram;
GLuint xrdVAO, xrdVBO;
int xrdPointCount;

GLuint atomProgram;
GLuint atomVAO, atomVBO, atomInstanceVBO;
int atomPointCount;
int atomCount;

GLuint wireframeProgram;
GLuint wireframeVAO, wireframeVBO;

GLuint vLoc, projLoc, tfLoc, mvLoc;
int width, height;
float aspect, timeFactor;
glm::mat4 pMat, vMat, mMat, mvMat, tMat, rMat;

stack<glm::mat4> mvStack;

bool showReciprocal = true;
bool showWireframe = true;

CIFData cifData;

struct AtomInstance {
	glm::vec3 center;
	float radius;
	glm::vec3 color;
	float padding;
};

void setupVertices(void);
GLuint createShaderProgram(const char* vertFile, const char* fragFile);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void setupAtoms(const CIFData& cifData);
void setupWireframe(const CIFData& cifData);


/*
-----------------INIT---------------------------
*/
void init(GLFWwindow* window) {
	cifData = CIFParser::parse("iron.cif");

	renderingProgram = createShaderProgram("points.vert", "points.frag");
	atomProgram = createShaderProgram("atoms.vert", "atoms.frag");
	wireframeProgram = createShaderProgram("wireframe.vert", "wireframe.frag");

	setupVertices();

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;

	pMat = glm::perspective(
		glm::radians(60.0f),
		aspect,
		0.1f,
		1000.0f
	);

	setupAtoms(cifData);
	setupWireframe(cifData);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 50.0f);


}


/*
-----------------DISPLAY---------------------------
*/
void display(GLFWwindow* window, double currentTime) {
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(renderingProgram);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	std::set<std::string> seenElements;
	for (auto& atom : cifData.atoms) {
		std::string element = AtomData::extractElement(atom.label);
		seenElements.insert(element);
	}
	
	if (!showReciprocal) {
		for (string e : seenElements) {
			ImGui::SetNextWindowSize(ImVec2(150, 0));
			int w, h;
			glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);
			ImGui::SetNextWindowPos(ImVec2(w - 150, 20), ImGuiCond_Always);
			ImGui::Begin("Key", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);

			glm::vec3 c = AtomData::getColor(e);
			ImVec4 color = ImVec4(c.x, c.y, c.z, 1.0f);
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 pos = ImGui::GetCursorScreenPos();
			drawList->AddCircleFilled(ImVec2(pos.x + 8, pos.y + 16), 10.0f, ImGui::ColorConvertFloat4ToU32(color));
			ImGui::Dummy(ImVec2(20, 16)); // reserves space so layout doesn't collapse
			ImGui::SameLine();
			ImGui::Text(e.c_str());

			ImGui::End();

		}
	}
		

	// Uniform locations
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	// ---- VIEW MATRIX ----
	glm::vec3 eye(
		camRadius * cos(camElevation) * sin(camAzimuth),
		camRadius * sin(camElevation),
		camRadius * cos(camElevation) * cos(camAzimuth)
	);

	vMat = glm::lookAt(eye, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	mvStack.push(vMat);

	// ---- XRD POINTS ----
	if (showReciprocal) {
		mvStack.push(mvStack.top());
		mvStack.top() *= glm::mat4(1.0f); // identity model

		glUniformMatrix4fv(mvLoc, 1, GL_FALSE,
			glm::value_ptr(mvStack.top()));

		glBindVertexArray(xrdVAO);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, xrdPointCount);

		mvStack.pop(); // pop XRD model
		mvStack.pop(); // pop view
	}
	else {
		//atom draw
		glUseProgram(atomProgram);

		GLuint atomMvLoc = glGetUniformLocation(atomProgram, "mv_matrix");
		GLuint atomProjLoc = glGetUniformLocation(atomProgram, "proj_matrix");
		
		glUniformMatrix4fv(atomMvLoc, 1, GL_FALSE, glm::value_ptr(vMat));
		glUniformMatrix4fv(atomProjLoc, 1, GL_FALSE, glm::value_ptr(pMat));
		

		glBindVertexArray(atomVAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, atomCount);

		if (showWireframe) {
			//wireframe
			glUseProgram(wireframeProgram);

			GLuint wfMvLoc = glGetUniformLocation(wireframeProgram, "mv_matrix");
			GLuint wfProjLoc = glGetUniformLocation(wireframeProgram, "proj_matrix");
			GLuint wfColorLoc = glGetUniformLocation(wireframeProgram, "lineColor");
			glUniform3f(wfColorLoc, 0.8f, 0.8f, 0.8f);
			glUniformMatrix4fv(wfMvLoc, 1, GL_FALSE, glm::value_ptr(vMat));
			glUniformMatrix4fv(wfProjLoc, 1, GL_FALSE, glm::value_ptr(pMat));


			glBindVertexArray(wireframeVAO);
			glDrawArrays(GL_LINES, 0, 24);
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/*
-----------------CALLBACKS---------------------------
*/

void window_reshape_callback(GLFWwindow* window, int newWidth, int newHeight) {
	aspect = (float)newWidth / (float)newHeight; // new width&height provided by the callback
	glViewport(0, 0, newWidth, newHeight); // sets screen region associated with framebuffer
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		showReciprocal = !showReciprocal;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		showWireframe = !showWireframe;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		const char* filterPatterns[] = { "*.cif" };
		const char* result = tinyfd_openFileDialog(
			"Open CIF File",
			"",
			1,
			filterPatterns,
			"CIF Files",
			0
		);
		if (result != nullptr) {
			glDeleteVertexArrays(1, &atomVAO);
			glDeleteBuffers(1, &atomVBO);
			glDeleteBuffers(1, &atomInstanceVBO);
			glDeleteVertexArrays(1, &xrdVAO);
			glDeleteBuffers(1, &xrdVBO);
			glDeleteVertexArrays(1, &wireframeVAO);
			glDeleteBuffers(1, &wireframeVBO);

			cifData = CIFParser::parse(result);
			setupAtoms(cifData);
			setupWireframe(cifData);
			setupVertices();
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camRadius -= (float)yoffset * 1.0f;
	if (camRadius < 1.0f) camRadius = 1.0f; // don't go inside the model
}


/*
-----------------MAIN---------------------------
*/
int main() {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	GLFWwindow* window = glfwCreateWindow(600, 600, "Window", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, window_reshape_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	init(window);
	


	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/*
-----------------IMPLEMENTATIONS---------------------------
*/

void setupVertices(void){
	
	//---------XRD POINTS----------------
	std::vector<XRDPoint> xrdPoints = XRDPoints::genFakeXRD(cifData.lattice.a);
	xrdPointCount = static_cast<int>(xrdPoints.size());

	glGenVertexArrays(1, &xrdVAO);
	glBindVertexArray(xrdVAO);

	glGenBuffers(1, &xrdVBO);
	glBindBuffer(GL_ARRAY_BUFFER, xrdVBO);
	glBufferData(GL_ARRAY_BUFFER,
		xrdPoints.size() * sizeof(XRDPoint),
		xrdPoints.data(),
		GL_STATIC_DRAW
	);

	//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
		sizeof(XRDPoint), (void*)0);
	glEnableVertexAttribArray(0);

	// intensity
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
		sizeof(XRDPoint),
		(void*)offsetof(XRDPoint, intensity));
	glEnableVertexAttribArray(1);

}

string readShaderSource(const char* file) {
	string content;
	ifstream fileStream(file, ios::in);
	string line = "";

	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

GLuint createShaderProgram(const char* vertFile, const char* fragFile) {
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	string vertShaderStr = readShaderSource(vertFile);
	string fragShaderStr = readShaderSource(fragFile);

	const char* vertShaderSrc = vertShaderStr.c_str();
	const char* fragShaderSrc = fragShaderStr.c_str();

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	glCompileShader(vShader);
	glCompileShader(fShader);

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);
	glLinkProgram(vfProgram);

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	return vfProgram;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		isDragging = true;
		glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		isDragging = false;
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!isDragging) return;

	double dx = xpos - lastMouseX;
	double dy = ypos - lastMouseY;

	camAzimuth += (float)dx * 0.01f;
	camElevation -= (float)dy * 0.01f;

	lastMouseX = xpos;
	lastMouseY = ypos;

	camElevation = glm::clamp(camElevation, -1.5f, 1.5f);
}

void setupAtoms(const CIFData& cifData) {
	// quad corners - two triangles
	float quadVertices[] = {
		-1.0f,  1.0f,
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f
	};

	// compute atom center positions
	std::vector<AtomInstance> instances;
	for (auto& atom : cifData.atoms) {
		bool hasZero = false;
		for (int i = 0; i < 3; i++) {
			if (atom.fractionalPos[i] < 0.001f) { hasZero = true; break; }
		}

		if (hasZero) {
			for (int dx = 0; dx <= 1; dx++)
				for (int dy = 0; dy <= 1; dy++)
					for (int dz = 0; dz <= 1; dz++) {
						glm::vec3 offset(dx * cifData.lattice.a,
							dy * cifData.lattice.b,
							dz * cifData.lattice.c);
						// only add if not duplicate
						glm::vec3 pos(
							atom.fractionalPos.x * cifData.lattice.a + offset.x,
							atom.fractionalPos.y * cifData.lattice.b + offset.y,
							atom.fractionalPos.z * cifData.lattice.c + offset.z
						);
						AtomInstance inst;
						inst.center = pos;
						std::string element = AtomData::extractElement(atom.label);
						inst.radius = AtomData::getRadius(element);
						inst.color = AtomData::getColor(element);
						inst.padding = 0.0f;
						instances.push_back(inst);

				

					}
		}
		else {
			AtomInstance inst;
			inst.center = glm::vec3(
				atom.fractionalPos.x * cifData.lattice.a,
				atom.fractionalPos.y * cifData.lattice.b,
				atom.fractionalPos.z * cifData.lattice.c
			);
			std::string element = AtomData::extractElement(atom.label);
			inst.radius = AtomData::getRadius(element);
			inst.color = AtomData::getColor(element);
			inst.padding = 0.0f;
			instances.push_back(inst);
		}
	}

	atomCount = static_cast<int>(instances.size());

	glGenVertexArrays(1, &atomVAO);
	glBindVertexArray(atomVAO);

	// quad corner offsets - location 1
	glGenBuffers(1, &atomVBO);
	glBindBuffer(GL_ARRAY_BUFFER, atomVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	// atom instances - location 0 and 2
	glGenBuffers(1, &atomInstanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, atomInstanceVBO);
	glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(AtomInstance), instances.data(), GL_STATIC_DRAW);

	// center - location 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AtomInstance), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 1);

	// radius - location 2
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(AtomInstance), (void*)offsetof(AtomInstance, radius));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);

	// color - location 3
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AtomInstance), (void*)offsetof(AtomInstance, color));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	
}

void setupWireframe(const CIFData& cifData) {
	float a = cifData.lattice.a;
	float b = cifData.lattice.b;
	float c = cifData.lattice.c;

	// 8 corners
	glm::vec3 corners[8] = {
		{0, 0, 0}, {a, 0, 0}, {0, b, 0}, {0, 0, c},
		{a, b, 0}, {a, 0, c}, {0, b, c}, {a, b, c}
	};

	// 12 edges as line pairs
	glm::vec3 lines[] = {
		corners[0], corners[1],  // bottom face
		corners[0], corners[2],
		corners[1], corners[4],
		corners[2], corners[4],
		corners[3], corners[5],  // top face
		corners[3], corners[6],
		corners[5], corners[7],
		corners[6], corners[7],
		corners[0], corners[3],  // vertical edges
		corners[1], corners[5],
		corners[2], corners[6],
		corners[4], corners[7]
	};

	glGenVertexArrays(1, &wireframeVAO);
	glBindVertexArray(wireframeVAO);

	glGenBuffers(1, &wireframeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, wireframeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);
}