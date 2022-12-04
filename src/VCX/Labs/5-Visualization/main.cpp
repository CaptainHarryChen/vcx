#include "Assets/bundled.h"
#include "Labs/5-Visualization/App.h"

int main() {
    using namespace VCX;
    return Engine::RunApp<Labs::Visualization::App>(Engine::AppContextOptions {
        .Title      = "VCX Labs 5: Visualization",
        .WindowSize = { 1024, 768 },
        .FontSize   = 16,

        .IconFileNames = Assets::DefaultIcons,
        .FontFileNames = Assets::DefaultFonts,
    });
}
