#include <algorithm>
#include <array>

#include "Labs/1-Drawing2D/CaseDrawFilled.h"
#include "Labs/1-Drawing2D/tasks.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Drawing2D {

    static constexpr auto c_Size = std::pair(320U, 320U);

    CaseDrawFilled::CaseDrawFilled():
        _empty(Common::CreateCheckboardImageRGB(c_Size.first, c_Size.second)) {
        gl_using(_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, c_Size.first, c_Size.second, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    void CaseDrawFilled::OnSetupPropsUI() {
        ImGui::Indent();
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        ImGui::TextWrapped("Hint: use the right mouse button to drag the point.");
        Common::ImGuiHelper::SaveImage(_texture, c_Size);
        ImGui::Unindent();
    }

    Common::CaseRenderResult CaseDrawFilled::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        auto const [width, height] = c_Size;
        if (_recompute) {
            _recompute = false;
            auto tex { Common::CreateCheckboardImageRGB(c_Size.first, c_Size.second) };
            DrawTriangleFilled(tex, { 0.6, 0.2, 0.1 }, _vertices[0], _vertices[1], _vertices[2]);
            gl_using(_texture);
            glTexSubImage2D(
                GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, tex.GetBytes().data());
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        return Common::CaseRenderResult {
            .Fixed     = true,
            .Image     = _texture,
            .ImageSize = c_Size,
        };
    }

    void CaseDrawFilled::OnProcessInput(ImVec2 const & pos) {
        auto         window    = ImGui::GetCurrentWindow();
        bool         hovered   = false;
        bool         held      = false;
        bool         rightHeld = false;
        ImVec2 const delta     = ImGui::GetIO().MouseDelta;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), nullptr, &rightHeld, ImGuiButtonFlags_MouseButtonRight);
        if (rightHeld) {
            _recompute = true;
            // find closest point
            if (_selectIdx == -1) {
                float minDist = 200;
                for (int i = 0; i < 3; ++i) {
                    float dist = (pos.x - _vertices[i].x) * (pos.x - _vertices[i].x) + (pos.y - _vertices[i].y) * (pos.y - _vertices[i].y);
                    if (dist < minDist) {
                        minDist    = dist;
                        _selectIdx = i;
                    }
                }
            }
            if (_selectIdx != -1) {
                _vertices[_selectIdx]   = _vertices[_selectIdx] + glm::ivec2(int(delta.x), int(delta.y));
                _vertices[_selectIdx].x = std::min(_vertices[_selectIdx].x, int(c_Size.first - 2));
                _vertices[_selectIdx].x = std::max(_vertices[_selectIdx].x, 1);
                _vertices[_selectIdx].y = std::min(_vertices[_selectIdx].y, int(c_Size.second - 2));
                _vertices[_selectIdx].y = std::max(_vertices[_selectIdx].y, 1);
            }
        } else {
            _selectIdx = -1;
        }
        if (held && delta.x != 0.f && ! rightHeld)
            ImGui::SetScrollX(window, window->Scroll.x - delta.x);
        if (held && delta.y != 0.f && ! rightHeld)
            ImGui::SetScrollY(window, window->Scroll.y - delta.y);
        if (_enableZoom && ! held && ImGui::IsItemHovered())
            Common::ImGuiHelper::ZoomTooltip(_texture, c_Size, pos);
    }
} // namespace VCX::Labs::Drawing2D
