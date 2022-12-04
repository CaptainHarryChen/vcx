#pragma once

#include <vector>
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Visualization {

	struct Car {
        float mileage;
        int   cylinders;
        float displacement;
        float horsepower;
        float weight;
        float acceleration;
        int   year;
	};

    class InteractProxy {
    public:
        glm::vec2 MousePos() const;
        glm::vec2 MouseDeltaPos() const;
        bool      IsHovering() const;
        bool      IsClicking(bool left = true) const;
        bool      IsDragging(bool left = true) const;
        glm::vec2 DraggingStartPoint(bool left = true) const;
        void      Update(ImVec2 const & size, ImVec2 const & pos, ImVec2 const & delta, bool hover, bool lHeld, bool rHeld);

    private:
        glm::ivec2 _size      = glm::ivec2(0);
        glm::vec2  _pos       = glm::vec2(0);
        glm::vec2  _dPos      = glm::vec2(0);
        glm::vec2  _lStart    = glm::vec2(0);
        glm::vec2  _rStart    = glm::vec2(0);
        bool       _hover     = false;
        bool       _move      = false;
        bool       _lClick    = false;
        bool       _rClick    = false;
        bool       _lDrag     = false;
        bool       _rDrag     = false;
        bool       _lLastDrag = false;
        bool       _rLastDrag = false;
    };

    void SetBackGround(Common::ImageRGB & canvas, glm::vec4 color);
    void DrawLine(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 from, glm::vec2 to, float width);
    void DrawRect(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 leftTop, glm::vec2 size, float width);
    void DrawFilledRect(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 leftTop, glm::vec2 size);
    void DrawCircle(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 origin, float radius, float width);
    void DrawFilledCircle(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 origin, float radius);
    bool PrintText(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 pos, float lineHeight, std::string const & caption);

}