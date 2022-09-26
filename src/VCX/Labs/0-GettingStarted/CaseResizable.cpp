#include "Labs/0-GettingStarted/CaseResizable.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::GettingStarted {
    struct Vertex {
        glm::vec2 Position;
        glm::vec2 _0;
        glm::vec3 Color;
        glm::vec1 _1;
    };

    static constexpr auto c_VertexData = std::to_array<Vertex>({
        { .Position { -0.5, -0.5 }, .Color { 1, 0, 0 }},
        { .Position {  0  ,  0.5 }, .Color { 0, 1, 0 }},
        { .Position {  0.5, -0.5 }, .Color { 0, 0, 1 }},
    });

    CaseResizable::CaseResizable() :
         _program(
            VCX::Engine::GL::UniqueProgram({
                VCX::Engine::GL::SharedShader("assets/shaders/triangle.vert"),
                VCX::Engine::GL::SharedShader("assets/shaders/triangle.frag") })),
        _mesh(
            VCX::Engine::GL::VertexLayout()
                .Of<Vertex>()
                .At(0, &Vertex::Position)
                .At(1, &Vertex::Color),
            VCX::Engine::make_span_bytes<Vertex>(c_VertexData)) {
    }

    void CaseResizable::OnSetupPropsUI() {
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
    }

    Common::CaseRenderResult CaseResizable::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        _frame.Resize(desiredSize);
        _frame.Use();
        const auto [width, height] = desiredSize;
        gl_using(_frame);
        _mesh.Draw({ _program.Use() });
        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseResizable::OnProcessInput(ImVec2 const& pos) {
        auto         window  = ImGui::GetCurrentWindow();
        bool         hovered = false;
        bool         held    = false;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);
        if (_enableZoom && ! held && ImGui::IsItemHovered())
            Common::ImGuiHelper::ZoomTooltip(_frame.GetColorAttachment(), _frame.GetSize(), pos, true);
    }
}

