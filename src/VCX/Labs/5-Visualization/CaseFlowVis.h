#pragma once

#include "Engine/Async.hpp"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"

namespace VCX::Labs::Visualization {
    struct VectorField2D {
      std::pair<std::uint32_t, std::uint32_t> size;
      std::vector<glm::vec2> vectors;

      void Resize(std::pair<std::uint32_t, std::uint32_t> const & s) {
          size = s;
          vectors.resize(s.first * s.second);
      }

      glm::vec2 & At(uint32_t x, uint32_t y) {
        x = std::min(x, size.first - 1);
        y = std::min(y, size.second - 1);
        return vectors[x * size.second + y];
      }

      glm::vec2 At(uint32_t x, uint32_t y) const {
        x = std::min(x, size.first - 1);
        y = std::min(y, size.second - 1);
        return vectors[x * size.second + y];
      }
    };

    class CaseFlowVis : public Common::ICase {
    public:
        CaseFlowVis();

        virtual std::string_view const GetName() override { return "Flow Visualization"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

    private:
        Engine::GL::UniqueTexture2D     _texture;
        Common::ImageRGB                _empty;
        Engine::Async<Common::ImageRGB> _task;
        std::array<VectorField2D, 3>    _fields;
        Common::ImageRGB                _noise;

        int _fieldId = 0;
        int _step = 10;

        bool _enableZoom = true;
        bool _recompute  = true;
        bool _running    = false;
    };
}
