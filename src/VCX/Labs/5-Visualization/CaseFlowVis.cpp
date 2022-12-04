#include <algorithm>
#include <array>
#include <fstream>
#include <random>

#include "Labs/5-Visualization/CaseFlowVis.h"
#include "Labs/5-Visualization/tasks.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Visualization {
    static constexpr auto c_Size = std::pair(720U, 720U);

    static constexpr auto c_FieldName = std::array<char const *, 3> {
        "Bipole",
        "Circle",
        "Turbulence"
    };

    CaseFlowVis::CaseFlowVis():
        _texture(
            Engine::GL::SamplerOptions {
                .MinFilter = Engine::GL::FilterMode::Linear,
                .MagFilter = Engine::GL::FilterMode::Nearest }),
        _empty(
            Common::CreatePureImageRGB(c_Size.first, c_Size.second, { 0, 0, 0 })),
        _noise(
          Common::CreatePureImageRGB(c_Size.first, c_Size.second, { 0, 0, 0 })) {
        // field 0
        _fields[0].Resize(c_Size);
        for (uint32_t i = 0; i < c_Size.first; ++i) {
            for (uint32_t j = 0; j < c_Size.second; ++j) {
                float x             = float(i + .5f) / c_Size.first;
                float y             = float(j + .5f) / c_Size.second;
                float vx            = (x - .5f) * (x - .5f) - (y - .5f) * (y - .5f) - 0.0625f;
                float vy            = 2 * (x - .5f) * (y - .5f);
                _fields[0].At(i, j) = glm::vec2(vx, vy);
            }
        }
        // field 1
        _fields[1].Resize(c_Size);
        for (uint32_t i = 0; i < c_Size.first; ++i) {
            for (uint32_t j = 0; j < c_Size.second; ++j) {
                float x             = float(i + .5f) / c_Size.first;
                float y             = float(j + .5f) / c_Size.second;
                float vx            = .5f - y;
                float vy            = x - .5f;
                _fields[1].At(i, j) = glm::vec2(vx, vy);
            }
        }
        // field 2
        std::ifstream file("assets/misc/Turbulence_720.txt");
        _fields[2].Resize(c_Size);
        for (uint32_t i = 0; i < c_Size.first; ++i) {
            for (uint32_t j = 0; j < c_Size.second; ++j) {
                float vx, vy;
                file >> vx >> vy;
                _fields[2].At(i, j) = glm::vec2(vx, vy);
            }
        }
        file.close();
        // noise
        std::default_random_engine            e;
        std::uniform_real_distribution<float> u(.0, 1.);
        glm::vec3 c1(.79f, .85f, 0.83f);
        glm::vec3 c2(.14f, .16f, .31f);
        for (std::size_t i = 0; i < c_Size.first / 2; ++i) {
            for (std::size_t j = 0; j < c_Size.second / 2; ++j) {
                float r= u(e);
                _noise.At(2 * i + 0, 2 * j + 0) = c1 * r + c2 * (1 - r);
                _noise.At(2 * i + 1, 2 * j + 0) = c1 * r + c2 * (1 - r);
                _noise.At(2 * i + 0, 2 * j + 1) = c1 * r + c2 * (1 - r);
                _noise.At(2 * i + 1, 2 * j + 1) = c1 * r + c2 * (1 - r);
            }
        }
    }

    void CaseFlowVis::OnSetupPropsUI() {
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        _recompute |= ImGui::Combo("Field", &_fieldId, c_FieldName.data(), c_FieldName.size());
        _recompute |= ImGui::SliderInt("Step", &_step, 0, 50);
        if (_running) {
            static const std::string t = "Running.....";
            ImGui::Text(t.substr(0, 7 + (static_cast<int>(ImGui::GetTime() / 0.1f) % 6)).c_str());
        }
        Common::ImGuiHelper::SaveImage(_texture, c_Size, false);
    }

    Common::CaseRenderResult CaseFlowVis::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        auto const [width, height] = c_Size;
        if (_recompute) {
            _recompute = false;
            _task.Emplace([=]() {
                Common::ImageRGB ret;
                LIC(ret, _noise, _fields[_fieldId], _step);
                return ret;
            });
            _running = true;
        }
        if (_running && _task.HasValue()) _running = false;
        _texture.Update(_task.ValueOr(_empty));
        return Common::CaseRenderResult {
            .Fixed     = true,
            .Image     = _texture,
            .ImageSize = c_Size,
        };
    }

    void CaseFlowVis::OnProcessInput(ImVec2 const & pos) {
        auto         window  = ImGui::GetCurrentWindow();
        bool         hovered = false;
        bool         anyHeld = false;
        ImVec2 const delta   = ImGui::GetIO().MouseDelta;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &anyHeld);
        if (! hovered) return;
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && delta.x != 0.f)
            ImGui::SetScrollX(window, window->Scroll.x - delta.x);
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && delta.y != 0.f)
            ImGui::SetScrollY(window, window->Scroll.y - delta.y);
        if (_enableZoom && ! anyHeld && ImGui::IsItemHovered())
            Common::ImGuiHelper::ZoomTooltip(_texture, c_Size, pos);
    }
} // namespace VCX::Labs::Visualization
