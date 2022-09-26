#include "Assets/bundled.hpp"
#include "Labs/0-GettingStarted/App.h"

int main() {
    using namespace VCX;
    return Engine::RunApp<Labs::GettingStarted::App>(Engine::AppContextOptions {
        .Title      = "VCX Labs 0: Getting Started",
        .WindowSize = { 800, 600 },
        .FontSize   = 16,

        .IconFileNames = Assets::DefaultIcons,
        .FontFileNames = Assets::DefaultFonts,
    });
}
