#include "Assets/bundled.h"
#include "Labs/Photon_Mapping/App.h"

int main() {
    using namespace VCX;
    return Engine::RunApp<Labs::Rendering::App>(Engine::AppContextOptions {
        .Title      = "VCX Labs 3: Rendering",
        .WindowSize = { 1024, 768 },
        .FontSize   = 16,

        .IconFileNames = Assets::DefaultIcons,
        .FontFileNames = Assets::DefaultFonts,
    });
}
