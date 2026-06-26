// win_dialog.cpp  –  Windows file dialog, NO raylib includes
// Keeping windows.h separate prevents ShowCursor / DrawText / CloseWindow conflicts.
#include "win_dialog.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

std::string PickImageFile() {
    char path[MAX_PATH] = {};
    OPENFILENAMEA ofn   = {};
    ofn.lStructSize  = sizeof(ofn);
    ofn.lpstrFilter  = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";
    ofn.lpstrFile    = path;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrTitle   = "Select Movie Poster";
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) return std::string(path);
    return "";
}
#else
std::string PickImageFile() { return ""; }
#endif
