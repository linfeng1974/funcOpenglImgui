#pragma once
#include "GlobalConfig.h"
#include "Shader.h"

/*###############################################################################
*  Opengl应用程序基类：初始化Opengl、GLFW/GLEW，上下文，相机，移动旋转，渲染循环
#################################################################################*/

//opengl类，生成窗口、制图
class OpenglApp {
public:
    OpenglApp();
    virtual ~OpenglApp(); //虚析构：确保子类正确析构
    //初始化Opengl状态
    void InitOpengl(const std::string& uniformColor, const std::string& uniformMv, const std::string& uniformProj);
    bool InitGlfwGlew(); //初始化GLFW窗口与GLEW
    void display(double currentTime);
    GLFWwindow* GetWindow() { return window; }//获取窗口指针

    //回调函数
    void OnWindowResize(int width, int height);//窗口大小改变回调
    //static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    //static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
   // static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    //外部接口
    void SetBackgroundColor(glm::vec3 color);
    void SetCameraPos(glm::vec3 cPos);
    void SetObjectTransform(glm::vec3 pos, float scale, float rX, float rY, float rZ, float tX, float tY);
    void SetRotateSpeed(float speedX, float speedY, float speedZ);
    GLuint GetRenderProgram() const { return renderingProgram; }
    GLuint GetColorLoc() const { return colorLoc; }
    GLuint GetMvLoc() const { return mvLoc; }
    GLuint GetProjLoc() const { return projLoc; }
    //glm::mat4 GetPMat() const { return pMat; }
    //glm::mat4 GetMvMat() const { return mvMat; }
    virtual void OnDraw() = 0;//每一帧绘制，虚函数，子类重写（未程序扩展留下接口）
    std::string GetError();
    bool HasError() const;

protected:
    void SetError(const std::string& msg);
    std::string openglError{};   //错误信息
    //void DrawCoordinateAxes() const;
    GLuint renderingProgram = 0;  //着色器程序ID

    GLint colorLoc = -1, mvLoc = -1, projLoc = -1;  //Uniform位置
    float aspect = 1.0f;            //宽高比
    glm::mat4 pMat{ 1.0f }, vMat{ 1.0f }, mMat{ 1.0f }, mvMat{ 1.0f };  //投影矩阵、视图矩阵、模型矩阵、模型视图矩阵
    std::vector<float> coordVertices; //坐标轴顶点
    GLFWwindow* window = nullptr;

    glm::mat4 rightCoordMat = glm::mat4(1.0f);//右手坐标系转换矩阵
    glm::vec3 backgroundColor = { 0.985f, 0.985f, 0.985f };//背景颜色
    glm::vec3 cameraPos = { 0.0f, 2.0f, 15.0f };//相机位置
    glm::vec3 objectPos = glm::vec3(0.0f, 0.0f, 0.0f);// 物体位置
    float translateX = 0.0f, translateY = 0.0f;//沿 XY轴平移距离
    float scaleVal = 1.0f;// 缩放比例
    float rotateX = 0.0f, rotateY = 0.0f, rotateZ = 0.0f; // 绕XYZ轴旋转角度
    float rotateSpeedX = 0.0f, rotateSpeedY = 0.0f, rotateSpeedZ = 0.0f;// 绕XYZ连续旋转速度
    int useRightHanded = 0;//是否使用右手坐标系，0-否 1-

    mutable std::mutex m_errMutex;
};

