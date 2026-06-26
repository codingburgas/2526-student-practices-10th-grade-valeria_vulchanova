#include "ui/theme.h"
#include "ui/screens.h"

int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT|FLAG_WINDOW_HIGHDPI);
    InitWindow(SW,SH,"Movix v4.0 - Cinema Management Suite");
    SetTargetFPS(60);SetExitKey(0);
    LoadFonts();
    RunLoginScreen();
    UnloadFonts();
    CloseWindow();
    return 0;
}
