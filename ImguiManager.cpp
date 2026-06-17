#include "ImguiManager.h"

//=================== 构造函数：初始化ImGui上下文和样式=======================================
ImguiManager::ImguiManager(GLFWwindow* window, ShareData& data, OpenglApp& openglApp)
    : m_data(data), m_openglApp(openglApp) {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // 启用鼠标交互（新版本正确宏定义）
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 同时启用键盘导航（可选）
    // 启用字体点阵（可选，用于非ASCII字符）
    io.Fonts->AddFontDefault();
    // 自动获取系统中文字体路径
    std::string font_path = im_GetSystemChineseFontPath();
    if (font_path.empty()) {
        // 找不到字体时的容错处理（可选）
       // _sdPtr->errorMessages.push_back("警告：未找到系统中文字体，可能无法显示中文！\n");
    }
    else {
        // 加载系统字体并支持中文
        ImFontConfig config;
        config.MergeMode = true;  // 合并到默认字体，不影响英文/数字
        // 加载字体（大小 16.0f 可根据需要调整）
        io.Fonts->AddFontFromFileTTF(font_path.c_str(), 12.0f, &config,
            io.Fonts->GetGlyphRangesChineseFull());  // 完整中文范围
    }
    ImGui::GetStyle().ScrollbarSize = 8.0f;
    // 配置 ImGui 样式（可选）
    ImGui::StyleColorsLight(); // 深色主题，也可选择 StyleColorsDark() 或 StyleColorsClassic()
    // 初始化GLFW和OpenGL3后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

}
//=================== 析构函数：清理ImGui资源=======================================
ImguiManager::~ImguiManager() {
    // 清理ImGui资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
//=================== 每帧调用：开始新一帧ImGui绘制=======================================
void ImguiManager::StartFrame() {
    // 开始新一帧ImGui绘制
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
//=================== 每帧调用：渲染ImGui内容到屏幕=======================================
void ImguiManager::Render() {
    // 渲染ImGui内容到屏幕,绘制 ImGui 内容（需在 OpenGL 其他绘制之后）
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
//=================== 下拉菜单：选择函数表达式类型=======================================
void ImguiManager::DropdownMenu() {
    //std::shared_ptr<ShareData> _sdPtr = shareDataPtr.lock();
    const char* _options[] = { "平面曲线",   "极坐标",   "参数方程",  "三维曲面",  "球极坐标" };
    auto& uiParams = m_data.uiParams;
    int typeIndex_ = static_cast<int>(uiParams.selectedExprType);
    // 开始下拉菜单（按钮显示选中值）
    if (ImGui::BeginCombo("##Dropdown", _options[typeIndex_])) {
        // 遍历选项
        for (int i = 0; i < IM_ARRAYSIZE(_options); i++) {
            bool _isSelected = (uiParams.selectedExprType == static_cast<FunctionType>(i));
            // 绘制选项，点击后更新选中值
            if (ImGui::Selectable(_options[i], _isSelected)) {
                uiParams.selectedExprType = static_cast<FunctionType>(i);
            }
            // 高亮当前选中项
            if (_isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo(); // 结束下拉菜单
    }
}
//============================数学特殊字符面板============================================
void ImguiManager::MathSymbolPanel()
{
    // std::shared_ptr<ShareData> _sdPtr = shareDataPtr.lock();
     // 折叠面板
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 13.5));
    if (ImGui::CollapsingHeader("数学符号面板"))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3)); // 调整按钮间距
        // 判断是否有激活或获得焦点的 ImGui 控件
        //bool anyActive = ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused();// || (ImGui::GetActiveID() != 0);

        // 若无激活控件，给出提示
        if (!m_data.uiParams.inputBoxIsFocused) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "提示：请先点击任意输入框以激活，然后再插入符号");
            ImGui::Spacing();
        }

        if (ImGui::BeginTable("math_symbols_table", 5, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Resizable, ImVec2(-FLT_MIN, 0))) {

            for (int i = 0; i < 15; i++)
            {
                ImGui::TableNextColumn();
                const char* symbol = mathSymbols[i];

                // 若没有激活的输入框则禁用按钮（避免误插入）
                if (!m_data.uiParams.inputBoxIsFocused) {
                    ImGui::BeginDisabled();
                    ImGui::Button(symbol, ImVec2(-FLT_MIN, 0));
                    ImGui::EndDisabled();
                }
                else {
                    if (ImGui::Button(symbol, ImVec2(-FLT_MIN, 0))) {
                        // 安全检查 im_inputBufId 范围
                        if (inputBufId < 0 || inputBufId >= (int)inputBoxDataInstances.size()) {
                            inputBufId = 0;
                        }
                        if (inputBoxDataInstances[inputBufId].selStart != inputBoxDataInstances[inputBufId].selEnd) {
                            // 有选区则替换选区内容
                            if (inputBoxDataInstances[inputBufId].selStart > inputBoxDataInstances[inputBufId].selEnd) {
                                //确保selStart小于selEnd
                                std::swap(inputBoxDataInstances[inputBufId].selStart, inputBoxDataInstances[inputBufId].selEnd);
                            }
                            inputBoxDataInstances[inputBufId].inputBuf.replace(
                                inputBoxDataInstances[inputBufId].selStart,
                                inputBoxDataInstances[inputBufId].selEnd - inputBoxDataInstances[inputBufId].selStart,
                                symbol);
                            // 更新光标位置到插入符号后
                            inputBoxDataInstances[inputBufId].cursorPos = inputBoxDataInstances[inputBufId].selStart + (int)strlen(symbol);
                            inputBoxDataInstances[inputBufId].selStart = inputBoxDataInstances[inputBufId].cursorPos;
                            inputBoxDataInstances[inputBufId].selEnd = inputBoxDataInstances[inputBufId].cursorPos;
                        }
                        else {
                            // 无选区则在光标位置插入
                            inputBoxDataInstances[inputBufId].inputBuf.insert(inputBoxDataInstances[inputBufId].cursorPos, symbol);
                            // 更新光标位置到插入符号后
                            inputBoxDataInstances[inputBufId].cursorPos += (int)strlen(symbol);
                        }
                        // 将键盘焦点放回输入框（可选）
                        ImGui::SetKeyboardFocusHere(-1);
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
    }

    ImGui::PopStyleVar();
}

//======================================函数图像控制面板==================================
void ImguiManager::ControlPanel() {
    //std::shared_ptr<ShareData> _sdPtr = shareDataPtr.lock();
    auto& viewParams = m_data.viewParams;
    if (ImGui::CollapsingHeader("控制面板")) {

        float fullWidth = ImGui::GetContentRegionAvail().x;
        float btnWidth = 80.0f;    // 按钮宽度
        float inputWidth = 80.0f;  // 数值输入框宽度
        // float sliderWidth = fullWidth - btnWidth - inputWidth;//-ImGui::GetStyle().ItemSpacing.x * 2;

           // -------------------------- XY轴平移控制 --------------------------
        ImGui::Text("XY轴平移（鼠标左键+CTRL可拖拽）");

        // 沿X轴平移
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("重置X##ResetTransX")) {
            viewParams.translateX = 0.0f; // 重置X轴平移
            // m_data.NotifyDataChanged();//通知数据变化
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##TransXVal", &viewParams.translateX, 0.1f, 0.5f, "%.1f")) { // 平移无范围限制，直接生效
            // _sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##TransXSlider", &viewParams.translateX, -20.0f, 20.0f, "X: %.1f")) {
            //_sdPtr->NotifyDataChanged();
        }

        // 沿Y轴平移
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("重置Y##ResetTransY")) {
            viewParams.translateY = 0.0f; // 重置Y轴平移
            // _sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##TransYVal", &viewParams.translateY, 0.1f, 0.5f, "%.1f")) {
            //_sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##TransYSlider", &viewParams.translateY, -20.0f, 20.0f, "Y: %.1f"))m_data.eventBus.Dispatch({ EventType::None,-1 });

        ImGui::Separator(); // 分隔线

        // -------------------------- 静态旋转控制（绕X/Y/Z轴） --------------------------
        ImGui::Text("静态旋转（角度）");

        // 绕X轴静态旋转
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("绕X轴##rotXBtn")) {
            viewParams.rotateX = 0.0f; // 重置角度按钮
            //_sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##rotXVal", &viewParams.rotateX, 1.0f, 5.0f, "%.1f")) {
            viewParams.rotateX = glm::clamp(viewParams.rotateX, -360.0f, 360.0f); // 限制角度范围
            //_sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##rotXSlider", &viewParams.rotateX, -360.0f, 360.0f, "X: %.1f°"))m_data.eventBus.Dispatch({ EventType::None, -1 });

        // 绕Y轴静态旋转
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("绕Y轴##rotYBtn")) {
            viewParams.rotateY = 0.0f; // 重置角度按钮
            //_sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##rotYVal", &viewParams.rotateY, 1.0f, 5.0f, "%.1f")) {
            viewParams.rotateY = glm::clamp(viewParams.rotateY, -360.0f, 360.0f);
            //_sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##rotYSlider", &viewParams.rotateY, -360.0f, 360.0f, "Y: %.1f°"))m_data.eventBus.Dispatch({ EventType::None, -1 });

        // 绕Z轴静态旋转
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("绕Z轴##rotZBtn")) {
            viewParams.rotateZ = 0.0f; // 重置角度按钮
            //_sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##rotZVal", &viewParams.rotateZ, 1.0f, 5.0f, "%.1f")) {
            viewParams.rotateZ = glm::clamp(viewParams.rotateZ, -360.0f, 360.0f);

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##rotZSlider", &viewParams.rotateZ, -360.0f, 360.0f, "Z: %.1f°"))m_data.eventBus.Dispatch({ EventType::None, -1 });

        ImGui::Separator(); // 分隔线

        // -------------------------- 连续旋转控制（绕X/Y/Z轴） --------------------------
        ImGui::Text("连续旋转（速度）");

        // 绕X轴连续旋转
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("绕X速##speedXBtn")) {
            viewParams.rotateSpeedX = 0.0f; // 停止旋转按钮

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##speedXVal", &viewParams.rotateSpeedX, 0.1f, 0.5f, "%.1f")) {
            viewParams.rotateSpeedX = glm::clamp(viewParams.rotateSpeedX, -5.0f, 5.0f); // 限制速度范围（负为反向）

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##speedXSlider", &viewParams.rotateSpeedX, -5.0f, 5.0f, "X: %.1f rad/s"))m_data.eventBus.Dispatch({ EventType::None, -1 });

        // 绕Y轴连续旋转
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("绕Y速##speedYBtn")) {
            viewParams.rotateSpeedY = 0.0f; // 停止旋转按钮

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##speedYVal", &viewParams.rotateSpeedY, 0.1f, 0.5f, "%.1f")) {
            viewParams.rotateSpeedY = glm::clamp(viewParams.rotateSpeedY, -5.0f, 5.0f);

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##speedYSlider", &viewParams.rotateSpeedY, -5.0f, 5.0f, "Y: %.1f rad/s"))m_data.eventBus.Dispatch({ EventType::None, -1 });

        // 绕Z轴连续旋转
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("绕Z速##speedZBtn")) {
            viewParams.rotateSpeedZ = 0.0f; // 停止旋转按钮

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##speedZVal", &viewParams.rotateSpeedZ, 0.1f, 0.5f, "%.1f")) {
            viewParams.rotateSpeedZ = glm::clamp(viewParams.rotateSpeedZ, -5.0f, 5.0f);

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##speedZSlider", &viewParams.rotateSpeedZ, -5.0f, 5.0f, "Z: %.1f rad/s"))m_data.eventBus.Dispatch({ EventType::None, -1 });

        ImGui::Separator(); // 分隔线

        // -------------------------- 缩放控制 --------------------------
        ImGui::Text("图形缩放");
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("重 置##scaleResetBtn")) {
            viewParams.scaleVal = 1.0f; // 重置缩放按钮
            // _sdPtr->NotifyDataChanged();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##scaleVal", &viewParams.scaleVal, 0.1f, 0.5f, "%.1f")) {
            viewParams.scaleVal = glm::clamp(viewParams.scaleVal, 0.1f, 10.0f); // 限制缩放范围（0.1倍~5倍）

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##scaleSlider", &viewParams.scaleVal, 0.1f, 10.0f, "比例: %.1f"))m_data.eventBus.Dispatch({ EventType::None, -1 });
    }
}

//=====================================设置面板===============================================
void ImguiManager::SettingsPanel() {
    //std::shared_ptr<ShareData> _sdPtr = shareDataPtr.lock();
    auto& viewParams = m_data.viewParams;
    auto& funcGraphParams = m_data.graphParams;
    auto& displayParams = m_data.displayParams;
    if (ImGui::CollapsingHeader("设置面板")) {
        float itemWidth = ImGui::GetContentRegionAvail().x * 0.45f; // 统一控件宽度
        float btnWidth = 80.0f;    // 按钮宽度
        float inputWidth = 80.0f;  // 数值输入框宽度
        // 坐标系设置
        ImGui::Text("坐标系选择");
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::RadioButton("右手坐标系##RightCoord", &viewParams.useRightHanded, 0)) {
            // 右手坐标系逻辑：无需额外矩阵变换（默认）
            // 可根据需求添加坐标轴方向切换代码
            viewParams.rightCoordMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::RadioButton("左手坐标系##LeftCoord", &viewParams.useRightHanded, 1)) {
            // 左手坐标系逻辑：通过镜像Z轴实现（核心变换）
            // 注：需在display函数中同步应用该变换
            viewParams.rightCoordMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -1.0f));
        }
        ImGui::Separator();

        // 采样点数量设置
        ImGui::Text("采样点数量(50到1000)");
        ImGui::SameLine(btnWidth + ImGui::GetStyle().ItemSpacing.x * 7);
        ImGui::Text("奇点阈值");
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::InputInt("##SamplePoints", &funcGraphParams.sampleCount, 50, 200, 0)) {
            // 限制采样点范围（避免无效值）
            funcGraphParams.sampleCount = std::max<int>(50, std::min<int>(1000, funcGraphParams.sampleCount));
        }
        ImGui::SameLine(btnWidth + ImGui::GetStyle().ItemSpacing.x * 7);
        ImGui::SetNextItemWidth(itemWidth);
        if (ImGui::InputInt("##singularityThreshold", &funcGraphParams.singularityThreshold, 5, 50, 0)) {
            funcGraphParams.singularityThreshold = std::max<int>(1, funcGraphParams.singularityThreshold);
        }
        ImGui::Separator();

        // -------------------------- 线条设置 --------------------------
        ImGui::Text("线条样式设置");
        // 线条粗细
        ImGui::SetNextItemWidth(btnWidth);
        if (ImGui::Button("重置粗细##ResetLineWidth")) {
            displayParams.lineWidth = 1.0f; // 恢复线条粗细默认值
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(inputWidth);
        if (ImGui::InputFloat("##LineWidthVal", &displayParams.lineWidth, 0.1f, 0.5f, "%.1f")) {
            displayParams.lineWidth = glm::clamp(displayParams.lineWidth, 0.5f, 5.0f); // 限制0.5-5.0px范围
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderFloat("##LineWidthSlider", &displayParams.lineWidth, 0.5f, 5.0f, "粗细: %.1fpx");

        // 线条类型选择（下拉菜单）
        const char* lineTypeOpts[] = { "实线",   "虚线",   "点线" };
        ImGui::SetNextItemWidth(itemWidth * 2 + ImGui::GetStyle().ItemSpacing.x + 20);
        if (ImGui::BeginCombo("线条类型##LineTypeCombo", lineTypeOpts[displayParams.lineType])) {
            for (int i = 0; i < IM_ARRAYSIZE(lineTypeOpts); i++) {
                bool isSelected = (displayParams.lineType == i);
                if (ImGui::Selectable(lineTypeOpts[i], isSelected)) {
                    displayParams.lineType = i;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();
        //  XYZ轴范围设置（与全局xMIN/xMAX等关联）
        ImGui::Text("坐标轴范围设置");
        // X轴范围
        ImGui::SetNextItemWidth(itemWidth - 7);
        if (ImGui::InputFloat("≤x≤##X最小值XMin", &displayParams.xAxisMIN)) {
            displayParams.xAxisMIN = std::min(displayParams.xAxisMIN, displayParams.xAxisMAX - 0.1f); // 确保最小值 < 最大值

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(itemWidth - 7);
        if (ImGui::InputFloat("##X最大值XMax", &displayParams.xAxisMAX)) {
            displayParams.xAxisMAX = std::max(displayParams.xAxisMAX, displayParams.xAxisMIN + 0.1f);

        }
        // Y轴范围
        ImGui::SetNextItemWidth(itemWidth - 7);
        if (ImGui::InputFloat("≤y≤##Y最小值YMin", &displayParams.yAxisMIN)) {
            displayParams.yAxisMIN = std::min(displayParams.yAxisMIN, displayParams.yAxisMAX - 0.1f);
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(itemWidth - 7);
        if (ImGui::InputFloat("##Y最大值YMax", &displayParams.yAxisMAX)) {
            displayParams.yAxisMAX = std::max(displayParams.yAxisMAX, displayParams.yAxisMIN + 0.1f);

        }
        // Z轴范围
        ImGui::SetNextItemWidth(itemWidth - 7);
        if (ImGui::InputFloat("≤z≤##Z最小值ZMin", &displayParams.zAxisMIN)) {
            displayParams.zAxisMIN = std::min(displayParams.zAxisMIN, displayParams.zAxisMAX - 0.1f);

        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(itemWidth - 7);
        if (ImGui::InputFloat("##Z最大值ZMax", &displayParams.zAxisMAX)) {
            displayParams.zAxisMAX = std::max(displayParams.zAxisMAX, displayParams.zAxisMIN + 0.1f);
            // m_coordinateAxes.GenerateCoordinateAxes();
             // for (auto& func : functions) func.ExpressionUpdate = true;
            // expressionDirty = true;
        }
        ImGui::Separator();

        //  相机位置设置（修改现有全局变量cameraX/Y/Z）
        ImGui::Text("相机位置");
        ImGui::SetNextItemWidth(itemWidth * 2 + ImGui::GetStyle().ItemSpacing.x + 20);
        ImGui::InputFloat3("##CameraPos", &viewParams.cameraPos.x);
        ImGui::Separator();

        // 物体位置设置（修改现有全局变量positionX/Y/Z）
        ImGui::Text(" 图形位置");
        ImGui::SetNextItemWidth(itemWidth * 2 + ImGui::GetStyle().ItemSpacing.x + 20);
        ImGui::InputFloat3("##ObjectPos", &viewParams.objectPos.x);
        ImGui::Separator();

        //  背景网格设置
        ImGui::Text("背景网格");
        ImGui::Checkbox("显示网格##ShowGrid", &displayParams.showGrid);
        ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x * 2);
        ImGui::SetNextItemWidth(itemWidth);
        ImGui::ColorEdit3("背景颜色##backgroundColor", &displayParams.backgroundColor.x, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x * 2);
        if (ImGui::Button("全部恢复默认##ResetAll", ImVec2(btnWidth, 0))) {
            // rightCoordMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
            funcGraphParams.singularityThreshold = 100;
            funcGraphParams.sampleCount = 200;
            displayParams.lineWidth = 1.0f;
            displayParams.lineType = 0;
            viewParams.cameraPos = glm::vec3(0.0f, 2.0f, 15.0f);
            viewParams.objectPos = glm::vec3(0.0f, 0.0f, 0.0f);
            displayParams.backgroundColor = glm::vec4(0.985f, 0.985f, 0.985f, 1.0f);
            displayParams.xAxisMAX = 10.0f;
            displayParams.xAxisMIN = -10.0f;
            displayParams.yAxisMAX = 10.0f;
            displayParams.yAxisMIN = -10.0f;
            displayParams.zAxisMAX = 10.0f;
            displayParams.zAxisMIN = -10.0f;

            //expressionDirty = true;
        }
    }
}

//===============================输入框回调函数==================================
int ImguiManager::InputBoxCallback(ImGuiInputTextCallbackData* data) {

    if (!data || !data->UserData) return 0;
    InputCallbackContext* ctx = static_cast<InputCallbackContext*>(data->UserData);
    if (!ctx || !ctx->instance) return 0;
    ctx->instance->m_data.uiParams.inputBoxIsFocused = true;
    ctx->instance->inputBufId = ctx->boxIndex;
    // InputBoxData* box = &ctx->instance->inputBoxDataInstances[ctx->boxIndex];
    ctx->instance->inputBoxDataInstances[ctx->boxIndex].cursorPos = data->CursorPos;
    ctx->instance->inputBoxDataInstances[ctx->boxIndex].selStart = data->SelectionStart;   // 更新选区信息（ImGui 提供 SelectionStart/SelectionEnd）
    ctx->instance->inputBoxDataInstances[ctx->boxIndex].selEnd = data->SelectionEnd;

    //box->isFocused = ImGui::IsItemActive() || ImGui::IsItemFocused();

    return 0;
}

//===========在每帧 UI 渲染阶段调用以实际渲染模态错误框（在 NewFrame() 之后、Render() 之前）=================
void ImguiManager::RenderErrorPopup(const std::string& gerror) {
    if (gerror.empty()) return;             // 若没有错误，直接返回
    ImGui::OpenPopup("错误信息"); // 确保 popup 标记为打开//（冗余安全调用）
    // BeginPopupModal 返回 true 当且仅当 popup 被打开且需要显示其内容
    if (ImGui::BeginPopupModal("错误信息", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", gerror.c_str()); // 显示错误信息（自动换行）
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        // 确认按钮，用户点击后关闭 popup
        if (ImGui::Button("确定", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();         // 关闭当前 popup（配对 BeginPopupModal）
        }
        ImGui::EndPopup();                     // 结束 popup 的内容区域
    }
    // ImGui::EndPopup();                     // 结束 popup 的内容区域
}

//===============================函数面板主函数：渲染所有子面板和控件=============================
void ImguiManager::ShowFunctionPanel(MathExpressParser& imParser) {

    // --------------------------
    // 在这里编写 ImGui 界面代码
    // --------------------------

    {
        auto& funcGraphParams = m_data.graphParams;
        auto& displayParams = m_data.displayParams;
        auto& viewParams = m_data.viewParams;
        auto& uiParams = m_data.uiParams;
        // auto& _functionLists = m_data.m_functions;//获取函数列表  
        int _glfwWindowWidth, _glfwWindowHeight;
        glfwGetFramebufferSize(m_openglApp.GetWindow(), &_glfwWindowWidth, &_glfwWindowHeight);
        //glfwGetWindowSize(window, &glfwWindowWidth, &glfwWindowHeight);
        imWindowWidth = _glfwWindowWidth / 5;
        imWindowHeight = _glfwWindowHeight;
        //glfwWindowWidth = (glfwWindowWidth < 900) ? glfwWindowWidth = 900 : glfwWindowWidth;
        ImGui::SetNextWindowPos(ImVec2((_glfwWindowWidth - imWindowWidth) * 1.0f, 0.0f)); //乘以1.0f确保浮点运算
        ImGui::SetNextWindowSize(ImVec2(imWindowWidth * 1.0f, imWindowHeight * 1.0f));

        //从这开始
        ImGui::Begin("函数面板##im_functionPanel", nullptr);

        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 15);
        DropdownMenu();
        static std::array<ImguiManager::InputCallbackContext, 9> callbackContexts;//用于获取输入框ID号
        int _id = 0;
        if (uiParams.selectedExprType == static_cast<FunctionType> (0)) {
            ImGui::Text("y=");
            ImGui::SameLine(); // 让输入框和标签在同一行
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 0;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##exprStr2D", &inputBoxDataInstances[0].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::Text("定义域:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
            _id = 4;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##xMIN2D", &inputBoxDataInstances[4].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::SameLine();
            ImGui::Text("≤x≤");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 5;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##xMAX2D", &inputBoxDataInstances[5].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
        }
        else if (uiParams.selectedExprType == static_cast<FunctionType> (1)) {
            ImGui::Text("ρ=");
            ImGui::SameLine(); // 让输入框和标签在同一行
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 0;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##exprStr_polar", &inputBoxDataInstances[0].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::Text("定义域:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
            _id = 4;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##thetaMIN_Polar", &inputBoxDataInstances[4].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::SameLine();
            ImGui::Text("≤θ≤");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 5;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##thetaMAX_Polar", &inputBoxDataInstances[5].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
        }
        else if (uiParams.selectedExprType == static_cast<FunctionType> (2)) {
            ImGui::Text("x=");
            ImGui::SameLine(); // 让输入框和标签在同一行
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 1;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##exprStr_paramX", &inputBoxDataInstances[1].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::Text("y=");
            ImGui::SameLine(); // 让输入框和标签在同一行
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 2;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##exprStr_paramY", &inputBoxDataInstances[2].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::Text("z=");
            ImGui::SameLine(); // 让输入框和标签在同一行
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 3;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##exprStr_paramZ", &inputBoxDataInstances[3].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::Text("定义域:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
            _id = 4;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##tMIN", &inputBoxDataInstances[4].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::SameLine();
            ImGui::Text("≤t≤");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 5;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##tMAX", &inputBoxDataInstances[5].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
        }
        else if (uiParams.selectedExprType == static_cast<FunctionType> (3)) {
            ImGui::Text("z=");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 0;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##exprStr3D", &inputBoxDataInstances[0].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::Text("定义域:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
            _id = 4;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##xMIN3D", &inputBoxDataInstances[4].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::SameLine();
            ImGui::Text("≤x≤");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 5;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##xMAX3D", &inputBoxDataInstances[5].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::NewLine();
            ImGui::SameLine(60);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
            _id = 6;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##yMIN3D", &inputBoxDataInstances[6].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::SameLine();
            ImGui::Text("≤y≤");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 7;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##yMAX3D", &inputBoxDataInstances[7].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
        }
        else if (uiParams.selectedExprType == static_cast<FunctionType> (4)) {
            ImGui::Text("r=");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 0;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##exprStr3D_polar", &inputBoxDataInstances[0].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::Text("定义域:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
            _id = 4;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##xMIN3D_polar", &inputBoxDataInstances[4].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::SameLine();
            ImGui::Text("≤θ≤");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 5;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##xMAX3D_polar", &inputBoxDataInstances[5].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::NewLine();
            ImGui::SameLine(60);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
            _id = 6;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##yMIN3D_polar", &inputBoxDataInstances[6].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
            ImGui::SameLine();
            ImGui::Text("≤φ≤");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            _id = 7;
            callbackContexts[_id] = { this, _id };
            ImGui::InputText("##yMAX3D_polar", &inputBoxDataInstances[7].inputBuf, ImGuiInputTextFlags_CallbackAlways, InputBoxCallback, (void*)&callbackContexts[_id]);
        }
        float halfwidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        //ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        if (ImGui::Button("确 认 输 入  ", ImVec2(halfwidth, 40))) {
            std::string tError_;
            auto genPtr = FuncVertGeneratorFactory::CreateGenerator(uiParams.selectedExprType, inputBoxDataInstances[0].inputBuf);
            if (uiParams.selectedExprType == static_cast<FunctionType> (3) || uiParams.selectedExprType == static_cast<FunctionType> (4)) {
                if (!inputBoxDataInstances[0].inputBuf.empty()) {
                    //imParser.EvaluateConstant(inputBoxDataInstances[0].inputBuf);
                    if (!genPtr->CheckMathSyntax(inputBoxDataInstances[0].inputBuf, tError_))allError.push_back(tError_);
                    auto _dingYiYu = imParser.EvaluateConstant(inputBoxDataInstances[4].inputBuf);
                    if (!_dingYiYu.has_value()) allError.push_back("定义域输入错误！");
                    else funcGraphParams.xminR = static_cast<float>(_dingYiYu.value());
                    _dingYiYu = imParser.EvaluateConstant(inputBoxDataInstances[5].inputBuf);
                    if (!_dingYiYu.has_value()) allError.push_back("定义域输入错误！");
                    else funcGraphParams.xmaxR = static_cast<float>(_dingYiYu.value());
                    _dingYiYu = imParser.EvaluateConstant(inputBoxDataInstances[6].inputBuf);
                    if (!_dingYiYu.has_value()) allError.push_back("定义域输入错误！");
                    else funcGraphParams.yminR = static_cast<float>(_dingYiYu.value());
                    _dingYiYu = imParser.EvaluateConstant(inputBoxDataInstances[7].inputBuf);
                    if (!_dingYiYu.has_value()) allError.push_back("定义域输入错误！");
                    else funcGraphParams.ymaxR = static_cast<float>(_dingYiYu.value());
                    //xmaxR = static_cast<float>(imParser.CompileExpression(inputBoxDataInstances [5].inputBuf, 0.0f, 0.0f).value());
                    //yminR = static_cast<float>(imParser.CompileExpression(inputBoxDataInstances [6].inputBuf, 0.0f, 0.0f).value());
                   //ymaxR = static_cast<float>(imParser.CompileExpression(inputBoxDataInstances [7].inputBuf, 0.0f, 0.0f).value());
                }
                else allError.push_back("请输入函数表达式！");
            }
            else {
                if (uiParams.selectedExprType == static_cast<FunctionType> (2)) {
                    if (inputBoxDataInstances[1].inputBuf.empty() && inputBoxDataInstances[2].inputBuf.empty() && inputBoxDataInstances[3].inputBuf.empty())allError.push_back("请输入函数表达式！");
                    else {
                        inputBoxDataInstances[0].inputBuf = inputBoxDataInstances[1].inputBuf + '\n' + inputBoxDataInstances[2].inputBuf + '\n' + inputBoxDataInstances[3].inputBuf;
                        /*if (!inputBoxDataInstances[1].inputBuf.empty())imParser.EvaluateConstant(inputBoxDataInstances[1].inputBuf);
                        if (!inputBoxDataInstances[2].inputBuf.empty())imParser.EvaluateConstant(inputBoxDataInstances[2].inputBuf);
                        if (!inputBoxDataInstances[3].inputBuf.empty())imParser.EvaluateConstant(inputBoxDataInstances[3].inputBuf);*/
                        if (!inputBoxDataInstances[1].inputBuf.empty())
                            if (!genPtr->CheckMathSyntax(inputBoxDataInstances[1].inputBuf, tError_))allError.push_back(tError_);
                        if (!inputBoxDataInstances[2].inputBuf.empty())
                            if (!genPtr->CheckMathSyntax(inputBoxDataInstances[2].inputBuf, tError_))allError.push_back(tError_);
                        if (!inputBoxDataInstances[3].inputBuf.empty())
                            if (!genPtr->CheckMathSyntax(inputBoxDataInstances[3].inputBuf, tError_))allError.push_back(tError_);
                        auto _dingYiYu = imParser.EvaluateConstant(inputBoxDataInstances[4].inputBuf);
                        if (!_dingYiYu.has_value()) allError.push_back("定义域输入错误！");
                        else funcGraphParams.xminR = static_cast<float>(_dingYiYu.value());
                        _dingYiYu = imParser.EvaluateConstant(inputBoxDataInstances[5].inputBuf);
                        if (!_dingYiYu.has_value()) allError.push_back("定义域输入错误！");
                        else funcGraphParams.xmaxR = static_cast<float>(_dingYiYu.value());
                        //xminR = static_cast<float>(imParser.CompileExpression(inputBoxDataInstances [4].inputBuf, 0.0f, 0.0f).value());
                       // xmaxR = static_cast<float>(imParser.CompileExpression(inputBoxDataInstances [5].inputBuf, 0.0f, 0.0f).value());
                    }
                }
                else {
                    if (!inputBoxDataInstances[0].inputBuf.empty()) {
                        // imParser.EvaluateConstant(inputBoxDataInstances[0].inputBuf);
                        if (!genPtr->CheckMathSyntax(inputBoxDataInstances[0].inputBuf, tError_))allError.push_back(tError_);
                        auto _dingYiYu = imParser.EvaluateConstant(inputBoxDataInstances[4].inputBuf);
                        if (!_dingYiYu.has_value()) allError.push_back("定义域输入错误！");
                        else funcGraphParams.xminR = static_cast<float>(_dingYiYu.value());
                        _dingYiYu = imParser.EvaluateConstant(inputBoxDataInstances[5].inputBuf);
                        if (!_dingYiYu.has_value()) allError.push_back("定义域输入错误！");
                        else funcGraphParams.xmaxR = static_cast<float>(_dingYiYu.value());
                        //xminR = static_cast<float>(imParser.CompileExpression(inputBoxDataInstances [4].inputBuf, 0.0f, 0.0f).value());
                        //xmaxR = static_cast<float>(imParser.CompileExpression(inputBoxDataInstances [5].inputBuf, 0.0f, 0.0f).value());
                    }
                    else {
                        allError.push_back("请输入函数表达式！");
                    }
                }
            }
            // imParser.CompileExpression(inputBoxDataInstances [0].inputBuf, 0.0f, 0.0f);
            if (allError.empty()) {
                if (uiParams.modifyFuncId != -1)   //修改函数
                {

                    /*_functionLists[uiParams.modiyFuncId]->expression. = inputBoxDataInstances[0].inputBuf;
                    _functionLists[uiParams.modiyFuncId]->expressionType=uiParams.selectedExprType;
                    _functionLists[uiParams.modiyFuncId]->isEnabled=true;
                    _functionLists[uiParams.modiyFuncId]->isDirty=true;   */ //函数是否更改
                    m_data.UpdateExpression(uiParams.modifyFuncId, inputBoxDataInstances[0].inputBuf, uiParams.selectedExprType);
                    uiParams.modifyFuncId = -1;
                }
                else {
                    glm::vec3 im_color = glm::vec3(0.3f + rand() / (float)RAND_MAX * 0.7f, 0.3f + rand() / (float)RAND_MAX * 0.7f, 0.3f + rand() / (float)RAND_MAX * 0.7f);
                    // FunctionProperty newFunc(inputBoxDataInstances [0].inputBuf, im_color, uiParams.selectedExprType,shareDataPtr);
                    //auto newFunc = std::make_unique<FunctionConfig>(inputBoxDataInstances[0].inputBuf, im_color, true,true,uiParams.selectedExprType);
                    auto newFunc = std::make_unique<FunctionConfig>();
                    newFunc->expression = inputBoxDataInstances[0].inputBuf;
                    newFunc->graphColor = im_color;
                    newFunc->type = uiParams.selectedExprType;
                    newFunc->isDirty = true;
                    newFunc->isVisible = true;
                    // newFunc->isEnabled=true;
                   //  _functionLists.push_back(std::move(newFunc));
                    m_data.AddFunction(std::move(*newFunc));
                }
                inputBoxDataInstances[0].inputBuf.clear();
                inputBoxDataInstances[1].inputBuf.clear();
                inputBoxDataInstances[2].inputBuf.clear();
                inputBoxDataInstances[3].inputBuf.clear();
            }
            else
            {
                if (uiParams.selectedExprType == static_cast<FunctionType> (2))inputBoxDataInstances[0].inputBuf.clear();//参数方程特殊处理
            }

            m_data.uiParams.inputBoxIsFocused = false;// 输入确认，取消输入框焦点，防止字符面板因无激活输入框而造成程序退出
            inputBufId = 0;
            // functions.emplace_back(input_buf, im_color, uiParams.selectedExprType);
        }
        ImGui::PopStyleColor(2);
        // ImGui::PopStyleVar();
        ImGui::SameLine();
        MathSymbolPanel();
        ControlPanel();
        //ImGui::SameLine(halfwidth+10);
        SettingsPanel();
        ImGui::Separator();
        ImGui::Text("函数列表：");

        ImGui::BeginChild("函数列表：", ImVec2(imWindowWidth * 0.67f, 0), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetScrollY(0);
        auto _funcListsize = m_data.GetFunctionCount();
        for (int i = 0; i < _funcListsize; i++) {
            const auto& func_ = m_data.GetFunction(i);  //?????????????
            if (!func_)continue;//空指针
            ImVec4 colorStr = ImVec4(func_->graphColor.x, func_->graphColor.y, func_->graphColor.z, 1.0f);
            if (func_->type == FunctionType::Parametric3D) {
                std::string param_x = func_->expression, param_y = "", param_z = "";
                size_t first = func_->expression.find('\n');
                size_t second = func_->expression.find('\n', first + 1);
                param_x = func_->expression.substr(0, first);
                param_y = func_->expression.substr(first + 1, second - first - 1);
                param_z = func_->expression.substr(second + 1);
                ImGui::TextColored(colorStr, "x=%s\ny=%s\nz=%s", param_x.c_str(), param_y.c_str(), param_z.c_str());
            }
            else ImGui::TextColored(colorStr, "%s=%s", func_->type == FunctionType::Cartesian2D ? "y" : func_->type == FunctionType::Polar ? "ρ" : func_->type == FunctionType::Parametric3D ? "z" : "r", func_->expression.c_str());
            //让上下两个BEGINCHILD对齐，输出的方程式与操作按钮在同一水平线上
            ImGui::Dummy(ImVec2(0, (ImGui::GetFrameHeight() - ImGui::CalcTextSize("y").y - ImGui::GetStyle().ItemSpacing.y)));
        }
        ImGui::EndChild();

        ImGui::SameLine(0.0f);
        ImGui::BeginChild("函数操作按钮：", ImVec2(imWindowWidth * 0.33f, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetScrollY(0);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
        _funcListsize = m_data.GetFunctionCount();
        for (int i = 0; i < _funcListsize; i++) {
            auto func_ = m_data.GetFunction(i);
            ImGui::PushID(i);
            bool _funcEnabled = func_->isVisible;
            if (ImGui::Checkbox("##im_check", &_funcEnabled)) {
                func_->isVisible = _funcEnabled;
            }
            ImGui::SameLine(0.0f);
            if (ImGui::Button("m##im_modiy")) {
                uiParams.modifyFuncId = i;
                if (uiParams.selectedExprType == static_cast<FunctionType>(2)) {
                    size_t firstSep = func_->expression.find('\n');
                    size_t secondSep = func_->expression.find('\n', firstSep + 1);
                    inputBoxDataInstances[1].inputBuf = func_->expression.substr(0, firstSep);
                    inputBoxDataInstances[2].inputBuf = func_->expression.substr(firstSep + 1, secondSep - firstSep - 1);
                    inputBoxDataInstances[3].inputBuf = func_->expression.substr(secondSep + 1);
                }
                else inputBoxDataInstances[0].inputBuf = func_->expression;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", "修改函数");
            }
            ImGui::SameLine(0.0f);
            glm::vec3 _color = func_->graphColor;
            if (ImGui::ColorEdit3("##color", (float*)&_color, ImGuiColorEditFlags_NoInputs)) {//修改颜色按钮
                func_->graphColor = _color;
            }
            ImGui::SameLine();
            if (ImGui::Button("x##im_erase")) {                   //删除按钮
                m_data.RemoveFunction(i);
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
            if (func_->type == FunctionType::Parametric3D)ImGui::Dummy(ImVec2(0, (ImGui::CalcTextSize("y").y * 2 - ImGui::GetStyle().ItemSpacing.y * 3 / 2)));
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", "删除函数");
            }
        }
        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::End();                                          // 结束窗口
    }
    if (!allError.empty()) {
        RenderErrorPopup(allError[0]);
        if (!ImGui::IsPopupOpen("错误信息")) {
            allError.erase(allError.begin());
        }
    }

}

