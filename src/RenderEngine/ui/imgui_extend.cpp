#include "imgui_extend.h"

void ImGui::BeginHorizontal(){
    ImGuiWindow* window = GetCurrentWindow();
    // window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(bar_rect.Min.x + window->DC.MenuBarOffset.x, bar_rect.Min.y + window->DC.MenuBarOffset.y);
    window->DC.LayoutType = ImGuiLayoutType_Horizontal;
}

void ImGui::EndHorizontal(){
    ImGuiWindow* window = GetCurrentWindow();
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
}

void ImGui::sampleButton(const char* label, bool *v)
{
    float padding = 10.0f;
    float bounding = 1.0f;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 label_size = ImGui::CalcTextSize(label);
    const ImVec2 button_size = ImVec2(label_size.x + padding * 2, label_size.y + padding * 2);
    const ImVec2 bound_size =  ImVec2(button_size.x + bounding * 2, button_size.y + bounding * 2);
    ImVec2 p_button = ImVec2(p.x + bounding, p.y + bounding);
    ImVec2 p_label = ImVec2(p_button.x + padding, p_button.y + padding);

    float radius = bound_size.y * 0.30f;

    // 透明的按钮
    if (ImGui::InvisibleButton(label, bound_size))
        *v = !*v;
    ImVec4 col_bf4;
    ImGuiStyle& style = ImGui::GetStyle();

    // 颜色自定义
    if (ImGui::IsItemActivated()) col_bf4 = *v ? style.Colors[40] : style.Colors[23];
    else if (ImGui::IsItemHovered()) col_bf4 =  *v ? style.Colors[42] : style.Colors[24];
    else col_bf4 = *v ? style.Colors[41] : style.Colors[22];

    ImU32 col_bg = IM_COL32(255 * col_bf4.x, 255 * col_bf4.y, 255 * col_bf4.z, 255 * col_bf4.w);
    ImU32 col_text = IM_COL32(255, 255, 255, 255);
    ImU32 col_bound = IM_COL32(0,0,0,255);
    
    // 绘制矩形形状
    draw_list->AddRect(p, ImVec2(p.x + bound_size.x, p.y + bound_size.y), col_bound , radius);
    draw_list->AddRectFilled(p_button, ImVec2(p_button.x + button_size.x, p_button.y + button_size.y), col_bg, radius);
    draw_list->AddText(p_label, col_text, label);
}

void ImGui::toggleButton(ImTextureID texId, const char* label, bool *v)
{
    if (*v == true)
    {

        ImGui::PushID(label);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(230/255.0, 179/255.0, 0/255.0, 105/255.0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(230/255.0, 179/255.0, 0/255.0, 255/255.0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(255/255.0, 153/255.0, 0/255.0, 255/255.0));
        ImGui::ImageButtonWithText(texId, label);
        if (ImGui::IsItemClicked(0))
        {
            *v = !*v;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    else
    {
        if (ImGui::ImageButtonWithText(texId ,label))
            *v = true;
    }
}

void ImGui::toggleButton(const char* label, bool *v)
{
    if (*v == true)
    {

        ImGui::PushID(label);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(230/255.0, 179/255.0, 0/255.0, 105/255.0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(230/255.0, 179/255.0, 0/255.0, 255/255.0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(255/255.0, 153/255.0, 0/255.0, 255/255.0));
        ImGui::Button(label);
        if (ImGui::IsItemClicked(0))
        {
            *v = !*v;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    else
    {
        if (ImGui::Button(label))
            *v = true;
    }
}
bool ImGui::ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col) {
    ImGuiWindow* window = GetCurrentWindow();
    
    if (window->SkipItems)
    return false;

    ImVec2 size = imageSize;
    if (size.x<=0 && size.y<=0) {size.x=size.y=ImGui::GetTextLineHeightWithSpacing();}
    else {
        if (size.x<=0)          size.x=size.y;
        else if (size.y<=0)     size.y=size.x;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(label);
    const ImVec2 textSize = ImGui::CalcTextSize(label,NULL,true);
    const bool hasText = textSize.x>0;

    const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
    const ImVec2 totalSizeWithoutPadding(size.x+innerSpacing+textSize.x,size.y>textSize.y ? size.y : textSize.y);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding*2);
    ImVec2 start(0,0);
    start = window->DC.CursorPos + padding;
    if (size.y<textSize.y) start.y+=(textSize.y-size.y)*.5f;

    const ImRect image_bb(start, start + size);
    start = window->DC.CursorPos + padding;start.x+=size.x+innerSpacing;
    if (size.y>textSize.y) start.y+=(size.y-textSize.y)*.5f;
    
    ItemSize(bb);
    if (!ItemAdd(bb, id))
    return false;

    bool hovered=false, held=false;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if (bg_col.w > 0.0f)
    window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

    window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

    if (textSize.x>0) ImGui::RenderText(start,label);
    return pressed;
}

void ImGui::beginTitle(const char* label){
    // 避免label输出，ImGui ID压入栈中
    ImGui::PushID(label);
}

void ImGui::endTitle(){
    // ImGui ID弹栈
    ImGui::PopID();
}

ImU32 ImGui::VecToImU(const dyno::Vec3f* v)
{
    return IM_COL32((*v)[0], (*v)[1], (*v)[2], 150);
}

bool ImGui::ColorBar(const char* label, const int* values, const dyno::Vec3f* col, int length)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImDrawList* draw_list = window->DrawList;
    ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;
    ImGuiID id = window->GetID(label);
    
    const float width = CalcItemWidth();
    g.NextItemData.ClearFlags();

    PushID(label);
    BeginGroup();

    float text_width = GetFrameHeight();
    float bars_width = GetFrameHeight();
    float bars_height = ImMax(bars_width * 1, width - 1 * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
    ImVec2 bar_pos = window->DC.CursorPos;
    ImRect bb(bar_pos, bar_pos + ImVec2(bars_width + text_width, bars_height));
    int grid_count = length - 1;  
    
    // Set Item Size
    ItemSize(ImVec2(bars_width + text_width, bars_height));
    if(!ItemAdd(bb, id))
        return false;
    // InvisibleButton("detail",ImVec2(bars_width + text_width, bars_height));
    // fprintf(stderr,"%d\n", grid_count);
    for (int i = 0; i < grid_count; ++i){
        draw_list->AddRectFilledMultiColor(
            ImVec2(bar_pos.x, bar_pos.y + i * (bars_height / grid_count)), 
            ImVec2(bar_pos.x + bars_width, bar_pos.y + (i + 1) * (bars_height / grid_count)), 
            VecToImU(col + i), VecToImU(col + i), VecToImU(col + i + 1), VecToImU(col + i + 1));
        char buf[20];
        sprintf(buf,"%d", values[i]);
        draw_list->AddText(ImVec2(bar_pos.x + bars_width,  bar_pos.y + i * (bars_height / grid_count)), IM_COL32(255,255,255,255),buf);
    }
    RenderFrameBorder(ImVec2(bar_pos.x, bar_pos.y), ImVec2(bar_pos.x + bars_width, bar_pos.y + bars_height), 0.0f);
    EndGroup();
    PopID();
    return true;
}


bool ImGui::ColorBar(const char* label, const int* values, const ImU32* col, int length)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImDrawList* draw_list = window->DrawList;
    ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;
    ImGuiID id = window->GetID(label);
    
    const float width = CalcItemWidth();
    g.NextItemData.ClearFlags();

    PushID(label);
    BeginGroup();

    float text_width = GetFrameHeight();
    float bars_width = GetFrameHeight();
    float bars_height = ImMax(bars_width * 1, width - 1 * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
    ImVec2 bar_pos = window->DC.CursorPos;
    ImRect bb(bar_pos, bar_pos + ImVec2(bars_width + text_width, bars_height));
    int grid_count = length - 1;  
    
    // Set Item Size
    ItemSize(ImVec2(bars_width + text_width, bars_height));
    if(!ItemAdd(bb, id))
        return false;
    // InvisibleButton("detail",ImVec2(bars_width + text_width, bars_height));
    // fprintf(stderr,"%d\n", grid_count);
    for (int i = 0; i < grid_count; ++i){
        draw_list->AddRectFilledMultiColor(
            ImVec2(bar_pos.x, bar_pos.y + i * (bars_height / grid_count)), 
            ImVec2(bar_pos.x + bars_width, bar_pos.y + (i + 1) * (bars_height / grid_count)), 
            col[i], col[i], col[i + 1], col[i + 1]);
        char buf[20];
        sprintf(buf,"%d", values[i]);
        draw_list->AddText(ImVec2(bar_pos.x + bars_width,  bar_pos.y + i * (bars_height / grid_count)), IM_COL32(255,255,255,255),buf);
    }
    RenderFrameBorder(ImVec2(bar_pos.x, bar_pos.y), ImVec2(bar_pos.x + bars_width, bar_pos.y + bars_height), 0.0f);
    EndGroup();
    PopID();
    return true;
}