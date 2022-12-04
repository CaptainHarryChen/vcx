#pragma once

#include "Engine/Async.hpp"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/ICase.h"
#include "Labs/5-Visualization/Data.h"

namespace VCX::Labs::Visualization {

	class CaseParallelCoordinates : public Common::ICase {
    public:
        CaseParallelCoordinates();

        virtual std::string_view const GetName() override { return "Parallel Coordinates"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

    private:
        Engine::GL::UniqueTexture2D      _texture;
        Common::ImageRGB                 _empty;
        std::vector<Car>                 _data;
        InteractProxy                    _proxy;
        bool                             _msaa;

        bool _recompute;
    };

}