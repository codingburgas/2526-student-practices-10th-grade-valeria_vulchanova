#pragma once
#include <string>
// Opens a native OS file picker for images.
// Isolated from raylib to avoid Windows API symbol conflicts.
std::string PickImageFile();
