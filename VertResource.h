#pragma once
#include "GlobalConfig.h"

/*##################################################################
* 管理VAO/VBO资源，生成顶点数据并上传到GPU
* 顶点数据（Vertice)与资源(Resource)VAO/VBO封装在一起，表达式修改时更新顶点数据并重传
####################################################################*/

//顶点资源
class VertResource
{
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    std::vector<float>vertices;
    std::vector<GLuint>indices;

    //创建VAO/VBO
    VertResource() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

    }
    //自动释放GPU资源
    ~VertResource() {
        if (vbo != 0)glDeleteBuffers(1, &vbo);
        if (ebo)glDeleteBuffers(1, &ebo);
        if (vao != 0)glDeleteVertexArrays(1, &vao);
    }
    //禁止拷贝，防止重复释放
    VertResource(const VertResource&) = delete;
    VertResource& operator=(const VertResource&) = delete;
    //允许移动（vector必须,vector扩容时，会析构与构造）
    VertResource(VertResource&& other)noexcept
        :vao(other.vao), vbo(other.vbo), ebo(other.ebo), vertices(std::move(other.vertices)), indices(std::move(other.indices)) {
        other.vao = other.vbo = other.ebo = 0;
    }
    VertResource& operator=(VertResource&& other)noexcept {
        if (this != &other) {
            // 释放当前资源
            if (vbo)glDeleteBuffers(1, &vbo);
            if (ebo)glDeleteBuffers(1, &ebo);
            if (vao)glDeleteVertexArrays(1, &vao);
            // 转移资源所有权
            vao = other.vao;
            vbo = other.vbo;
            ebo = other.ebo;
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            // 重置源对象
            other.vao = other.vbo = other.ebo = 0;
        }
        return *this;
    }
};




