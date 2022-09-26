#include <algorithm>

#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Common::ImGuiHelper {
	void TextCentered(std::string_view const text) {
        auto const windowWidth = ImGui::GetWindowSize().x;
        auto const textWidth   = ImGui::CalcTextSize(text.data()).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * .5f);
        ImGui::Text(text.data());
	}

	void ZoomTooltip(
		Engine::GL::UniqueTexture2D const & tex,
		std::pair<std::uint32_t, std::uint32_t> texSize,
		ImVec2 const & pos,
		bool const flipped,
		float const regionSize,
		float const zoomLevel) {
		auto const [width, height] = texSize;
		ImGui::BeginTooltip();
		float  regionX = std::clamp(pos.x - regionSize * .5f, 0.f, width - regionSize);
		float  regionY = std::clamp(pos.y - regionSize * .5f, 0.f, height - regionSize);
		ImGui::Text("Min: (%.0f, %.0f)", regionX, regionY);
		ImGui::Text("Max: (%.0f, %.0f)", regionX + regionSize, regionY + regionSize);
		ImVec2 uv0 = ImVec2(regionX / width, flipped ? 1 - regionY / height : regionY / height);
		ImVec2 uv1 = ImVec2((regionX + regionSize) / width, flipped ? 1 - (regionY + regionSize) / height : (regionY + regionSize) / height);
		ImGui::Image(
			reinterpret_cast<void *>(std::uintptr_t(tex.Get())),
			ImVec2(regionSize * zoomLevel, regionSize * zoomLevel),
			uv0, uv1,
			{ 1, 1, 1, 1 }, { 1, 1, 1, .5f });
		ImGui::EndTooltip();
	}
}
