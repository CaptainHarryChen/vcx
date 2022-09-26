#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Mesh.h"
#include "Engine/GL/shader.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"

namespace VCX::Labs::GettingStarted {

    class CaseResizable : public Common::ICase {
    public:
        CaseResizable();

        virtual std::string_view const GetName() override { return "Draw resizable images"; }
        
        virtual void OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void OnProcessInput(ImVec2 const & pos) override;

    private:
        Engine::GL::UniqueProgram _program;
        Engine::GL::UniqueFrame<> _frame;
        Engine::GL::UniqueMesh    _mesh;

        bool _enableZoom;
    };
}
