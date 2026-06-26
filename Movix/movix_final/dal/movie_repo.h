#pragma once
#include "types.h"
#include <vector>

// Colour theme presets for the Add Movie form
struct ColourTheme { int pA_r,pA_g,pA_b, pB_r,pB_g,pB_b, ac_r,ac_g,ac_b; const char* name; };
extern const ColourTheme COLOUR_THEMES[8];

void MovieRepo_Init();                          // load (seed defaults if absent)
int  MovieRepo_Count();
const MovieEntry& MovieRepo_Get(int idx);
bool MovieRepo_Add(MovieEntry e);
bool MovieRepo_Delete(int idx);
void MovieRepo_Save();
