#pragma once
#include "exprtk.hpp"
#include<vector>
#include<string>
#include <optional>
#include <regex> //正则表达式,c++17以上
#include <cmath>
#include <mutex>


/*##################################################################################
* 数学解析专用库（exprTK封装），提供常量求值、2D曲线求值、极坐标求值等接口，供顶点生成器调用
* 提供错误接口，供UI显示错误信息
* 提供表达式语法检查接口，供UI预检查输入表达式
####################################################################################*/

// ===希腊字母转义,用正则表达式批量替换希腊字母 把希腊字母π转为PI========
static std::string SpecialCharToAnsi(const std::string& str) {
    std::string SpecialCharA = str;

    static const std::vector<std::pair<std::regex, std::string>> map = {
        {std::regex(R"(π)"), "PI"},
        {std::regex(R"(θ)"), "theta"},
        {std::regex(R"(α)"), "alpha"},
        {std::regex(R"(β)"), "beta"},
        {std::regex(R"(φ)"), "phi"}
    };
    // 批量替换
    for (const auto& p : map) {
        SpecialCharA = std::regex_replace(SpecialCharA, p.first, p.second);
    }
    return SpecialCharA;
}

// ======================数学解析器（exprtk 封装）====================
class MathExpressParser
{
public:
    MathExpressParser() {
        m_symbolTable.add_constants();
        m_symbolTable.add_constant("e", std::exp(1.0));
        m_symbolTable.add_constant("PI", exprtk::details::numeric::constant::pi);
        m_symbolTable.add_function("fac", [](double n) {
            double r = 1;
            for (int i = 2; i <= (int)n; ++i) r *= i;
            return r;
            });
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

    // ==============================常量求值=============================
    std::optional<double> EvaluateConstant(const std::string& expr) {
        auto r = compile(expr, [](exprtk::symbol_table<double>&) {});
        if (!r) return std::nullopt;
        double v = m_expression.value();
        if (std::isnan(v) || std::isinf(v)) {
            SetError("结果非数值(NaN/Inf)");
            return std::nullopt;
        }
        return v;
    }

    // ========================================2D曲线求值====================================
    bool Evaluate2D(const std::string& expr, float x1, float x2, int cnt, std::vector<float>& outVertices) {
        outVertices.clear();
        double x = 0;
        if (!compile(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("x", x); }))
            return false;

        outVertices.reserve(3 * (cnt + 1));
        for (int i = 0; i <cnt; ++i) {
            x = x1 + (x2 - x1) * i / (cnt-1);
            double y = m_expression.value();
            outVertices.push_back(static_cast<float>(x));
            outVertices.push_back(static_cast<float>(y));
            outVertices.push_back(0.0f);
        }
        return true;
    }

    // ===================================极坐标求值=============================================
    bool EvaluatePolar(const std::string& expr, float t1, float t2, int cnt, std::vector<float>& outVertices) {
        outVertices.clear();
        double theta = 0.0;
        if (!compile(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("theta", theta); }))
            return false;

        outVertices.reserve(3 * (cnt + 1));
        for (int i = 0; i <cnt; ++i) {
            theta = t1 + (t2 - t1) * i /( cnt-1);
            double r = m_expression.value();
            outVertices.push_back(static_cast<float>(r * std::cos(theta)));
            outVertices.push_back(static_cast<float>(r * std::sin(theta)));
            outVertices.push_back(0.0f);
        }
        return true;
    }

    //=======================参数求值（供3D曲线）========================
    // 参数表达式格式：x=expr1;y=expr2;z=expr3，expr里可以用t变量
    // 返回值：true表示成功，false表示失败（表达式错误或结果非数值），outVertices输出顶点数据
    // 示例：x=cos(t);y=sin(t);z=t/10
    // ========================================================================================
    bool EvaluateParametric(const std::string& expr, float t1, float t2, int cnt, std::vector<float>& outVertices) {
        outVertices.clear();
        std::string xExpr, yExpr, zExpr;
        // 解析参数表达式
        {
            size_t firstSep = expr.find('\n');
            size_t secondSep = expr.find('\n', firstSep + 1);
            std::string xExpr = expr.substr(0, firstSep);
            std::string yExpr = expr.substr(firstSep + 1, secondSep - firstSep - 1);
            std::string zExpr = expr.substr(secondSep + 1);
            if (xExpr.empty())xExpr = "0.0";
            if (yExpr.empty())yExpr = "0.0";
            if (zExpr.empty())zExpr = "0.0";

            //计算参数方程的点
            outVertices.resize(3 * (cnt + 1));
            //expr.Expression3DIndices.clear();
            double t = 0.0;
            if (!compile(xExpr, [&](exprtk::symbol_table<double>& temp_sym) {
                temp_sym.add_variable("t", t); }))
                return false;
            //auto xExprVal =std::move( m_expression); //预计算x表达式
            for (int i = 0; i < cnt; i++) {
                t = t1 + i * (t2 - t1) / (cnt - 1);
                double x = m_expression.value();
                outVertices[i * 3] = (float)x;
            }

            if (!compile(yExpr, [&](exprtk::symbol_table<double>& temp_sym) {temp_sym.add_variable("t", t); }))
                return false;
            //auto yExprVal =std::move( m_expression); //预计算y表达式
            for (int i = 0; i < cnt; i++) {
                t = t1 + i * (t2 - t1) / (cnt - 1);
                double y = m_expression.value();
                outVertices[i * 3 + 1] = (float)y;
            }

            if (!compile(zExpr, [&](exprtk::symbol_table<double>& temp_sym) {temp_sym.add_variable("t", t); }))
                return false;
            //auto zExprVal = std::move(m_expression); //预计算z表达式
            for (int i = 0; i < cnt; i++) {
                t = t1 + i * (t2 - t1) / (cnt - 1);
                double z = m_expression.value();
                outVertices[i * 3 + 2] = (float)z;
            }
        }
        return true;
    }

    //=======================三维曲面求值========================
    // 参数表达式格式：z=expr，expr里可以用x和y变量
    // 返回值：true表示成功，false表示失败（表达式错误或结果非数值），outVertices输出顶点数据
    // 示例：z=sin(sqrt(x^2+y^2))/sqrt(x^2+y^2)
    //============================================================
    bool Evaluator3D(const std::string& expr, float x1, float x2, float y1, float y2, int samples, std::vector<float>& outVertices) {
        outVertices.clear();
        double x = 0, y = 0;
        if (!compile(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("x", x);
            temp_sym.add_variable("y", y);
            })) return false;

        int nx = samples;
        int ny = samples;
        outVertices.reserve(nx * ny * 3);
        for (int j = 0; j < ny; j++) {
            y = y1 + (y2 - y1) * j / (ny - 1);
            for (int i = 0; i < nx; i++) {
                x = x1 + (x2 - x1) * i / (nx - 1);
                double z = m_expression.value();
                outVertices.push_back((float)x);
                outVertices.push_back((float)y);
                outVertices.push_back((float)z);
            }
        }
        return true;
    }

    //=======================球坐标求值========================
    // 参数表达式格式：r=expr，expr里可以用theta和phi变量
    // =========================================================
    bool EvaluatorSpherical(const std::string& expr, float theta1, float theta2, float phi1, float phi2, int samples, std::vector<float>& outVertices) {
        outVertices.clear();
        double theta = 0, phi = 0;
        if (!compile(expr, [&](exprtk::symbol_table<double>& temp_sym) {
            temp_sym.add_variable("theta", theta);
            temp_sym.add_variable("phi", phi);
            })) return false;

        int ntheta = samples;
        int nphi = samples;
        outVertices.reserve(ntheta * nphi * 3);

        for (int j = 0; j < nphi; j++) {
            phi = phi1 + (phi2 - phi1) * j / (nphi - 1);
            for (int i = 0; i < ntheta; i++) {
                theta = theta1 + (theta2 - theta1) * i / (ntheta - 1);
                double r = m_expression.value();
                double x = r * sin(phi) * cos(theta);
                double y = r * sin(phi) * sin(theta);
                double z = r * cos(phi);

                outVertices.push_back((float)x);
                outVertices.push_back((float)y);
                outVertices.push_back((float)z);
            }
        }
        return true;
    }


    // =======================表达式语法检查（供UI预检查）==============================
    template<typename F>
    bool CheckMathSyntax(const std::string& expr, F&& addVars) {
        return compile(expr, std::forward<F>(addVars));
    }

private:
    template<typename F>
    bool compile(const std::string& expr, F&& addVars) {
        ClearError();
        m_expression = exprtk::expression<double>();//每次编译前重置expression
        exprtk::symbol_table<double> st = m_symbolTable;
        addVars(st);
        m_expression.register_symbol_table(st);

        std::string clean = SpecialCharToAnsi(expr);
        if (!m_parser.compile(clean, m_expression)) {
            SetError("解析失败: " + m_parser.error());
            return false;
        }
        return true;
    }

    //======================== 错误处理（线程安全）=============================
    void SetError(const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_errMutex);
        m_lastError = msg;
    }

    void ClearError() {
        std::lock_guard<std::mutex> lock(m_errMutex);
        m_lastError.clear();
    }

    exprtk::symbol_table<double> m_symbolTable;
    exprtk::expression<double> m_expression;
    exprtk::parser<double> m_parser;

    std::string m_lastError;
    mutable std::mutex m_errMutex;
};