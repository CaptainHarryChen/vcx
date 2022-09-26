#include <algorithm>
#include <array>

#include "Labs/0-GettingStarted/CaseFixed.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::GettingStarted {

    static constexpr auto c_Sizes = std::to_array<std::pair<std::uint32_t, std::uint32_t>>({
        { 320U, 320U },
        { 640U, 640U } 
    });

    static constexpr auto c_SizeItems = std::array<char const *, 2> {
        "Small (320 x 320)",
        "Large (640 x 640)"
    };

    static constexpr auto c_BgItems = std::array<char const *, 3> {
        "White",
        "Black",
        "Checkboard"
    };

    CaseFixed::CaseFixed() : 
        _empty({
            Common::CreatePureImageRGB(c_Sizes[0].first, c_Sizes[0].second, { 2.f / 17, 2.f / 17, 2.f / 17 }),
            Common::CreatePureImageRGB(c_Sizes[1].first, c_Sizes[1].second, { 2.f / 17, 2.f / 17, 2.f / 17 })
        }) {
        for (std::size_t i = 0; i < _textures.size(); ++i) {
            auto const useTexture { _textures[i].Use() };
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, c_Sizes[i].first, c_Sizes[i].second, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        }
    }

    void CaseFixed::OnSetupPropsUI() {
        auto newSizeId = _sizeId;
        auto newBgId   = _bgId;

        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        ImGui::Combo("Size", &newSizeId, c_SizeItems.data(), c_SizeItems.size());
        ImGui::Combo("Background", &newBgId, c_BgItems.data(), c_BgItems.size());

        if (newSizeId != _sizeId || newBgId != _bgId)
            _recompute = true;

        _sizeId = newSizeId;
        _bgId   = newBgId;
    }

    Common::CaseRenderResult CaseFixed::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        auto const width = c_Sizes[_sizeId].first;
        auto const height = c_Sizes[_sizeId].second;
        // auto const [width, height] = c_Sizes[_sizeId];
        if (_recompute) {
            _recompute = false;
            _task.Emplace([=]() {
                Common::ImageRGB image({ 0, 0 });
                switch (_bgId) {
                case 0:
                    image = Common::CreatePureImageRGB(width, height, { 1., 1., 1. });
                    break;
                case 1:
                    image = Common::CreatePureImageRGB(width, height, { 0., 0., 0. });
                    break;
                case 2:
                    image = Common::CreateCheckboardImageRGB(width, height);
                    break;
                }
                for (std::size_t x = 0; x < width; ++x)
                    for (std::size_t y = 0; y < height; ++y)
                        if (x + y < width) image.SetAt({ x, y }, { 1., 0., 0. });
                return image;
            });
        }
        auto const useTexture { _textures[_sizeId].Use() };
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, width, height,
            GL_RGB, GL_UNSIGNED_BYTE,
            _task.ValueOr(_empty[_sizeId]).GetBytes().data());
        glGenerateMipmap(GL_TEXTURE_2D);
        return Common::CaseRenderResult {
            .Fixed     = true,
            .Image     = _textures[_sizeId],
            .ImageSize = c_Sizes[_sizeId],
        };
    }

    void CaseFixed::OnProcessInput(ImVec2 const & pos) {
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
            Common::ImGuiHelper::ZoomTooltip(_textures[_sizeId], c_Sizes[_sizeId], pos);
    }
}
