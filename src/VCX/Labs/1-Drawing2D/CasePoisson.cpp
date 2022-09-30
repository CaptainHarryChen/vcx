#include <algorithm>
#include <array>

#include "Engine/loader.h"
#include "Labs/1-Drawing2D/CasePoisson.h"
#include "Labs/1-Drawing2D/tasks.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Drawing2D {

    static constexpr auto       c_FrontSize = std::pair(280U, 111U);
    static constexpr auto       c_BackSize  = std::pair(1706U, 1280U);
    static constexpr glm::ivec2 c_Offset { 500U, 300U };

    static void Overlap(Common::ImageRGB & output, Common::ImageRGB const & inputBack, Common::ImageRGB const & inputFront, const glm::ivec2 & offset) {
        output = inputBack;
        for (std::size_t y = 0; y < inputFront.GetSizeY(); ++y)
            for (std::size_t x = 0; x < inputFront.GetSizeX(); ++x) {
                glm::vec3 color = inputFront.GetAt({ x, y });
                output.SetAt({ x + offset.x, y + offset.y }, color);
            }
    }

    CasePoisson::CasePoisson():
        _inputBack(Common::AlphaBlend(
            Engine::LoadImageRGBA("assets/images/lake.jpg"),
            Common::CreatePureImageRGB(c_BackSize.first, c_BackSize.second, { 0, 0, 0 }))),
        _inputFront(Common::AlphaBlend(
            Engine::LoadImageRGBA("assets/images/plane.jpg"),
            Common::CreatePureImageRGB(c_FrontSize.first, c_FrontSize.second, { 0, 0, 0 }))),
        _empty(Common::CreatePureImageRGB(c_BackSize.first, c_BackSize.second, { 0, 0, 0 })) {
        gl_using(_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, c_BackSize.first, c_BackSize.second, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    void CasePoisson::OnSetupPropsUI() {
        ImGui::Indent();
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        if (ImGui::Checkbox("Inpainting", &_enablePoisson)) {
            _recompute = true;
        }
        if (! _task.IsCompleted()) {
            static const std::string t = "Running.....";
            ImGui::Text(t.substr(0, 7 + (int(ImGui::GetTime()/ 0.1f) % 6)).c_str());
        }
        Common::ImGuiHelper::SaveImage(_texture, c_BackSize);
        ImGui::Unindent();
    }

    Common::CaseRenderResult CasePoisson::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        auto const [width, height] = c_BackSize;
        if (_recompute) {
            _recompute = false;
            if (_enablePoisson) {
                _task.Emplace([&inputBack = _inputBack, &inputFront = _inputFront]() {
                    Common::ImageRGB tex({ c_BackSize.first, c_BackSize.second });
                    Inpainting(tex, inputBack, inputFront, c_Offset);
                    return tex;
                });
            } else {
                _task.Emplace([&inputBack = _inputBack, &inputFront = _inputFront]() {
                    Common::ImageRGB tex({ c_BackSize.first, c_BackSize.second });
                    Overlap(tex, inputBack, inputFront, c_Offset);
                    return tex;
                });
            }
        }
        gl_using(_texture);
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, _task.ValueOr(_empty).GetBytes().data());
        glGenerateMipmap(GL_TEXTURE_2D);
        return Common::CaseRenderResult {
            .Fixed     = true,
            .Image     = _texture,
            .ImageSize = c_BackSize,
        };
    }

    void CasePoisson::OnProcessInput(ImVec2 const & pos) {
        auto         window  = ImGui::GetCurrentWindow();
        bool         hovered = false;
        bool         held    = false;
        ImVec2 const delta   = ImGui::GetIO().MouseDelta;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);
        if (held && delta.x != 0.f)
            ImGui::SetScrollX(window, window->Scroll.x - delta.x);
        if (held && delta.y != 0.f)
            ImGui::SetScrollY(window, window->Scroll.y - delta.y);
        if (_enableZoom && ! held && ImGui::IsItemHovered())
            Common::ImGuiHelper::ZoomTooltip(_texture, c_BackSize, pos);
    }
} // namespace VCX::Labs::Drawing2D
