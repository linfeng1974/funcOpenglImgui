#pragma once
#include "ShareData.h"
#include <GL/glew.h>  
#include<vector>

class CoordinateAxes {
public:
	CoordinateAxes(ShareData& data);
	~CoordinateAxes();
	void GenerateCoordinateAxes();
	void DrawCoordinateAxes(GLint colorLoc) const;
	void GenerateAxisQuad(float thickness,
		float x0, float y0, float z0,
		float x1, float y1, float z1,
		std::vector<float>& out);
private:
	GLuint coordVao = 0, coordVbo = 0;//坐标轴使用
	std::vector<float>coordVertices;
	ShareData& m_data;
};


