#include <fstream>


#include "Labs/5-Visualization/CaseParallelCoordinates.h"
#include "Labs/5-Visualization/tasks.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Visualization {
    static constexpr auto c_Size = std::pair(960U, 960U);

    CaseParallelCoordinates::CaseParallelCoordinates():
        _texture(
            Engine::GL::SamplerOptions {
                .MinFilter = Engine::GL::FilterMode::Linear,
                .MagFilter = Engine::GL::FilterMode::Nearest }),
        _empty(c_Size.first, c_Size.second),
        _msaa(false),
        _recompute(true) {

        _empty.Fill(glm::vec3(1));

        std::ifstream file("assets/misc/cars.txt");
        int           total = 0;
        file >> total;
        _data.reserve(total);
        for (int i = 0; i < total; ++i) {
            auto & [f1, f2, f3, f4, f5, f6, f7] = _data.emplace_back();
            file >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7;
        }
    }

    void CaseParallelCoordinates::OnSetupPropsUI() {
        _recompute |= ImGui::Checkbox("Anti-Aliasing", &_msaa);
        Common::ImGuiHelper::SaveImage(_texture, {c_Size.first * (_msaa ? 2 : 1), c_Size.second * (_msaa ? 2 : 1)}, false);
    }

    Common::CaseRenderResult CaseParallelCoordinates::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        Common::ImageRGB result(c_Size.first * (_msaa ? 2 : 1), c_Size.second * (_msaa ? 2 : 1));
        if (PaintParallelCoordinates(result, _proxy, _data, _recompute)) {
            _recompute = false;
            _texture.Update(result);
        }
        return Common::CaseRenderResult {
            .Fixed     = true,
            .Image     = _texture,
            .ImageSize = c_Size,
        };
    }

    void CaseParallelCoordinates::OnProcessInput(ImVec2 const & pos) {
        auto         window  = ImGui::GetCurrentWindow();
        bool         hovered = false;
        bool         anyHeld = false;
        auto &       io      = ImGui::GetIO();
        ImVec2 const delta   = io.MouseDelta;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &anyHeld);
        _proxy.Update(
            ImVec2(c_Size.first, c_Size.second),
            pos,
            delta,
            hovered,
            ImGui::IsMouseDown(ImGuiMouseButton_Left),
            ImGui::IsMouseDown(ImGuiMouseButton_Right));
    }
}