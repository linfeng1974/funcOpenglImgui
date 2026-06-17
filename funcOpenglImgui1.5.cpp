// funcOpenglImgui1.5.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// funcOpenglImgui1.4.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

#include "OpenglApp.h"
#include "GLRenderer.h"
#include "ImguiManager.h"
#include "GridRenderer.h"
#include "CoordinateAxes.h"

/*########################################################################################
* 主程序
##########################################################################################*/



//=====================主应用类：继承OpenglApp,实现OnDraw=========================
//OpenglApp数据使用的是protected模式
class FuncPlotApp :public OpenglApp {
public:
	FuncPlotApp(ShareData& data, GLRenderer& glrenderer) :m_data(data), m_glRenderer(glrenderer) {}
	//初始化所有模块
	bool Init() {
		if (!InitGlfwGlew())return false;
		InitOpengl("color", "mv_matrix", "proj_matrix");//"color", "mv_matrix", "proj_matrix"是着色器里对应的变量

		//m_imguiManager = std::make_unique<ImguiManager>(GetWindow(), m_data, *this);//创建ImguiManager
		//m_parser = std::make_unique<MathExpressParser>();//创建数学解析器

		return true;
	}
	//重写绘制：先画曲线，再画UI
	void OnDraw() override {

		m_glRenderer.DrawAll(colorLoc);
		//m_imguiManager->StartFrame();
		//m_imguiManager->ShowFunctionPanel(*m_parser);
		//m_imguiManager->Render();
	}
	//主循环
	/*void MainLoop() {
		while (!glfwWindowShouldClose(GetWindow())) {
			glfwPollEvents();                   // 处理事件（如键盘输入、鼠标移动等）
			OnDraw();
			glfwSwapBuffers(GetWindow());   // 交换前后缓冲（双缓冲机制，避免画面闪烁）
		}
	  }*/




private:
	ShareData& m_data;
	GLRenderer& m_glRenderer;
	//CoordinateAxes m_coord;
	//std::unique_ptr<ImguiManager>m_imguiManager;
	//std::unique_ptr<MathExpressParser>m_parser;
};

//设置回调
struct CallbackPtr {
	ShareData* data;
	FuncPlotApp* plot;
};

//========================鼠标交互全局变量=============================

static inline bool isMouseDragging = false;    // 鼠标是否处于拖拽状态
static inline bool isMouseScaling = false;     // 鼠标是否处于缩放状态
static inline double lastMouseX = 0.0;         // 上一帧鼠标X坐标
static inline double lastMouseY = 0.0;         // 上一帧鼠标Y坐标
static inline float dragSensitivity = 0.01f;   // 拖拽灵敏度
static inline float scaleSensitivity = 0.01f;  // 缩放灵敏度

//=====================鼠标交互回调函数：缩放和平移（旋转）========================
	// 鼠标按下回调：记录初始位置和状态
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	// 先让ImGui处理事件,防止鼠标被GLFW窗口鼠标回调函数拦截
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	// 关键判断：若ImGui窗口需要捕获鼠标，直接返回，不执行OpenGL的缩放逻辑
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			// 按下左键：开启拖拽，记录当前鼠标位置
			isMouseDragging = true;
			glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
		}
		else if (action == GLFW_RELEASE) {
			// 释放左键：关闭拖拽
			isMouseDragging = false;
		}
	}
}

// 鼠标移动回调：处理拖拽（旋转图形）
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {

	// 先让ImGui处理事件,防止鼠标被GLFW窗口鼠标回调函数拦截
	ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
	// 关键判断：若ImGui窗口需要捕获鼠标，直接返回，不执行OpenGL的缩放逻辑
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}
	auto* g_data = static_cast<CallbackPtr*>(glfwGetWindowUserPointer(window));
	if (!g_data) return;

	// 获取CTRL键按下状态
	int ctrlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) | glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL);
	if (isMouseDragging) {
		// 计算鼠标移动增量
		float deltaX = (float)(xpos - lastMouseX) * dragSensitivity;
		float deltaY = (float)(ypos - lastMouseY) * dragSensitivity;
		if (ctrlPressed) {
			// -------------------------- CTRL+左键平移 --------------------------
			// 平移方向与鼠标移动一致，放大增量提升手感
			g_data->data->viewParams.translateX += deltaX * 20.0f;
			g_data->data->viewParams.translateY -= deltaY * 20.0f; // 负号：让Y轴平移与视觉一致
		}
		else {
			// 拖拽控制模型绕Y轴（水平移动）和X轴（垂直移动）旋转
			g_data->data->viewParams.rotateY += deltaX * 50.0f;  // 放大增量，提升拖拽手感
			g_data->data->viewParams.rotateX += deltaY * 50.0f;  // 负号：让垂直拖拽方向与视觉一致

			// 限制X轴旋转范围（避免过度翻转）
			g_data->data->viewParams.rotateX = glm::clamp(g_data->data->viewParams.rotateX, -90.0f, 90.0f);
		}
		// 更新上一帧鼠标位置
		lastMouseX = xpos;
		lastMouseY = ypos;
	}
}

// 鼠标滚轮回调：处理缩放（放大/缩小图形）
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	// 先让ImGui处理事件,防止鼠标被GLFW窗口鼠标回调函数拦截
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	// 关键判断：若ImGui窗口需要捕获鼠标，直接返回，不执行OpenGL的缩放逻辑
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}

	auto g_data = static_cast<CallbackPtr*>(glfwGetWindowUserPointer(window));
	if (!g_data) return;
	// 滚轮Y方向增量：上滚放大，下滚缩小
	g_data->data->viewParams.scaleVal = glm::clamp(g_data->data->viewParams.scaleVal + (float)yoffset * scaleSensitivity * 5, 0.1f, 5.0f);
}




//程序入口
int main() {
	ShareData myShareData;
	GLRenderer myGLRenderer(myShareData);
	FuncPlotApp myApp(myShareData, myGLRenderer);
	CoordinateAxes myCoord(myShareData);
	GridRenderer myGridRenderer(myShareData);

	if (!myApp.Init()) {
		std::cerr << "" << std::endl;
		return -1;
	}
	GLFWwindow* window = myApp.GetWindow();

	std::unique_ptr<ImguiManager>myImguiManager = std::make_unique<ImguiManager>(window, myShareData, myApp);//创建ImguiManager
	std::unique_ptr<MathExpressParser>myParser = std::make_unique<MathExpressParser>();//创建数学解析器

	//回调设置
	CallbackPtr callbackPtr{ &myShareData,&myApp };
	glfwSetWindowUserPointer(window, &callbackPtr);
	glfwSetMouseButtonCallback(window, mouse_button_callback);    // 鼠标按下/释放
	glfwSetCursorPosCallback(window, cursor_position_callback);   // 鼠标移动
	glfwSetScrollCallback(window, scroll_callback);               // 鼠标滚轮
	// glfwSetWindowSizeCallback(window, window_reshape_callback);
   //glfwSetWindowUserPointer(window, &myApp);
	glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int width, int height) {
		auto* app = static_cast<CallbackPtr*>(glfwGetWindowUserPointer(win));
		if (app) {
			app->plot->OnWindowResize(width, height);
		}
		});

	//glfwSetWindowUserPointer(window, &myApp);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();                   // 处理事件（如键盘输入、鼠标移动等）
		myImguiManager->StartFrame();
		myImguiManager->ShowFunctionPanel(*myParser);
		myApp.SetBackgroundColor(myShareData.displayParams.backgroundColor);
		myApp.SetCameraPos(myShareData.viewParams.cameraPos);
		myApp.SetObjectTransform(myShareData.viewParams.objectPos, myShareData.viewParams.scaleVal, myShareData.viewParams.rotateX,
			myShareData.viewParams.rotateY, myShareData.viewParams.rotateZ, myShareData.viewParams.translateX, myShareData.viewParams.translateY);
		myApp.SetRotateSpeed(myShareData.viewParams.rotateSpeedX, myShareData.viewParams.rotateSpeedY, myShareData.viewParams.rotateSpeedZ);
		myGridRenderer.UpdateGrid(-10.0f, 10.0f, -10.0f, 10.0f, myShareData.viewParams.scaleVal);
		myCoord.GenerateCoordinateAxes();
		//以下三个成员函数顺序不能变

		myApp.display(glfwGetTime());
		myCoord.DrawCoordinateAxes(myApp.GetColorLoc());
		if (myShareData.displayParams.showGrid)myGridRenderer.DrawGrid(myApp.GetColorLoc(), myApp.GetMvLoc(), myApp.GetProjLoc(), myShareData.viewParams.scaleVal);

		myImguiManager->Render();
		//im_show(window);
		glfwSwapBuffers(window);                                          // 交换前后缓冲（双缓冲机制，避免画面闪烁）
	}
	return 0;
}


