#include "logIn.h"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(SW, SH, "Movix – Cinema Management");
    SetTargetFPS(60);
    LoadFonts();

    RunLoginScreen();

    UnloadFonts();
    CloseWindow();
}