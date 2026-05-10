#pragma once
#include "logIn.h"   

struct Movie {
    const char* title;
    const char* genre;
    const char* director;
    const char* cast;        
    const char* duration;
    const char* year;
    float       rating;      
    float       price;       
    const char* desc;
    Color       posterA;     
    Color       posterB;    
    Color       accent;      
};

void RunMovieScreen();