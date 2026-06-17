#pragma once
#include "GlobalConfig.h"
#include "ShareData.h"
#include "FuncVertGenerator.h"
#include "VertResource.h"

/*##########################################################################################
* 渲染器类：负责监听函数列表变化事件，生成顶点数据，上传GPU，并提供绘制接口
* 从这个类开始，函数配置和OpenGL资源完全解耦，函数配置只负责数学表达式和参数，渲染器负责生成顶点数据和GPU资源
* 也就是数学属性开始与资源属性联系在一起了，函数配置改变会触发事件，渲染器监听到事件后重建对应的顶点数据和GPU资源
############################################################################################*/

//渲染器
class GLRenderer
{
public:
    GLRenderer(ShareData& data) : m_data(data) {
        // 订阅事件,监听函数添加、修改、删除等事件，及时更新顶点数据和OpenGL资源
        m_data.eventBus.Register([this](const Event& e) {
            OnEvent(e); });
    }

    // ========== 错误接口 ==========
    std::string GetError() {
        std::lock_guard<std::mutex> lock(m_errMutex);
        std::string e = std::move(m_lastError);
        m_lastError.clear();
        return e;
    }

    bool HasError() const {
        std::lock_guard<std::mutex> lock(m_errMutex);
        return !m_lastError.empty();
    }

    //=================绘制所有曲线========================
    void DrawAll(GLint colorLoc) {

        //glClear(GL_COLOR_BUFFER_BIT);
        //逐个绘制，不重建、不全局刷新
        for (size_t i = 0; i < m_data.GetFunctionCount(); ++i)
        {
            const auto funcPtr = m_data.GetFunction(i);  //C++ 允许 const 左值引用 延长临时变量的生命周期，和变量本身活得一样久。

            if (funcPtr->isVisible) {
                glUniform3f(colorLoc, funcPtr->graphColor.x, funcPtr->graphColor.y, funcPtr->graphColor.z);
                DrawCurve(i);
            }
        }
    }

private:
    //=======事件处理：实现函数列表与资源列表的严格同步============
    void OnEvent(const Event& e) {
        switch (e.type) {
        case EventType::FunctionAdded:
            //函数新增一个，资源也新增一个，永远保持下标i一一对应
                //m_vertResource.emplace_back();
            m_vertResourcePtr.push_back(std::make_unique<VertResource>());
            //立即生成顶点并上传,新建的函数与新建的资源联系对应起来
            if (e.funcIndex >= 0)RebuildCurve(e.funcIndex);
            break;
        case EventType::FunctionModified:
            if (e.funcIndex >= 0)RebuildCurve(e.funcIndex);
            break;
        case EventType::FunctionRemoved:
            size_t index = e.funcIndex;
            if (index < m_vertResourcePtr.size())m_vertResourcePtr.erase(m_vertResourcePtr.begin() + index);
            break;
            //default:
              //  break;
        }
    }

    //========重建单条曲线：数学计算->生成顶点->上传GPU============
    //========新建的函数与新建的资源联系对应起来===================
    void RebuildCurve(int index) {
        if (index >= m_data.GetFunctionCount())return;
        if (index >= m_vertResourcePtr.size())return;
        const auto* cfgPtr = m_data.GetFunction(index);
        if (!cfgPtr) {
            SetError("GLRenderer.h中RebulidCurve:获取函数配置错误！");
            return;
        }
        auto& res = *m_vertResourcePtr[index];
        //创建数学生成器
        auto gen = FuncVertGeneratorFactory::CreateGenerator(cfgPtr->type, cfgPtr->expression);
        if (!gen) {
            SetError("不支持的函数类型");
            return;
        }

        const auto& gp = m_data.graphParams;
        gen->SetDomain(gp.xminR, gp.xmaxR, gp.yminR, gp.ymaxR, gp.sampleCount);

        //生成顶点
        res.vertices.clear();
        if (!gen->GenerateVertices(res.vertices)) {
            SetError("顶点生成失败");
            return;
        }

        //生成索引（处理奇点断点）
        res.indices.clear();
        if (cfgPtr->type == FunctionType::Cartesian2D || cfgPtr->type == FunctionType::Polar || cfgPtr->type == FunctionType::Parametric3D)
            GenerateCurveIndices(res, m_data.graphParams.singularityThreshold);
        else if (cfgPtr->type == FunctionType::Cartesian3D || cfgPtr->type == FunctionType::Spherical)
            GenerateSurfaceIndices(res, m_data.graphParams.singularityThreshold);
        //上传VAO/VBO/EBO
        UploadGpu(res);
    }


    //===================计算相对应的索引====================
    //生成曲线索引
    //奇点、无效点、无穷大判断，比如1 / X在X = 0或sqrt(-1)处断开
    void GenerateCurveIndices(VertResource& res, int  singularity_threshold) {
        for (int i = 0; i < res.vertices.size() / 3 - 1; i++) {
            glm::vec3 p1(res.vertices[i * 3], res.vertices[i * 3 + 1], res.vertices[i * 3 + 2]);
            glm::vec3 p2(res.vertices[(i + 1) * 3], res.vertices[(i + 1) * 3 + 1], res.vertices[(i + 1) * 3 + 2]);
            if (std::isfinite(p1.x) && std::isfinite(p1.y) && std::isfinite(p1.z)) {
                if (std::isfinite(p2.x) && std::isfinite(p2.y) && std::isfinite(p2.z)) {
                    if (glm::distance(p1, p2) < (float)singularity_threshold) {
                        res.indices.push_back((GLuint)i);
                        res.indices.push_back((GLuint)(i + 1));
                    }
                }
            }
        }
    }

    //生成曲面索引
    // 奇点、无效点、无穷大判断，比如1 / X在X = 0或sqrt(-1)处断开
    //先生成顶点对应的二维索引数组，再根据二维索引数组生成线段索引，避免奇点断点连接

    void GenerateSurfaceIndices(VertResource& res, int  singularity_threshold) {
        // res.vertices.clear();
       // int cols = points + 1;  // 每行的顶点数（x方向点数）

        int rows = m_data.graphParams.sampleCount;
        int cols = m_data.graphParams.sampleCount;
        std::vector<std::vector<int>> interimIdx(rows + 1, std::vector<int>(cols + 1, -1));

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {

                interimIdx[i][j] = i * cols + j;
            }
        }
        // 生成x方向的线段（同一行内，相邻点连接）
        for (int i = 0; i < rows; i++) {  // 遍历每一行
            for (int j = 0; j < cols; j++) {  // 遍历行内每个点（除最后一个）
                int currentIdx = interimIdx[i][j];       // 当前点索引
                int rightIdx = interimIdx[i][static_cast<std::vector<int, std::allocator<int>>::size_type>(j) + 1];           // 右侧相邻点索引
                if (currentIdx != -1 && rightIdx != -1) {
                    glm::vec3 p1(res.vertices[currentIdx * 3], res.vertices[currentIdx * 3 + 1], res.vertices[currentIdx * 3 + 2]);
                    glm::vec3 p2(res.vertices[rightIdx * 3], res.vertices[rightIdx * 3 + 1], res.vertices[rightIdx * 3 + 2]);
                    //奇点、无效点、无穷大判断，比如1/X在X=0或sqrt(-1)处断开
                    if (std::isfinite(p1.x) && std::isfinite(p1.y) && std::isfinite(p1.z)) {
                        if (std::isfinite(p2.x) && std::isfinite(p2.y) && std::isfinite(p2.z)) {
                            if (glm::distance(p1, p2) < singularity_threshold) {
                                res.indices.push_back((GLuint)currentIdx);
                                res.indices.push_back((GLuint)rightIdx);
                            }
                        }
                    }
                }
            }
        }

        // 生成y方向的线段（同一列内，相邻行连接，可选）
        for (int j = 0; j < cols; j++) {  // 遍历每一列
            for (int i = 0; i < rows; i++) {  // 遍历列内每个点（除最后一个）
                int currentIdx = interimIdx[i][j];         // 当前点索引
                int downIdx = interimIdx[i + 1][j];        // 下方相邻点索引
                if (currentIdx != -1 && downIdx != -1) {
                    glm::vec3 p1(res.vertices[currentIdx * 3], res.vertices[currentIdx * 3 + 1], res.vertices[currentIdx * 3 + 2]);
                    glm::vec3 p2(res.vertices[downIdx * 3], res.vertices[downIdx * 3 + 1], res.vertices[downIdx * 3 + 2]);
                    if (std::isfinite(p1.x) && std::isfinite(p1.y) && std::isfinite(p1.z)) {
                        if (std::isfinite(p2.x) && std::isfinite(p2.y) && std::isfinite(p2.z)) {
                            if (glm::distance(p1, p2) < singularity_threshold) {
                                res.indices.push_back((GLuint)currentIdx);
                                res.indices.push_back((GLuint)downIdx);
                            }
                        }
                    }
                }
            }
        }
    }


    //============上传顶点数据到VBO（匹配你的3D顶点着色器）==============
    void UploadGpu(VertResource& res)
    {
        if (res.vertices.empty())return;

        // 绑定VAO
        glBindVertexArray(res.vao);
        // 绑定VBO并上传数据
        glBindBuffer(GL_ARRAY_BUFFER, res.vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            res.vertices.size() * sizeof(float), res.vertices.data(),
            GL_DYNAMIC_DRAW  // 动态更新（表达式修改时重传）
        );
        // 配置顶点属性（匹配你的着色器：location=0, vec2）
        glVertexAttribPointer(
            0,                  // location=0
            3,                  // vec3（x,y）
            GL_FLOAT,           // 数据类型
            GL_FALSE,           // 不归一化
            3 * sizeof(float),  // 步长
            nullptr             // 偏移量
        );
        // 启用顶点属性
        glEnableVertexAttribArray(0);
        //有索引->创建并上传EBO
        if (!res.indices.empty()) {
            if (!res.ebo)glGenBuffers(1, &res.ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res.ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, res.indices.size() * sizeof(GLuint), res.indices.data(), GL_DYNAMIC_DRAW);
        }
        else { //无索引：删除EBO
            if (res.ebo) {
                glDeleteBuffers(1, &res.ebo);
                res.ebo = 0;
            }
        }
        // 解绑VAO/VBO（防止污染）
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    //===================绘制单条曲线（调用glDrawArrays或glDrawElements）======================
    void DrawCurve(size_t index) {
        if (index >= m_vertResourcePtr.size())return;
        const auto& res = *m_vertResourcePtr[index];
        if (res.vertices.empty())return;
        glBindVertexArray(res.vao);
        if (!res.indices.empty()) {
            glDrawElements(GL_LINES, (GLsizei)res.indices.size(), GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)(res.vertices.size() / 3));
        }
        glBindVertexArray(0);
    }
    //============设置错误信息==========================
    void SetError(const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_errMutex);
        m_lastError = msg;
    }

    ShareData& m_data;
    std::vector<std::unique_ptr<VertResource>> m_vertResourcePtr;//运行时对象,vector类型如果是类（有资源对象），标准做法是用智能指针
    std::string m_lastError;
    mutable std::mutex m_errMutex;
};

