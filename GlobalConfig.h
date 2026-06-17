#pragma once
// 启用GLM实验功能，必须在GLM头文件前定义
#define GLM_ENABLE_EXPERIMENTAL

// C++标准库头文件
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <mutex>
#include <future>
#include <atomic>

// 第三方库头文件
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#define GLFW_INCLUDE_NONE   //
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//================ 函数类型枚举=================
enum class FunctionType : uint8_t
{
    Cartesian2D = 0,  // 平面曲线 y=f(x)
    Polar = 1,            // 极坐标 r=f(θ)
    Parametric3D = 2,     // 参数方程 x(t),y(t),z(t)
    Cartesian3D = 3,      // 三维曲面 z=f(x,y)
    Spherical = 4         // 球坐标
};

// 默认采样点
constexpr int DEFAULT_SAMPLES = 100;

