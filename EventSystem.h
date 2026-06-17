#pragma once
#include "GlobalConfig.h"

/*######################################################################
* 观察者模式实现的事件系统，主要用于函数的增删改等事件通知，保持函数列表和OpenGL资源列表的同步
########################################################################*/

// 事件类型
enum class EventType : uint8_t
{
    None = 0,
    FunctionAdded,// 函数添加
    FunctionRemoved,// 函数删除
    FunctionModified,// 函数表达式/颜色修改
    FunctionVisibilityChanged,
    AllFunctionsCleared,
    SettingsChanged
};

// 事件结构体,增删函数需要同步增删VAO/VBO/EBO
struct Event
{
    EventType type = EventType::None;
    int funcIndex = -1;// 关联函数索引，-1表示全局事件
};

// ==================事件总线（同步派发）================
class EventDispatcher
{
public:
    using Callback = std::function<void(const Event&)>;

    // ============注册事件回调================
    void Register(Callback cb) {
        m_callbacks.push_back(std::move(cb));
    }

    // ===========派发事件，通知所有订阅者============
    void Dispatch(const Event& e) const {
        for (const auto& cb : m_callbacks) {
            if (cb)cb(e);
        }
    }

private:
    std::vector<Callback> m_callbacks;
};

