#pragma once

#include "Engine/Async.hpp"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"

namespace VCX::Labs::GettingStarted {

    class CaseFixed : public Common::ICase {
    public:
        CaseFixed();

        virtual std::string_view const GetName() override { return "Draw fixed images"; }
        
        virtual void OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void OnProcessInput(ImVec2 const & pos) override;
    
    private:

        std::array<Engine::GL::UniqueTexture2D, 2> _textures;

        std::array<Common::ImageRGB, 2> _empty;

        Engine::Async<Common::ImageRGB> _task;

        int  _sizeId     = 0;
        int  _bgId       = 0;
        bool _enableZoom = true;
        bool _recompute  = true;
    };
}
