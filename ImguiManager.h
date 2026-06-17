#pragma once
#include "GlobalConfig.h"
#include"ShareData.h"
#include "EventSystem.h"
//#include "CoordinateAxes.h"
#include "MathExprParser.h"
#include "OpenglApp.h"
#include "FuncVertGenerator.h"

#include "imgui.h"
#include "imgui_stdlib.h"  // 包含包装器头文件,inputtext可以使用std::string类型接受数据
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <array>

/*##########################################################################################
* Imgui类，增删函数时，使用事件通知模式，其他数据变化直接引用ShareData
* 除了增删使用EventSystem触发外，其他地方直接使用ShareData共享数据，但保留了EventSystem触发接口，
以备未来扩展（EventType::None）（如增删函数时需要同步更新输入框数据等）
############################################################################################*/

struct InputBoxData
{     //输入框字符插入和选区数据
    std::string inputBuf;
    int cursorPos = 0;
    int selStart = 0;   // 选区起始位置（含）
    int selEnd = 0;     // 选区结束位置（不含）
    bool isFocused = false;//输入框是否获得焦点
};

// 根据操作系统自动获取系统中文字体路径（imgui中使用，仅供参考，实际路径可能因系统版本不同而异）
static std::string im_GetSystemChineseFontPath() {
#ifdef _WIN32
    // Windows 系统：优先微软雅黑，其次宋体
    //return "C:/Windows/Fonts/msyh.ttc";       // 微软雅黑（常用）
    return "C:/Windows/Fonts/simsun.ttc";  // 宋体
#elif __APPLE__
    // macOS 系统：苹方字体
    return "/System/Library/Fonts/PingFang.ttc";  // 苹方
#elif __linux__
    // Linux 系统：优先思源黑体（多数发行版预装）
    return "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";  // 思源黑体
#else
    // 其他系统：返回空（需手动处理）
    return "";
#endif
}

//========================= ImGui UI面板：负责函数输入、按钮、编辑 ==============================
class ImguiManager
{
public:
    ImguiManager(GLFWwindow* window, ShareData& data, OpenglApp& openglApp);
    ~ImguiManager();

    void StartFrame();
    void Render();
    void MathSymbolPanel(); //数学特殊字符面板
    void ControlPanel();//函数图像控制面板
    void SettingsPanel();//函数图像设置面板
    static int InputBoxCallback(ImGuiInputTextCallbackData* data);//输入框回调函数,必须是静态的static
    void RenderErrorPopup(const std::string& gerror);//错误弹窗
    void ShowFunctionPanel(MathExpressParser& imParser);
    //void CollectErrors();// 每帧收集所有错误(其他模块的错误接口)，并存储在allError中，供错误弹窗显示
private:
    void DropdownMenu();
    std::string inputExprStr = "sin(x)";
    int inputBufId = 0;//inputBoxData结构体的下标，用在数学面板里

    int imWindowWidth = 300;
    int imWindowHeight = 600;
    // 新增：回调上下文（绑定实例+输入框索引）数字面板用
    struct InputCallbackContext {
        ImguiManager* instance;  // ImguiManager实例指针
        int boxIndex;            // 当前输入框的索引（对应inputBoxDataInstances的下标）
    };
    std::array<InputBoxData, 9> inputBoxDataInstances = { {
    {"",0,0,0,1},{"",0,0,0,1},{"",0,0,0,1},{"",0,0,0,1}, { "-10",0,0,0,1 },
     {"10",0,0,0,1},{"-10",0,0,0,1},{"10",0,0,0,1},{"",0,0,0,1}} }; //预定义9个输入框上下文
    std::array<const char*, 15> mathSymbols = {
       "x",  "y",  "θ",  "t", "φ", // 第一行
        "()",  "+",  "-",  "*",  "/",  // 第二行
       "sin()",  "cos()", "fac()",  "^","π" };// 第三行

    ShareData& m_data;

    OpenglApp& m_openglApp;
    //CoordinateAxes& m_coordinateAxes;
    std::vector<std::string>allError{};//存储所有错误
};

