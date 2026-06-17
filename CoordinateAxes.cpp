#include "CoordinateAxes.h"

CoordinateAxes::CoordinateAxes(ShareData& data) :m_data(data) {
    coordVao = 0;
    coordVbo = 0;
}
CoordinateAxes::~CoordinateAxes() {
    if (coordVbo) {
        glDeleteBuffers(1, &coordVbo);
        coordVbo = 0;
    }
    if (coordVao) {
        glDeleteVertexArrays(1, &coordVao);
        coordVao = 0;
    }
}

// 生成一条“粗线”（小矩形）
void CoordinateAxes::GenerateAxisQuad(float thickness,
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    std::vector<float>& out)
{
    glm::vec3 A(x0, y0, z0);
    glm::vec3 B(x1, y1, z1);
    glm::vec3 dir = glm::normalize(B - A);
    glm::vec3 up(0, 1, 0);
    if (fabs(glm::dot(dir, up)) > 0.99f)up = glm::vec3(0, 0, 1); // 避免与dir平行
    glm::vec3 right = glm::normalize(glm::cross(dir, up));
    // if (glm::length(right) < 0.001f) right = glm::vec3(1, 0, 0);

    glm::vec3 offset = right * (thickness * 0.5f);

    glm::vec3 p0 = A - offset;
    glm::vec3 p1 = A + offset;
    glm::vec3 p2 = B + offset;
    glm::vec3 p3 = B - offset;

    out.insert(out.end(), { p0.x,p0.y,p0.z, p1.x,p1.y,p1.z, p2.x,p2.y,p2.z });
    out.insert(out.end(), { p0.x,p0.y,p0.z, p2.x,p2.y,p2.z, p3.x,p3.y,p3.z });
}

//生成坐标轴
void CoordinateAxes::GenerateCoordinateAxes() {
    coordVertices.clear();
    /* coordVertices = {
         // X 轴
         m_data.displayParams.xAxisMIN , 0.0f, 0.0f,m_data.displayParams.xAxisMAX, 0.0f, 0.0f,
         // Y 轴
         0.0f,m_data.displayParams.yAxisMIN, 0.0f,0.0f,  m_data.displayParams.yAxisMAX, 0.0f,
         // Z 轴
         0.0f, 0.0f,m_data.displayParams.zAxisMIN,0.0f, 0.0f,  m_data.displayParams.zAxisMAX
     };*/
    float thick = 0.1f; // 粗细

    // X 轴（红）
    GenerateAxisQuad(thick,
        m_data.displayParams.xAxisMIN, 0.0f, 0.0f,
        m_data.displayParams.xAxisMAX, 0.0f, 0.0f,
        coordVertices);
    // Y 轴（绿）
    GenerateAxisQuad(thick,
        0.0f, m_data.displayParams.yAxisMIN, 0.0f,
        0.0f, m_data.displayParams.yAxisMAX, 0.0f,
        coordVertices);
    // Z 轴（蓝）
    GenerateAxisQuad(thick,
        0.0f, 0.0f, m_data.displayParams.zAxisMIN,
        0.0f, 0.0f, m_data.displayParams.zAxisMAX,
        coordVertices);

    if (coordVao == 0)glGenVertexArrays(1, &coordVao);
    if (coordVbo == 0)glGenBuffers(1, &coordVbo);// 单个VBO存储所有坐标轴顶点
    glBindVertexArray(coordVao);
    glBindBuffer(GL_ARRAY_BUFFER, coordVbo);
    glBufferData(GL_ARRAY_BUFFER, coordVertices.size() * sizeof(float), coordVertices.data(), GL_STATIC_DRAW);
    // 位置属性配置
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
//绘制坐标轴
void CoordinateAxes::DrawCoordinateAxes(GLint colorLoc) const {
    //用矩形画坐标轴，让其变粗
    if (coordVao == 0)return;
    // glLineWidth(3.0f); //坐标轴线宽
    glBindVertexArray(coordVao);
    // 1. 绘制X轴 (红色)
    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // 2. 绘制Y轴 (绿色)
    glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 6, 6);
    // 3. 绘制Z轴 (蓝色)
    glUniform3f(colorLoc, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 12, 6);
    //glLineWidth(m_data.displayParams.lineWidth); //恢复默认线宽
    glBindVertexArray(0);
}