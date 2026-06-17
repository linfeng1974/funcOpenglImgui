#pragma once
#include "GlobalConfig.h"
#include "ShareData.h"

// 固定世界坐标系网格（不旋转、可缩放）
class GridRenderer
{
public:
    GridRenderer(ShareData& data) :m_data(data) {
        // glGenVertexArrays(1, &gridVao);
        // glGenBuffers(1, &gridVbo);
    }

    ~GridRenderer() {
        if (gridVbo) {
            glDeleteBuffers(1, &gridVbo);
            gridVbo = 0;
        }
        if (gridVao) {
            glDeleteVertexArrays(1, &gridVao);
            gridVao = 0;
        }
    }

    // 更新网格（根据当前显示范围 + 缩放）
    void UpdateGrid(float xMin, float xMax, float yMin, float yMax, float scale) {
        gridVertices.clear();

        // 网格间距随缩放自适应（不旋转）
        float step = 1.0f * scale;
        step = std::max(0.1f, step);
        xMin = std::floor(xMin / step) * step;//保证网格线从整数倍开始,保证经过原点
        // X 轴竖线
        for (float x = xMin; x <= xMax; x += step) {
            gridVertices.insert(gridVertices.end(), { x, yMin, 0, x, yMax, 0 });
        }

        // Y 轴横线
        yMin = std::floor(yMin / step) * step;//保证网格线从整数倍开始,保证经过原点
        for (float y = yMin; y <= yMax; y += step) {
            gridVertices.insert(gridVertices.end(), { xMin, y, 0, xMax, y, 0 });
        }
        if (gridVao == 0)glGenVertexArrays(1, &gridVao);
        if (gridVbo == 0)glGenBuffers(1, &gridVbo);
        // 上传 GPU
        glBindVertexArray(gridVao);
        glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // 绘制网格（固定世界矩阵，不旋转）
    void DrawGrid(GLint colorLoc, GLint mvLoc, GLint projLoc, float scale) {

        if (gridVertices.empty() || gridVao == 0) return;
        if (mvLoc == -1 || projLoc == -1)return;
        // ✅ 关键：网格使用纯世界矩阵，不旋转
       // glm::mat4 gridMV = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        //glm::mat4 gridMV = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));
        glm::mat4 gridV = glm::translate(glm::mat4(1.0f), glm::vec3(-m_data.viewParams.cameraPos.x, -m_data.viewParams.cameraPos.y, -m_data.viewParams.cameraPos.z));
        glm::mat4 gridM = glm::translate(glm::mat4(1.0f), glm::vec3(m_data.viewParams.objectPos.x, m_data.viewParams.objectPos.y, m_data.viewParams.objectPos.z));
        gridM = glm::scale(gridM, glm::vec3(m_data.viewParams.scaleVal, m_data.viewParams.scaleVal, 1.0f)); // 缩放
        glm::mat4 gridMV = gridV * gridM;
        /*  // ✅ 关键：网格必须用正交投影，才能在透视下可见
           float left = -scale * 10;
           float right = scale * 10;
           float bottom = -scale * 10;
           float top = scale * 10;
           glm::mat4 gridProj = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);*/
           // glm::mat4 gridMV = glm::mat4(1.0f);
        glBindVertexArray(gridVao);
        glUniform3f(colorLoc, 0.8f, 0.8f, 0.8f);

        if (mvLoc != -1)glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(gridMV));
        //if(projLoc!=-1)glUniformMatrix4fv(projLoc, 1, GL_FALSE, &gridProj[0][0]);
        glDrawArrays(GL_LINES, 0, (GLsizei)(gridVertices.size() / 3));
        glBindVertexArray(0);
    }

private:
    GLuint gridVao = 0, gridVbo = 0;
    std::vector<float> gridVertices;
    ShareData& m_data;
};

