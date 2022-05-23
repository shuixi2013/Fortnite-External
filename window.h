#pragma once

GLFWwindow* g_window;

static void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, XorStr("OverEW: %d: %s\n").c_str(), error, description);
}

void setupWindow()
{
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit()) {
		std::cout << XorStr("Over Init Failed!\n").c_str();
		return;
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (!monitor) {
		fprintf(stderr, XorStr("Failed to get monitor!\n").c_str());
		return;
	}

	g_width = glfwGetVideoMode(monitor)->width;
	g_height = glfwGetVideoMode(monitor)->height;

	glfwWindowHint(GLFW_FLOATING, true);
	glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_MAXIMIZED, false);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);

	g_window = glfwCreateWindow(g_width, g_height, XorStr("xxx????xxxx").c_str(), NULL, NULL);

	if (g_window == NULL) {
		std::cout << XorStr("Over: 0x2!\n").c_str();
		return;
	}

	glfwSetWindowAttrib(g_window, GLFW_DECORATED, false);
	glfwSetWindowAttrib(g_window, GLFW_MOUSE_PASSTHROUGH, true);

	glfwSetWindowMonitor(g_window, NULL, 0, 0, g_width, g_height + 1, 0);

	glfwMakeContextCurrent(g_window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, XorStr("Over: 0x1!\n").c_str());
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.IniFilename = NULL;
	io.Fonts->AddFontFromMemoryCompressedTTF(Test_compressed_data, Test_compressed_size, 13.f);

	menucolors();

	ImGui_ImplGlfw_InitForOpenGL(g_window, true);
	ImGui_ImplOpenGL3_Init(XorStr("#version 130").c_str());
}

void cleanupWindow()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_window);
	glfwTerminate();
}

BOOL CALLBACK retreiveFortniteWindow(HWND hwnd, LPARAM lparam)
{
	DWORD process_id;
	GetWindowThreadProcessId(hwnd, &process_id);
	if (process_id == g_pid) {
		fortnite_wnd = hwnd;
	}
	return TRUE;
}

bool isFortniteFocused = false;

void activateFortniteWindow()
{
	SetForegroundWindow(fortnite_wnd);
}

void handleKeyPresses()
{
	if (GetAsyncKeyState(VK_F1) & 1) {
		g_overlay_visible = !g_overlay_visible;
		glfwSetWindowAttrib(g_window, GLFW_MOUSE_PASSTHROUGH, !g_overlay_visible);
		if (g_overlay_visible) {
			HWND overlay_window = glfwGetWin32Window(g_window);
			SetForegroundWindow(overlay_window);
		}
		else {
			activateFortniteWindow();
		}
	}
}
