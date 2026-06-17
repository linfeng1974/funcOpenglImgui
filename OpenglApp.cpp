
#include "OpenglApp.h"

OpenglApp::OpenglApp() = default;

OpenglApp::~OpenglApp() {
    //m_notifier.setCallback(nullptr);//清空数据回调，防止悬空指针
    if (renderingProgram) {
        glDeleteProgram(renderingProgram);
        renderingProgram = 0;
    }

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}
//=======================设置函数==============================
//设置背景颜色
void OpenglApp::SetBackgroundColor(glm::vec3 color) {
    backgroundColor = color;
}
//设置相机位置
void OpenglApp::SetCameraPos(glm::vec3 cPos) {
    cameraPos = cPos;
}
//设置物体变换
void OpenglApp::SetObjectTransform(glm::vec3 pos, float scale, float rX, float rY, float rZ, float tX, float tY) {
    objectPos = pos;
    scaleVal = scale;
    rotateX = rX, rotateY = rY, rotateZ = rZ;
    translateX = tX, translateY = tY;
}
//设置旋转速度
void OpenglApp::SetRotateSpeed(float speedX, float speedY, float speedZ) {
    rotateSpeedX = speedX;
    rotateSpeedY = speedY;
    rotateSpeedZ = speedZ;
}

//==================初始化GLFW和窗口==========================
bool OpenglApp::InitGlfwGlew() {

    if (!glfwInit()) {                                                    // 初始化GLFW库（必须在使用其他GLFW函数前调用）
        //m_data.errorMessages.push_back("初始化GLFW错误");
        SetError("初始化GLFW错误");
        return false;                                                        // 初始化失败，退出程序
    }
    // 设置窗口创建参数（指定OpenGL版本和配置）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                       // 主版本号3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                       // 次版本号3（即OpenGL 3.3）
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);       // 使用核心模式
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);               // 向前兼容（MacOS必需）

    // 创建窗口对象（宽800，高600，标题"函数图形"）
    // 获取主显示器工作区并处理 DPI
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    int workX = 0, workY = 0, workW = 0, workH = 0, fbW = 0, fbH = 0;
    if (primary) {
        glfwGetMonitorWorkarea(primary, &workX, &workY, &workW, &workH);
        float scaleX = 1.0f, scaleY = 1.0f;
        glfwGetMonitorContentScale(primary, &scaleX, &scaleY); // 用于高 DPI 调整（可选）
        // 若需要帧缓冲像素尺寸：
        fbW = int(workW * scaleX + 0.5f) - 150;
        fbH = int(workH * scaleY + 0.5f) - 100;
    }
    window = glfwCreateWindow(fbW, fbH, "函数图形可视化", nullptr, nullptr);
    if (!window) {                                                      // 窗口创建失败
        // m_data.errorMessages.push_back("创建GLFW 窗口错误，可能原因：\n1.显卡驱动不支持OpenGL 3.3及以上版本\n2.使用远程桌面连接（RDP）等虚拟显示器环境");
        SetError("创建GLFW 窗口错误，可能原因：\n1.显卡驱动不支持OpenGL 3.3及以上版本\n2.使用远程桌面连接（RDP）等虚拟显示器环境");
        glfwTerminate();                                                // 清理GLFW资源
        return false;                                                      // 退出程序
    }

    //回调设置

  /*  glfwSetWindowUserPointer(window, this); // 将当前对象指针存储在窗口用户指针中，供回调函数使用
    //窗口大小回调
    glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int width, int height) {
        auto* app = static_cast<OpenglApp*>(glfwGetWindowUserPointer(win));
        if (app) {
            app->OnWindowResize(width, height);
        }
        });*/
    //glfwSetMouseButtonCallback(window, mouse_button_callback);    // 鼠标按下/释放
   // glfwSetCursorPosCallback(window, cursor_position_callback);   // 鼠标移动
//   glfwSetScrollCallback(window, scroll_callback);               // 鼠标滚轮
    // glfwSetWindowSizeCallback(window, window_reshape_callback);
    // 关键：把 ShareData 传给 GLFW 而不是 this！
   // glfwSetWindowUserPointer(window, &myShareData);
    
    glfwMakeContextCurrent(window);                                       // 将窗口的OpenGL上下文设置为当前线程的主上下文
    glewExperimental = GL_TRUE;                                         // 启用实验性特性（核心模式必需）

    if (glewInit() != GLEW_OK) {                                         // 初始化失败
        //m_data.errorMessages.push_back("初始化GLEW错误，可能原因：\n1.显卡驱动不支持OpenGL 3.3及以上版本\n2.使用远程桌面连接（RDP）等虚拟显示器环境");
        SetError("初始化GLEW错误，可能原因：\n1.显卡驱动不支持OpenGL 3.3及以上版本\n2.使用远程桌面连接（RDP）等虚拟显示器环境");
        //glfwTerminate();                                                 // 清理资源
        return false;                                                       // 退出程序
    }
    // while(glGetError()!=GL_NO_ERROR){}                              //清除GLEW初始化时可能产生的无效枚举错误
    glfwSwapInterval(1);  //垂直同步
    // 注册回调函数（关键：鼠标+窗口大小）
    //glfwSetMouseButtonCallback(window, mouse_button_callback);    // 鼠标按下/释放
   // glfwSetCursorPosCallback(window, cursor_position_callback);   // 鼠标移动
   // glfwSetScrollCallback(window, scroll_callback);               // 鼠标滚轮
   // glfwSetWindowSizeCallback(window, window_reshape_callback);

    return true;
}

//============窗口大小改变：更新视口与投影矩阵========================
void OpenglApp::OnWindowResize(int width, int height) {
    //右侧留Imgui空间，所以视口宽度=总宽-1/5宽
    aspect = (float)(width - width / 5) / (float)height;
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);//透视投影：FOV=60度（1.0472rad），近裁剪面0.1，远裁剪面1000
    glViewport(0, 0, width - width / 5, height);
}

//=====================初始化Opengl状态：着色器、深度测试、面剔除等========================
void OpenglApp::InitOpengl(const std::string& uniformColor, const std::string& uniformMv, const std::string& uniformProj) {

    // 在此处创建着色器程序（确保已在 InitGlfwGlew 中创建并切换了上下文）

    if (renderingProgram == 0) {
        renderingProgram = Utils::createShaderProgram();
        if (renderingProgram == 0) {
            SetError("创建着色器程序错误，可能原因：\n1.显卡驱动不支持OpenGL 3.3及以上版本\n2.使用远程桌面连接（RDP）等虚拟显示器环境");
            return;
        }
    }
    //获取Uniform变量位置
    colorLoc = glGetUniformLocation(renderingProgram, uniformColor.c_str());
    mvLoc = glGetUniformLocation(renderingProgram, uniformMv.c_str());
    projLoc = glGetUniformLocation(renderingProgram, uniformProj.c_str());
    glEnable(GL_DEPTH_TEST);//启用深度测试
    glDepthFunc(GL_LEQUAL);//设置深度测试函数为GL_LEQUAL，允许通过深度值小于或等于当前深度缓冲值的片段
    glEnable(GL_CULL_FACE);//启用面剔除
    //glFrontFace(GL_CCW); //坐标系同步设置面剔除朝向，默认右手系
    if (useRightHanded == 0) glFrontFace(GL_CCW); //坐标系同步设置面剔除朝向，右手系
    if (useRightHanded == 1)glFrontFace(GL_CW);   //坐标系同步设置面剔除朝向,左手系
    //GenerateCoordinateAxes();//生成坐标轴顶点并传输到GPU
    //初始化视口和右手坐标系
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    //glfwGetWindowSize(window, &width, &height);
    OnWindowResize(width, height);
    rightCoordMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));//初始化右手坐标系

    // if (Utils::checkOpenGLError("init"))m_data.errorMessages.push_back(Utils::g_glerrorStr);
}

//=====================每一帧绘制：清屏、设置变换、调用OnDraw（子类重写）========================
void OpenglApp::display(double currentTime) {
    //清屏
    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(renderingProgram); //使用着色器程序
    //if (Utils::checkOpenGLError("glUseProgram"))m_data.errorMessages.push_back(Utils::g_glerrorStr);
    vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraPos.x, -cameraPos.y, -cameraPos.z));
    mMat = glm::translate(glm::mat4(1.0f), glm::vec3(objectPos.x + translateX, objectPos.y + translateY, objectPos.z));
    mMat = glm::scale(mMat, glm::vec3(scaleVal, scaleVal, scaleVal)); // 缩放
    // 静态旋转（先绕X，再绕Y，最后绕Z）
    mMat = glm::rotate(mMat, glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
    mMat = glm::rotate(mMat, glm::radians(rotateY), glm::vec3(0.0f, 1.0f, 0.0f));
    mMat = glm::rotate(mMat, glm::radians(rotateZ), glm::vec3(0.0f, 0.0f, 1.0f));
    // mMat = glm::rotate(mMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
     // 连续旋转（基于时间和速度，与静态旋转叠加）
    mMat = glm::rotate(mMat, (float)currentTime * rotateSpeedX, glm::vec3(1.0f, 0.0f, 0.0f));
    mMat = glm::rotate(mMat, (float)currentTime * rotateSpeedY, glm::vec3(0.0f, 1.0f, 0.0f));
    mMat = glm::rotate(mMat, (float)currentTime * rotateSpeedZ, glm::vec3(0.0f, 0.0f, 1.0f));
    // mMat = glm::rotate(mMat, (float)currentTime * 0.3f, glm::vec3(0.0f, 0.0f, 1.0f));
   // mMat = glm::rotate(mMat, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mvMat = vMat * rightCoordMat * mMat;
    //上传矩阵到Shader
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    //DrawCoordinateAxes();
    OnDraw();
    //glBindVertexArray(vao[0]);

    //if (Utils::checkOpenGLError("coordinate"))_sdPtr->errorMessages.push_back(Utils::g_glerrorStr);
    // -------------------------- 设置线条样式（先应用到坐标轴） --------------------------

}
// ========== 错误接口 ==========
std::string OpenglApp::GetError() {
    std::lock_guard<std::mutex> lock(m_errMutex);
    std::string e = std::move(openglError);
    openglError.clear();
    return e;
}

bool OpenglApp::HasError() const {
    std::lock_guard<std::mutex> lock(m_errMutex);
    return !openglError.empty();
}
void OpenglApp::SetError(const std::string& msg) {
    std::lock_guard<std::mutex> lock(m_errMutex);
    openglError = msg;
}

