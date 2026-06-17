#pragma once
#include "GlobalConfig.h"
#include "MathExprParser.h"

/*################################################################################
* 工厂模式实现顶点生成器接口和具体生成器类，供GLRenderer调用
* 包括2D笛卡尔生成器和极坐标生成器，未来可扩展参数方程、三维曲面等生成器
* 以及表达式语法检查接口，供UI预检查输入表达式，提升用户体验
##################################################################################*/

// ============顶点生成器接口==============
class IFuncVertGenerator
{
public:
    virtual ~IFuncVertGenerator() = default;
    virtual void SetExpression(const std::string& expr) = 0;
    virtual void SetDomain(float x1, float x2, float y1, float y2, int samples) = 0;  // 设置定义域/采样数
    virtual bool GenerateVertices(std::vector<float>& outVertices) = 0;
    virtual FunctionType GetType() const = 0;
    virtual bool CheckMathSyntax(const std::string& expr, std::string& outErr) = 0;//检查数学语法
};

// =================2D笛卡尔生成器=================
class Cartesian2DGenerator : public IFuncVertGenerator
{
public:
    Cartesian2DGenerator(const std::string& expr) : m_expr(expr) {}

    void SetExpression(const std::string& expr) override { m_expr = expr; }
    void SetDomain(float x1, float x2, float, float, int s) override {
        m_x1 = x1; m_x2 = x2; m_samples = s;
    }
    // 生成顶点，调用MathExpressParser求值接口
    bool GenerateVertices(std::vector<float>& outVertices) override {
        return m_parser.Evaluate2D(m_expr, m_x1, m_x2, m_samples, outVertices);
    }

    FunctionType GetType() const override { return FunctionType::Cartesian2D; }
    // 语法检查接口，供UI预检查输入表达式，提升用户体验
    bool CheckMathSyntax(const std::string& expr, std::string& outErr) override {
        double x = 0;
        bool ok = m_parser.CheckMathSyntax(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("x", x); });
        outErr = m_parser.GetError();
        return ok;
    }

private:
    std::string m_expr;
    float m_x1 = -10, m_x2 = 10;
    int m_samples = DEFAULT_SAMPLES;
    MathExpressParser m_parser;
};

// =====================极坐标生成器========================
class PolarGenerator : public IFuncVertGenerator
{
public:
    PolarGenerator(const std::string& expr) : m_expr(expr) {}

    void SetExpression(const std::string& expr) override { m_expr = expr; }
    void SetDomain(float t1, float t2, float, float, int s) override {
        m_t1 = t1; m_t2 = t2; m_samples = s;
    }
    // 生成顶点，调用MathExpressParser求值接口
    bool GenerateVertices(std::vector<float>& outVertices) override {
        return m_parser.EvaluatePolar(m_expr, m_t1, m_t2, m_samples, outVertices);
    }

    FunctionType GetType() const override { return FunctionType::Polar; }
    // 语法检查接口，供UI预检查输入表达式，提升用户体验
    bool CheckMathSyntax(const std::string& expr, std::string& outErr) override {
        double theta = 0;
        bool ok = m_parser.CheckMathSyntax(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("theta", theta); });
        outErr = m_parser.GetError();
        return ok;
    }

private:
    std::string m_expr;
    float m_t1 = 0, m_t2 = 6.28318f;
    int m_samples = DEFAULT_SAMPLES;
    MathExpressParser m_parser;
};

//=====================参数方程生成器========================
class ParametricGenerator : public IFuncVertGenerator
{
public:
    ParametricGenerator(const std::string& expr) : m_expr(expr) {}
    void SetExpression(const std::string& expr) override { m_expr = expr; }
    void SetDomain(float t1, float t2, float, float, int s) override {
        m_t1 = t1; m_t2 = t2; m_samples = s;
    }
    // 生成顶点，调用MathExpressParser求值接口
    bool GenerateVertices(std::vector<float>& outVertices) override {
        return m_parser.EvaluateParametric(m_expr, m_t1, m_t2, m_samples, outVertices);
    }
    FunctionType GetType() const override { return FunctionType::Parametric3D; }
    // 语法检查接口，供UI预检查输入表达式，提升用户体验
    bool CheckMathSyntax(const std::string& expr, std::string& outErr) override {
        double t = 0;
        bool ok = m_parser.CheckMathSyntax(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("t", t); });
        outErr = m_parser.GetError();
        return ok;
    }
private:
    std::string m_expr;
    float m_t1 = -5.0f, m_t2 = 5.0f;
    int m_samples = DEFAULT_SAMPLES;
    MathExpressParser m_parser;
};

//================三维方程生成器
class Cartesian3DGenerator : public IFuncVertGenerator
{
public:
    Cartesian3DGenerator(const std::string& expr) : m_expr(expr) {}
    void SetExpression(const std::string& expr) override { m_expr = expr; }
    void SetDomain(float x1, float x2, float y1, float y2, int s) override {
        m_x1 = x1; m_x2 = x2; m_y1 = y1; m_y2 = y2; m_samples = s;
    }
    bool GenerateVertices(std::vector<float>& outVertices) override {
        return m_parser.Evaluator3D(m_expr, m_x1, m_x2, m_y1, m_y2, m_samples, outVertices);
    }
    FunctionType GetType() const override { return FunctionType::Cartesian3D; }
    bool CheckMathSyntax(const std::string& expr, std::string& outErr) override {
        double x = 0, y = 0;
        bool ok = m_parser.CheckMathSyntax(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("x", x);
            temp_sym.add_variable("y", y);
            });
        outErr = m_parser.GetError();
        return ok;
    }
private:
    std::string m_expr;
    float m_x1 = -5, m_x2 = 5, m_y1 = -5, m_y2 = 5;
    int m_samples = DEFAULT_SAMPLES;
    MathExpressParser m_parser;
};

//================球坐标生成器========================
class SphericalGenerator : public IFuncVertGenerator
{
public:
    SphericalGenerator(const std::string& expr) : m_expr(expr) {}
    void SetExpression(const std::string& expr) override { m_expr = expr; }
    void SetDomain(float theta1, float theta2, float phi1, float phi2, int s) override {
        m_theta1 = theta1; m_theta2 = theta2; m_phi1 = phi1; m_phi2 = phi2; m_samples = s;
    }
    bool GenerateVertices(std::vector<float>& outVertices) override {
        return m_parser.EvaluatorSpherical(m_expr, m_theta1, m_theta2, m_phi1, m_phi2, m_samples, outVertices);
    }
    FunctionType GetType() const override { return FunctionType::Spherical; }
    bool CheckMathSyntax(const std::string& expr, std::string& outErr) override {
        double theta = 0, phi = 0;
        bool ok = m_parser.CheckMathSyntax(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("theta", theta);
            temp_sym.add_variable("phi", phi);
            });
        outErr = m_parser.GetError();
        return ok;
    }
private:
    std::string m_expr;
    float m_theta1 = 0, m_theta2 = 6.28318f, m_phi1 = 0, m_phi2 = 3.14159f;
    int m_samples = DEFAULT_SAMPLES;
    MathExpressParser m_parser;
};

// ==============生成器工厂=====================
class FuncVertGeneratorFactory
{
public:
    static std::unique_ptr<IFuncVertGenerator> CreateGenerator(FunctionType t, const std::string& expr) {
        switch (t) {
        case FunctionType::Cartesian2D:
            return std::make_unique<Cartesian2DGenerator>(expr);
        case FunctionType::Polar:
            return std::make_unique<PolarGenerator>(expr);
        case FunctionType::Parametric3D:
            return std::make_unique<ParametricGenerator>(expr);
        case FunctionType::Cartesian3D:
            return std::make_unique<Cartesian3DGenerator>(expr);
        case FunctionType::Spherical:
            return std::make_unique<SphericalGenerator>(expr);
        default:
            return nullptr;
        }
    }
};

