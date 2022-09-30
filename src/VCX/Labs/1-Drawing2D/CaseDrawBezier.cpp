#include <algorithm>
#include <array>

#include "Labs/1-Drawing2D/CaseDrawBezier.h"
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

    CaseDrawBezier::CaseDrawBezier():
        _empty(Common::CreateCheckboardImageRGB(c_Size.first, c_Size.second)) {
        gl_using(_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, c_Size.first, c_Size.second, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    void CaseDrawBezier::OnSetupPropsUI() {
        ImGui::Indent();
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        ImGui::TextWrapped("Hint: use the right mouse button to drag the point.");
        Common::ImGuiHelper::SaveImage(_texture, c_Size);
        ImGui::Unindent();
    }

    Common::CaseRenderResult CaseDrawBezier::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        auto const [width, height] = c_Size;
        if (_recompute) {
            _recompute = false;
            auto                  tex { Common::CreateCheckboardImageRGB(c_Size.first, c_Size.second) };
            constexpr std::size_t n    = 20;
            glm::fvec2            prev = _handles[0];
            for (std::size_t i = 1; i <= n; ++i) {
                auto const curr = CalculateBezierPoint(
                    _handles,
                    float(i) / float(n));
                DrawLine(tex, { 0, 0, 0 }, prev, curr);
                prev = curr;
            }
            for (std::size_t i = 0; i < _handles.size(); ++i) {
                DrawPoint(tex, { 0.2, 0.7, 0.4 }, _handles[i]);
            }
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

    void CaseDrawBezier::OnProcessInput(ImVec2 const & pos) {
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
                for (int i = 0; i < _handles.size(); ++i) {
                    float dist = (pos.x - _handles[i].x) * (pos.x - _handles[i].x) + (pos.y - _handles[i].y) * (pos.y - _handles[i].y);
                    if (dist < minDist) {
                        minDist    = dist;
                        _selectIdx = i;
                    }
                }
            }
            if (_selectIdx != -1) {
                _handles[_selectIdx]   = _handles[_selectIdx] + glm::fvec2(int(delta.x), int(delta.y));
                _handles[_selectIdx].x = std::min(_handles[_selectIdx].x, float(c_Size.first - 3));
                _handles[_selectIdx].x = std::max(_handles[_selectIdx].x, 2.f);
                _handles[_selectIdx].y = std::min(_handles[_selectIdx].y, float(c_Size.second - 3));
                _handles[_selectIdx].y = std::max(_handles[_selectIdx].y, 2.f);
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
