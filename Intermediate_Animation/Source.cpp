#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <nfd/nfd.h>
#include "Shader.h"
#include "Model.h"
#include "Animation.h"
#include "Animator.h"
#include "Camera.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int screenWidth = 800;
int screenHeight = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

static Camera camera;
static float lastX = screenWidth / 2.0f; //Starts the x and y position of mouse at middle of screen
static float lastY = screenHeight / 2.0f;
static bool firstMouse = true;
static float deltaTime = 0.0f; //Time between current and last frame
static float lastFrame = 0.0f;
static bool cameraSpedUp = false;

//IMGUI Related Variables
static bool enable_Mouse_Cursor = false; //Specifies whether mouse cursor should be usalbe (for GUI)
static bool correct_input_mode_set = true;
static bool first_mouse_callback = false; //Used to either save original mouse coords when entering cursor mode. Or loading saved mouse coords when camera mode.
static float original_xpos_cursor_mode = 0.0f;
static float original_ypos_cursor_mode = 0.0f;
static float scaleFactor = 1.0f;
static bool animation_paused = false;
static std::vector<std::string> animationNames;

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Skeleton Animation", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//IMGUI Setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	glEnable(GL_DEPTH_TEST);

	int current_Animation = 0;
	int new_Animation = 0;

	Shader shader(".\\shaders\\general.vert", ".\\shaders\\general.frag");
	Model theModel(".\\model\\cube.glb");
	Animation theAnimation(".\\model\\cube.glb", &theModel, 0); //Can do this as animation is also stored inthe dae file
	animationNames = theAnimation.animationNames;
	Animator animator(&theAnimation);

	nfdchar_t* model_path = NULL;

	shader.use();
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		glfwPollEvents();

		//IMGUI
		if (!correct_input_mode_set) {
			if (enable_Mouse_Cursor) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			correct_input_mode_set = true;
			first_mouse_callback = true;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::ShowDemoWindow();
		ImGui::Begin("Animation Menu");
		if (ImGui::Button("Load Model")) {
			nfdresult_t result = NFD_OpenDialog("glb", NULL, &model_path);
			if (result == NFD_OKAY) {
				theModel = Model(model_path);
				theAnimation = Animation(model_path, &theModel, 0);
				animationNames = theAnimation.animationNames;
				current_Animation = 0;
				new_Animation = 0;
				animator = Animator(&theAnimation);
				scaleFactor = 1.0f; //Reset Scale for new model
				animation_paused = false;
			}
		}
		ImGui::SetItemTooltip("Load a 3D Model and it's Animation by Selecting it's File");
		if (ImGui::InputFloat("Scale", &scaleFactor, 0.1f, 1.0f, "%.3f")) {
			if (scaleFactor < 0.1f) { //Clamp
				scaleFactor = 0.1f;
			}
		}
		if (!animation_paused && ImGui::Button("Pause Animation")) {
			animation_paused = true;
		}
		if (animation_paused && ImGui::Button("UnPause Animation")) {
			animation_paused = false;
		}
		ImGui::Text("Animations");
		for (int i = 0; i < animationNames.size(); i++) {
			ImGui::RadioButton(animationNames[i].c_str(), &new_Animation, i);
		}
		ImGui::End();

		if (current_Animation != new_Animation) {
			current_Animation = new_Animation;
			if (model_path == NULL) //If didn't load any other model, reuse defalt path
				theAnimation = Animation(".\\model\\cube.glb", &theModel, current_Animation);
			
			else
				theAnimation = Animation(model_path, &theModel, current_Animation);
			animationNames = theAnimation.animationNames;
			animator = Animator(&theAnimation);
			if (animation_paused) {
				animator.updateAnimation(deltaTime); //If Animation is paused, we call updateAnimation once in order to update to the new animation without unpausing.
			}
		}

		if (!animation_paused)
			animator.updateAnimation(deltaTime);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 500.0f);
		glm::mat4 view = camera.getViewMatrix();
		shader.setMatrix4Float("projection", projection);
		shader.setMatrix4Float("view", view);
		std::vector<glm::mat4> transforms = animator.getFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); i++)
			shader.setMatrix4Float("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(scaleFactor));
		shader.setMatrix4Float("model", model);
		theModel.Draw(shader);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	ImGuiIO& io = ImGui::GetIO();
	if (xpos < 0 || xpos > screenWidth || ypos < 0 || ypos > screenHeight)
		io.AddMousePosEvent(-FLT_MAX, -FLT_MAX); //Tells Imgui that Mouse is offscreen to prevet bugging
	else
		io.AddMousePosEvent(xpos, ypos);
	io.WantCaptureMouse = enable_Mouse_Cursor;

	if (!io.WantCaptureMouse) {
		if (first_mouse_callback) {
			xpos = original_xpos_cursor_mode;
			ypos = original_ypos_cursor_mode;
			io.AddMousePosEvent(xpos, ypos);
			glfwSetCursorPos(window, xpos, ypos);
			first_mouse_callback = false;
		}
		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; //Reversed since y-coords go from bot to top

		lastX = xpos;
		lastY = ypos;

		camera.processMouseMovement(xoffset, yoffset);
	}
	else {
		if (first_mouse_callback) {
			original_xpos_cursor_mode = lastX;
			original_ypos_cursor_mode = lastY;
			first_mouse_callback = false;
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		enable_Mouse_Cursor = !enable_Mouse_Cursor;
		correct_input_mode_set = false;
	}
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		if (!cameraSpedUp) {
			camera.movementSpeed = 20.0f; //8x speed
			cameraSpedUp = true;
		}
		else {
			camera.movementSpeed = 2.5f;
			cameraSpedUp = false;
		}
	}
}