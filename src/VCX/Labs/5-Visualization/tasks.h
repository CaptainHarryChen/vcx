#pragma once

#include "Labs/Common/ImageRGB.h"
#include "Labs/5-Visualization/CaseFlowVis.h"
#include "Labs/5-Visualization/Data.h"

namespace VCX::Labs::Visualization {
    bool PaintParallelCoordinates(Common::ImageRGB & input, InteractProxy const & proxy, std::vector<Car> const & data, bool force);
    void LIC(Common::ImageRGB &, Common::ImageRGB const &, VectorField2D const &, int const &);
};