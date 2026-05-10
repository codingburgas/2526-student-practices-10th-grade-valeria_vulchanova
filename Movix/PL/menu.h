#pragma once
#include "logIn.h"   // reuses all shared types, colours, fonts, helpers

struct Movie {
    const char* title;
    const char* genre;
    const char* director;
    const char* cast;        // leading stars
    const char* duration;
    const char* year;
    float       rating;      // 0 – 10
    float       price;       // BGN
    const char* desc;
    Color       posterA;     // gradient top colour
    Color       posterB;     // gradient bottom colour
    Color       accent;      // highlight colour for that card
};

void RunMovieScreen();