#pragma once
#include "GlobalConfig.h"
#include "EventSystem.h"


//#include <recursive_mutex>

// 函数配置
struct FunctionConfig
{
    std::string expression;
    FunctionType type = FunctionType::Cartesian2D;
    bool isVisible = true;
    bool isDirty = true;
    glm::vec3  graphColor{ 1.0f, 0.0f, 0.0f };
};

// UI 参数
struct UIParams
{
    bool inputBoxIsFocused = false;//输入框是否获得焦点
    FunctionType selectedExprType = FunctionType::Cartesian2D;
    int modifyFuncId = -1;//记录正在修改的函数ID
};

// 视图参数
struct ViewParams
{
    float translateX = 0.0f, translateY = 0.0f;//沿 XY轴平移距离
    float rotateX = 0.0f, rotateY = 0.0f, rotateZ = 0.0f; // 绕XYZ轴旋转角度
    float scaleVal = 1.0f;// 缩放比例
    float rotateSpeedX = 0.0f, rotateSpeedY = 0.0f, rotateSpeedZ = 0.0f;// 绕XYZ连续旋转速度
    glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 15.0f);// 相机位置
    glm::vec3 objectPos = glm::vec3(0.0f, 0.0f, 0.0f);// 物体位置
    glm::mat4 rightCoordMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));//初始化右手坐标系转换矩阵
    int useRightHanded = 0;//是否使用右手坐标系，0-否 1-是
};

// 图形参数
struct FunctionGraphParams
{
    int sampleCount = 100;
    int singularityThreshold = 10;
    float xminR = -10.0f, yminR = -10.0f, xmaxR = 10.0f, ymaxR = 10.0f;
};

// 显示样式参数
struct DisplayParams
{
    glm::vec4 backgroundColor = glm::vec4(0.985f, 0.985f, 0.985f, 1.0f);// 背景颜色

    int lineType = 0;// 线条样式：0-实线，1-虚线，2-点线
    float lineWidth = 1.0f;
    float xAxisMIN = -10.0f, xAxisMAX = 10.0f;
    float yAxisMIN = -10.0f, yAxisMAX = 10.0f;
    float zAxisMIN = -10.0f, zAxisMAX = 10.0f;
    bool showGrid = true;// 背景网格显示状态
};
// 共享数据（唯一数据中心）
class ShareData
{
public:
    ShareData() = default;
    ~ShareData() = default;

    EventDispatcher eventBus;
    ViewParams      viewParams;
    UIParams	uiParams;
    FunctionGraphParams graphParams;
    DisplayParams   displayParams;

    // ========== 错误接口（每个类都一样） ==========
    std::string GetError() {
        std::lock_guard<std::recursive_mutex> lock(m_errorMutex);
        std::string e = std::move(m_lastError);
        m_lastError.clear();
        return e;
    }

    bool HasError() const {
        std::lock_guard<std::recursive_mutex> lock(m_errorMutex);
        return !m_lastError.empty();
    }

    // ========== 函数列表操作（线程安全） ==========
    size_t GetFunctionCount() const {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return m_functions.size();
    }

    FunctionConfig* GetFunction(size_t index) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (index >= m_functions.size()) return nullptr;
        return &m_functions[index];
    }

    //==============增加函数，同时通知OpenGL资源同步增删列表===============
    int AddFunction(FunctionConfig cfg) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_functions.push_back(std::move(cfg));
        // m_functions.push_back(func);
        int idx = static_cast<int>(m_functions.size()) - 1;
        eventBus.Dispatch({ EventType::FunctionAdded, idx });
        return idx;
    }

    //==============删除函数，同时通知OpenGL资源同步增删列表===============
    void RemoveFunction(int index) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (index < 0 || index >= (int)m_functions.size()) return;
        m_functions.erase(m_functions.begin() + index);
        eventBus.Dispatch({ EventType::FunctionRemoved, index });
    }

    //=================修改表达式=====================
    void UpdateExpression(int index, std::string expr, FunctionType type) {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (index >= m_functions.size()) return;
        m_functions[index].expression = std::move(expr);
        m_functions[index].isVisible = true;
        m_functions[index].isDirty = true;
        m_functions[index].type = type;
        eventBus.Dispatch({ EventType::FunctionModified, index });
    }

private:
    void SetError(const std::string& msg) {
        std::lock_guard<std::recursive_mutex> lock(m_errorMutex);
        m_lastError = msg;
    }

    std::vector<FunctionConfig> m_functions;
    // 递归锁（解决死锁）
    mutable std::recursive_mutex m_mutex;
    std::string m_lastError;
    mutable std::recursive_mutex m_errorMutex;
};

