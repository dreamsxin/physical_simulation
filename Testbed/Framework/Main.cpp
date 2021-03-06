/*
* Copyright (c) 2006-2016 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glew/glew.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include "DebugDraw.h"
#include "Test.h"
#include "../Tests/BulletTest.h"

#include "glfw/glfw3.h"
#include <stdio.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

// This include was added to support MinGW
#ifdef _WIN32
#include <crtdbg.h>
#endif
int doGUI = true;
//
struct UIState
{
	bool showMenu;
};

//
namespace
{
GLFWwindow *mainWindow = NULL;
UIState ui;

int32 testIndex = 0;
int32 testSelection = 0;
int32 testCount = 0;
TestEntry *entry;
Test *test;
Settings settings;
bool rightMouseDown;
b2Vec2 lastp;
}

std::vector<float> positions;

//
static void sCreateUI(GLFWwindow *window)
{
	ui.showMenu = true;

	// Init UI
	const char *fontPath = "Data/DroidSans.ttf";
	ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath, 15.f);

	if (ImGui_ImplGlfwGL3_Init(window, false) == false)
	{
		fprintf(stderr, "Could not init GUI renderer.\n");
		assert(false);
		return;
	}

	ImGuiStyle &style = ImGui::GetStyle();
	style.FrameRounding = style.GrabRounding = style.ScrollbarRounding = 2.0f;
	style.FramePadding = ImVec2(4, 2);
	style.DisplayWindowPadding = ImVec2(0, 0);
	style.DisplaySafeAreaPadding = ImVec2(0, 0);
}

//
static void sResizeWindow(GLFWwindow *, int width, int height)
{
	g_camera.m_width = width;
	g_camera.m_height = height;
}

//
static void sKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
	bool keys_for_ui = ImGui::GetIO().WantCaptureKeyboard;
	if (keys_for_ui)
		return;

	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			// Quit
			glfwSetWindowShouldClose(mainWindow, GL_TRUE);
			break;

		case GLFW_KEY_LEFT:
			// Pan left
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(2.0f, 0.0f);
				test->ShiftOrigin(newOrigin);
			}
			else
			{
				g_camera.m_center.x -= 0.5f;
			}
			break;

		case GLFW_KEY_RIGHT:
			// Pan right
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(-2.0f, 0.0f);
				test->ShiftOrigin(newOrigin);
			}
			else
			{
				g_camera.m_center.x += 0.5f;
			}
			break;

		case GLFW_KEY_DOWN:
			// Pan down
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(0.0f, 2.0f);
				test->ShiftOrigin(newOrigin);
			}
			else
			{
				g_camera.m_center.y -= 0.5f;
			}
			break;

		case GLFW_KEY_UP:
			// Pan up
			if (mods == GLFW_MOD_CONTROL)
			{
				b2Vec2 newOrigin(0.0f, -2.0f);
				test->ShiftOrigin(newOrigin);
			}
			else
			{
				g_camera.m_center.y += 0.5f;
			}
			break;

		case GLFW_KEY_HOME:
			// Reset view
			g_camera.m_zoom = 1.0f;
			g_camera.m_center.Set(0.0f, 20.0f);
			break;

		case GLFW_KEY_Z:
			// Zoom out
			g_camera.m_zoom = b2Min(1.1f * g_camera.m_zoom, 20.0f);
			break;

		case GLFW_KEY_X:
			// Zoom in
			g_camera.m_zoom = b2Max(0.9f * g_camera.m_zoom, 0.02f);
			break;

		case GLFW_KEY_R:
			// Reset test
			delete test;
			test = entry->createFcn();
			if (auto bt = dynamic_cast<BulletTest *>((Test *)test))
			{
				bt->Setup(&settings);
			}
			break;

		case GLFW_KEY_SPACE:
			// Launch a bomb.
			if (test)
			{
				test->LaunchBomb();
			}
			break;

		case GLFW_KEY_O:
			settings.singleStep = true;
			break;

		case GLFW_KEY_P:
			settings.pause = !settings.pause;
			break;

		case GLFW_KEY_LEFT_BRACKET:
			// Switch to previous test
			--testSelection;
			if (testSelection < 0)
			{
				testSelection = testCount - 1;
			}
			break;

		case GLFW_KEY_RIGHT_BRACKET:
			// Switch to next test
			++testSelection;
			if (testSelection == testCount)
			{
				testSelection = 0;
			}
			break;

		case GLFW_KEY_TAB:
			ui.showMenu = !ui.showMenu;

		default:
			if (test)
			{
				test->Keyboard(key);
			}
		}
	}
	else if (action == GLFW_RELEASE)
	{
		test->KeyboardUp(key);
	}
	// else GLFW_REPEAT
}

//
static void sCharCallback(GLFWwindow *window, unsigned int c)
{
	ImGui_ImplGlfwGL3_CharCallback(window, c);
}

//
static void sMouseButton(GLFWwindow *window, int32 button, int32 action, int32 mods)
{
	ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);

	double xd, yd;
	glfwGetCursorPos(mainWindow, &xd, &yd);
	b2Vec2 ps((float32)xd, (float32)yd);

	// Use the mouse to move things around.
	if (button == GLFW_MOUSE_BUTTON_1)
	{
		//<##>
		//ps.Set(0, 0);
		b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
		if (action == GLFW_PRESS)
		{
			if (mods == GLFW_MOD_SHIFT)
			{
				test->ShiftMouseDown(pw);
			}
			else
			{
				test->MouseDown(pw);
			}
		}

		if (action == GLFW_RELEASE)
		{
			test->MouseUp(pw);
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_2)
	{
		if (action == GLFW_PRESS)
		{
			lastp = g_camera.ConvertScreenToWorld(ps);
			rightMouseDown = true;
		}

		if (action == GLFW_RELEASE)
		{
			rightMouseDown = false;
		}
	}
}

//
static void sMouseMotion(GLFWwindow *, double xd, double yd)
{
	b2Vec2 ps((float)xd, (float)yd);

	b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
	test->MouseMove(pw);

	if (rightMouseDown)
	{
		b2Vec2 diff = pw - lastp;
		g_camera.m_center.x -= diff.x;
		g_camera.m_center.y -= diff.y;
		lastp = g_camera.ConvertScreenToWorld(ps);
	}
}

//
static void sScrollCallback(GLFWwindow *window, double dx, double dy)
{
	ImGui_ImplGlfwGL3_ScrollCallback(window, dx, dy);
	bool mouse_for_ui = ImGui::GetIO().WantCaptureMouse;

	if (!mouse_for_ui)
	{
		if (dy > 0)
		{
			g_camera.m_zoom /= 1.1f;
		}
		else
		{
			g_camera.m_zoom *= 1.1f;
		}
	}
}

//
static void sRestart()
{
	delete test;
	entry = g_testEntries + testIndex;
	test = entry->createFcn();
	if (auto bt = dynamic_cast<BulletTest *>((Test *)test))
	{
		bt->Setup(&settings);
	}
}

//
static bool sTestEntriesGetName(void *, int idx, const char **out_name)
{
	*out_name = g_testEntries[idx].name;
	return true;
}

//
static void sInterface()
{
	int menuWidth = 200;
	if (ui.showMenu)
	{
		ImGui::SetNextWindowPos(ImVec2((float)g_camera.m_width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)g_camera.m_height - 20));
		ImGui::Begin("Testbed Controls", &ui.showMenu, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false); // Disable TAB

		ImGui::PushItemWidth(-1.0f);

		ImGui::Text("Test");
		if (ImGui::Combo("##Test", &testIndex, sTestEntriesGetName, NULL, testCount, testCount))
		{
			delete test;
			entry = g_testEntries + testIndex;
			test = entry->createFcn();
			testSelection = testIndex;
		}
		ImGui::Separator();

		ImGui::Text("Vel Iters");
		ImGui::SliderInt("##Vel Iters", &settings.velocityIterations, 0, 50);
		ImGui::Text("Pos Iters");
		ImGui::SliderInt("##Pos Iters", &settings.positionIterations, 0, 50);
		ImGui::Text("Hertz");
		ImGui::SliderFloat("##Hertz", &settings.hz, 5.0f, 120.0f, "%.0f hz");
		ImGui::PopItemWidth();

		ImGui::Checkbox("Sleep", &settings.enableSleep);
		ImGui::Checkbox("Warm Starting", &settings.enableWarmStarting);
		ImGui::Checkbox("Time of Impact", &settings.enableContinuous);
		ImGui::Checkbox("Sub-Stepping", &settings.enableSubStepping);

		ImGui::Separator();

		ImGui::Checkbox("Shapes", &settings.drawShapes);
		ImGui::Checkbox("Joints", &settings.drawJoints);
		ImGui::Checkbox("AABBs", &settings.drawAABBs);
		ImGui::Checkbox("Contact Points", &settings.drawContactPoints);
		ImGui::Checkbox("Contact Normals", &settings.drawContactNormals);
		ImGui::Checkbox("Contact Impulses", &settings.drawContactImpulse);
		ImGui::Checkbox("Friction Impulses", &settings.drawFrictionImpulse);
		ImGui::Checkbox("Center of Masses", &settings.drawCOMs);
		ImGui::Checkbox("Statistics", &settings.drawStats);
		ImGui::Checkbox("Profile", &settings.drawProfile);

		ImVec2 button_sz = ImVec2(-1, 0);
		if (ImGui::Button("Pause (P)", button_sz))
			settings.pause = !settings.pause;

		if (ImGui::Button("Single Step (O)", button_sz))
			settings.singleStep = !settings.singleStep;

		if (ImGui::Button("Restart (R)", button_sz))
			sRestart();

		if (ImGui::Button("Quit", button_sz))
			glfwSetWindowShouldClose(mainWindow, GL_TRUE);

		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}

	//ImGui::ShowTestWindow(NULL);
}

//
void glfwErrorCallback(int error, const char *description)
{
	fprintf(stderr, "GLFW error occured. Code: %d. Description: %s\n", error, description);
}

#define SQR(x) ((x) * (x))
//
extern "C" float *my_func(int argc, char **argv)
{
	settings.bodies.clear();
	settings.rotations.clear();
	settings.sizes.clear();

	doGUI = argc > 1 ? std::atoi(argv[1]) : 1;
	settings.gravity = argc > 2 ? std::atof(argv[2]) : -100;
	settings.friction = argc > 3 ? std::atof(argv[3]) : 0.2;
	settings.rest = argc > 4 ? std::atof(argv[4]) : 0.75;

	int skip = 5;
	for (int i = skip; i < argc; i++)
	{
		auto idx = (i - skip) % 6;
		if (idx == 0)
		{
			settings.bodies.push_back({0.0, 0.0});
			settings.rotations.push_back({0.0});
			settings.sizes.push_back({1.0, 0.1});
			settings.gravity_on.push_back(false);
			settings.bodies[settings.bodies.size() - 1].x = std::atof(argv[i]);
		}
		else if (idx == 1)
		{
			settings.bodies[settings.bodies.size() - 1].y = std::atof(argv[i]);
		}
		else if (idx == 2)
		{
			settings.rotations[settings.bodies.size() - 1] = std::atof(argv[i]);
		}
		else if (idx == 3)
		{
			settings.sizes[settings.bodies.size() - 1].x = std::atof(argv[i]);
		}
		else if (idx == 4)
		{
			settings.sizes[settings.bodies.size() - 1].y = std::atof(argv[i]);
		}
		else if (idx == 5)
		{
			settings.gravity_on[settings.bodies.size() - 1] = std::atoi(argv[i]);
		}
	}

	//doGUI = std::atoi(argv[1]);
	settings.doGUI = doGUI;
#if defined(_WIN32)
	// Enable memory-leak reports
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif

	if (doGUI)
	{
		glfwSetErrorCallback(glfwErrorCallback);

		g_camera.m_width = 1024;
		g_camera.m_height = 640;

		if (glfwInit() == 0)
		{
			fprintf(stderr, "Failed to initialize GLFW\n");
			return NULL;
		}

		char title[64];
		sprintf(title, "Box2D Testbed Version %d.%d.%d", b2_version.major, b2_version.minor, b2_version.revision);
#if defined(__APPLE__)
		// Without these settings on macOS, OpenGL 2.1 will be used by default which will cause crashes at boot.
		// This code is a slightly modified version of the code found here: http://www.glfw.org/faq.html#how-do-i-create-an-opengl-30-context
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		mainWindow = glfwCreateWindow(g_camera.m_width, g_camera.m_height, title, NULL, NULL);
		if (mainWindow == NULL)
		{
			fprintf(stderr, "Failed to open GLFW mainWindow.\n");
			glfwTerminate();
			return NULL;
		}
#endif
		glfwMakeContextCurrent(mainWindow);
		//printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

		glfwSetScrollCallback(mainWindow, sScrollCallback);
		glfwSetWindowSizeCallback(mainWindow, sResizeWindow);
		glfwSetKeyCallback(mainWindow, sKeyCallback);
		glfwSetCharCallback(mainWindow, sCharCallback);
		glfwSetMouseButtonCallback(mainWindow, sMouseButton);
		glfwSetCursorPosCallback(mainWindow, sMouseMotion);
		glfwSetScrollCallback(mainWindow, sScrollCallback);

#if defined(__APPLE__) == FALSE
		//glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
			exit(EXIT_FAILURE);
		}
#endif

		g_debugDraw.Create();

		sCreateUI(mainWindow);
	}

	testCount = 0;
	while (g_testEntries[testCount].createFcn != NULL)
	{
		++testCount;
	}

	testIndex = b2Clamp(testIndex, 0, testCount - 1);
	testSelection = testIndex;

	entry = g_testEntries + testIndex;
	test = entry->createFcn();

	if (auto bt = dynamic_cast<BulletTest *>((Test *)test))
	{
		bt->Setup(&settings);
	}

	double time1;

	// Control the frame rate. One draw per monitor refresh.
	if (doGUI)
	{
		glfwSwapInterval(1);
		glClearColor(0.3f, 0.3f, 0.3f, 1.f);
		time1 = glfwGetTime();
	}

	double frameTime = 0.0;

	auto run_loop = doGUI ? !glfwWindowShouldClose(mainWindow) : true;
	auto run_sim = true;

	auto xMin = -40.0f;
	auto xMax = 40.0f;
	auto yMin = 0.26f;
	auto yMax = 50.0f;
	auto vMin = 0.3f;

	positions.clear();
	positions.push_back(0.0);

	for (int i = 0; run_loop && (i < 750 || doGUI); i++)
	{
		if (doGUI)
		{
			glfwGetWindowSize(mainWindow, &g_camera.m_width, &g_camera.m_height);

			int bufferWidth, bufferHeight;
			glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
			glViewport(0, 0, bufferWidth, bufferHeight);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ImGui_ImplGlfwGL3_NewFrame();
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2((float)g_camera.m_width, (float)g_camera.m_height));
			ImGui::Begin("Overlay", NULL, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
			ImGui::SetCursorPos(ImVec2(5, (float)g_camera.m_height - 20));
			ImGui::Text("%.1f ms", 1000.0 * frameTime);
			ImGui::End();

			glEnable(GL_DEPTH_TEST);
		}

		test->Step(&settings);

		positions.push_back(settings.p1.x);
		positions.push_back(settings.p1.y);

		if (doGUI)
		{
			if (testSelection != testIndex)
			{
				testIndex = testSelection;
				delete test;
				entry = g_testEntries + testIndex;
				test = entry->createFcn();
				g_camera.m_zoom = 1.0f;
				g_camera.m_center.Set(0.0f, 20.0f);
			}

			test->DrawTitle(entry->name);
			glDisable(GL_DEPTH_TEST);

			sInterface();

			// Measure speed
			double time2 = glfwGetTime();
			double alpha = 0.9f;
			frameTime = alpha * frameTime + (1.0 - alpha) * (time2 - time1);
			time1 = time2;

			ImGui::Render();
			glfwSwapBuffers(mainWindow);
			glfwPollEvents();
		}

		auto totalV = SQR(settings.v1.x) + SQR(settings.v1.y);

		auto world_bounds = settings.p1.x > xMin && settings.p1.x < xMax && settings.p1.y > yMin && settings.p1.y < yMax;

		run_sim = world_bounds ? true : false;
		run_loop = doGUI ? !glfwWindowShouldClose(mainWindow) : run_sim;
	}
	//printf("%f %f\n", settings.p1.x, settings.p1.y);
	if (test)
	{
		delete test;
		test = NULL;
	}

	if (doGUI)
	{
		g_debugDraw.Destroy();
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();
	}

	positions[0] = positions.size() - 1;
	return positions.data();
}

int main(int argc, char **argv)
{
	float *rets = my_func(argc, argv);
	auto size = positions.size();
	printf("%f %f\n", positions[size - 2], positions[size - 1]);
	return 0;
}
