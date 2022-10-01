#include <algorithm>
#include <array>

#include <stb_image_write.h>

#include "Engine/loader.h"
#include "Labs/1-Drawing2D/CaseDithering.h"
#include "Labs/1-Drawing2D/tasks.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Drawing2D {

    static constexpr auto c_Size      = std::pair(180U, 215U);
    static constexpr auto c_SizeLarge = std::pair(3 * 180U, 3 * 215U);
    static constexpr auto c_NoiseSize = std::pair(256U, 256U);

    CaseDithering::CaseDithering():
        _input(Common::AlphaBlend(
            Engine::LoadImageRGBA("assets/images/david-180x215.png"),
            Common::CreatePureImageRGB(c_Size.first, c_Size.second, { 0, 0, 0 }))),
        _empty(Common::CreatePureImageRGB(c_Size.first, c_Size.second, { 0, 0, 0 })),
        _emptyLarge(Common::CreatePureImageRGB(c_SizeLarge.first, c_SizeLarge.second, { 0, 0, 0 })),
        _blueNoise(Common::AlphaBlend(
            Engine::LoadImageRGBA("assets/images/bluenoise-256x256.png"),
            Common::CreatePureImageRGB(c_NoiseSize.first, c_NoiseSize.second, { 0, 0, 0 }))),
        _algType(AlgorithmType::Original) {
        gl_using(_textureLarge);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, c_SizeLarge.first, c_SizeLarge.second, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        gl_using(_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, c_Size.first, c_Size.second, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    void CaseDithering::OnSetupPropsUI() {
        ImGui::Indent();
        ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        ImGui::Text("Algorithm Type");
        AlgorithmType oldType = _algType;
        ImGui::Bullet();
        if (ImGui::Selectable("Original", _algType == AlgorithmType::Original)) _algType = AlgorithmType::Original;
        ImGui::Bullet();
        if (ImGui::Selectable("Threshold", _algType == AlgorithmType::Threshold)) _algType = AlgorithmType::Threshold;
        ImGui::Bullet();
        if (ImGui::Selectable("Uniform Random", _algType == AlgorithmType::Random)) _algType = AlgorithmType::Random;
        ImGui::Bullet();
        if (ImGui::Selectable("Blue Noise", _algType == AlgorithmType::BlueNoise)) _algType = AlgorithmType::BlueNoise;
        ImGui::Bullet();
        if (ImGui::Selectable("Ordered", _algType == AlgorithmType::Ordered)) _algType = AlgorithmType::Ordered;
        ImGui::Bullet();
        if (ImGui::Selectable("Error Diffuse", _algType == AlgorithmType::ErrorDiffuse)) _algType = AlgorithmType::ErrorDiffuse;
        if (_isLarge) {
            Common::ImGuiHelper::SaveImage(_textureLarge, c_SizeLarge);
        } else {
            Common::ImGuiHelper::SaveImage(_texture, c_Size);
        }
        ImGui::Unindent();
        if (oldType != _algType) _recompute = true;
    }

    Common::CaseRenderResult CaseDithering::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        if (_recompute) {
            _recompute = false;
            switch (_algType) {
            case AlgorithmType::Original:
                _task.Emplace([&input = _input]() {
                    return input;
                });
                break;
            case AlgorithmType::Threshold:
                _task.Emplace([&input = _input]() {
                    Common::ImageRGB tex({ c_Size.first, c_Size.second });
                    DitheringThreshold(tex, input);
                    return tex;
                });
                break;
            case AlgorithmType::Random:
                _task.Emplace([&input = _input]() {
                    Common::ImageRGB tex({ c_Size.first, c_Size.second });
                    DitheringRandomUniform(tex, input);
                    return tex;
                });
                break;
            case AlgorithmType::BlueNoise:
                _task.Emplace([&input = _input, &noise = _blueNoise]() {
                    Common::ImageRGB tex({ c_Size.first, c_Size.second });
                    DitheringRandomBlueNoise(tex, input, noise);
                    return tex;
                });
                break;
            case AlgorithmType::Ordered:
                _task.Emplace([&input = _input]() {
                    Common::ImageRGB tex({ c_SizeLarge.first, c_SizeLarge.second });
                    DitheringOrdered(tex, input);
                    return tex;
                });
                break;
            case AlgorithmType::ErrorDiffuse:
                _task.Emplace([&input = _input]() {
                    Common::ImageRGB tex({ c_Size.first, c_Size.second });
                    DitheringErrorDiffuse(tex, input);
                    return tex;
                });
                break;
            default:
                break;
            }
        }
        if (_algType == AlgorithmType::Ordered) {
            _isLarge = true;
            gl_using(_textureLarge);
            glTexSubImage2D(
                GL_TEXTURE_2D, 0, 0, 0, c_SizeLarge.first, c_SizeLarge.second, GL_RGB, GL_UNSIGNED_BYTE, _task.ValueOr(_emptyLarge).GetBytes().data());
            glGenerateMipmap(GL_TEXTURE_2D);
            return Common::CaseRenderResult {
                .Fixed     = true,
                .Image     = _textureLarge,
                .ImageSize = c_SizeLarge,
            };
        } else {
            _isLarge = false;
            gl_using(_texture);
            glTexSubImage2D(
                GL_TEXTURE_2D, 0, 0, 0, c_Size.first, c_Size.second, GL_RGB, GL_UNSIGNED_BYTE, _task.ValueOr(_empty).GetBytes().data());
            glGenerateMipmap(GL_TEXTURE_2D);
            return Common::CaseRenderResult {
                .Fixed     = true,
                .Image     = _texture,
                .ImageSize = c_Size,
            };
        }
    }

    void CaseDithering::OnProcessInput(ImVec2 const & pos) {
        auto         window  = ImGui::GetCurrentWindow();
        bool         hovered = false;
        bool         held    = false;
        ImVec2 const delta   = ImGui::GetIO().MouseDelta;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);
        if (held && delta.x != 0.f)
            ImGui::SetScrollX(window, window->Scroll.x - delta.x);
        if (held && delta.y != 0.f)
            ImGui::SetScrollY(window, window->Scroll.y - delta.y);
        if (_enableZoom && ! held && ImGui::IsItemHovered()) {
            if (_isLarge) Common::ImGuiHelper::ZoomTooltip(_texture, c_SizeLarge, pos);
            else Common::ImGuiHelper::ZoomTooltip(_texture, c_Size, pos);
        }
    }
} // namespace VCX::Labs::Drawing2D
