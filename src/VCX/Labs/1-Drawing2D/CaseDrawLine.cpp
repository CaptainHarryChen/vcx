#include <algorithm>
#include <array>

#include "Labs/1-Drawing2D/CaseDrawLine.h"
#include "Labs/1-Drawing2D/tasks.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Drawing2D {

    static constexpr auto c_Size = std::pair(320U, 320U);

    static void DrawPoint(Common::ImageRGB & canvas, glm::vec3 color, glm::ivec2 pos) {
        for (int dx = -2; dx <= 2; ++dx) {
            for (int dy = -2; dy <= 2; ++dy) {
                canvas.SetAt({ unsigned(pos.x + dx), unsigned(pos.y + dy) }, color);
            }
        }
    }

    CaseDrawLine::CaseDrawLine():
        _empty(Common::CreateCheckboardImageRGB(c_Size.first, c_Size.second)) {
        gl_using(_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, c_Size.first, c_Size.second, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    void CaseDrawLine::OnSetupPropsUI() {
        ImGui::Indent();
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        ImGui::TextWrapped("Hint: use the right mouse button to drag the point.");
        Common::ImGuiHelper::SaveImage(_texture, c_Size);
        ImGui::Unindent();
    }

    Common::CaseRenderResult CaseDrawLine::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        auto const [width, height] = c_Size;
        if (_recompute) {
            _recompute = false;
            auto tex { Common::CreateCheckboardImageRGB(c_Size.first, c_Size.second) };
            DrawLine(tex, { 0, 0, 0 }, _lineP0, _lineP1);
            DrawPoint(tex, { 0.2, 0.7, 0.4 }, _lineP0);
            DrawPoint(tex, { 0.2, 0.7, 0.4 }, _lineP1);
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

    void CaseDrawLine::OnProcessInput(ImVec2 const & pos) {
        auto         window    = ImGui::GetCurrentWindow();
        bool         hovered   = false;
        bool         held      = false;
        bool         rightHeld = false;
        ImVec2 const delta     = ImGui::GetIO().MouseDelta;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), nullptr, &rightHeld, ImGuiButtonFlags_MouseButtonRight);
        if (rightHeld) {
            _recompute = true;

            if (_selectIdx == -1) {
                float minDist = 200;
                // find closest point
                float dist0 = (pos.x - _lineP0.x) * (pos.x - _lineP0.x) + (pos.y - _lineP0.y) * (pos.y - _lineP0.y);
                float dist1 = (pos.x - _lineP1.x) * (pos.x - _lineP1.x) + (pos.y - _lineP1.y) * (pos.y - _lineP1.y);
                if (dist0 < dist1 && dist0 < minDist) _selectIdx = 0;
                else if (dist1 < dist0 && dist1 < minDist) _selectIdx = 1;
            }
            if (_selectIdx == 0) { // select point 0
                _lineP0   = _lineP0 + glm::ivec2(int(delta.x), int(delta.y));
                _lineP0.x = std::min(_lineP0.x, int(c_Size.first - 3));
                _lineP0.x = std::max(_lineP0.x, 2);
                _lineP0.y = std::min(_lineP0.y, int(c_Size.second - 3));
                _lineP0.y = std::max(_lineP0.y, 2);
            } else if (_selectIdx == 1) { // select point 1
                _lineP1   = _lineP1 + glm::ivec2(int(delta.x), int(delta.y));
                _lineP1.x = std::min(_lineP1.x, int(c_Size.first - 3));
                _lineP1.x = std::max(_lineP1.x, 2);
                _lineP1.y = std::min(_lineP1.y, int(c_Size.second - 3));
                _lineP1.y = std::max(_lineP1.y, 2);
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
