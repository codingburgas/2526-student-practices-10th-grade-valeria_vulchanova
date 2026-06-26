#pragma once
#include "../raylib/include/raylib.h"
#include <string>
#include <vector>

static const int SW=1100, SH=720;

// Colours
extern const Color C_BG,C_PANEL,C_GOLD,C_GOLD2,C_GOLDDIM,C_WHITE,
                   C_GREY,C_GREYLT,C_INPUTBG,C_RED,C_GREEN,C_ACCENT;
// Fonts
extern Font fTitle,fUI,fUIBold;
void LoadFonts(); void UnloadFonts();

// Helpers
Color CA(Color c,float a);
Color LerpC(Color a,Color b,float t);
void DrawTextC(Font f,const char* txt,float cx,float y,float sz,float sp,Color c);
std::string TruncText(const char* txt,Font f,float sz,float sp,float maxW);
std::vector<std::string> WrapText(const std::string& s,Font f,float sz,float sp,float maxW);

// Particles
struct Particle{float x,y,vx,vy,r,life,maxLife;Color col;};
extern std::vector<Particle> gParts;
void UpdateParticles(float dt);void DrawParticles();

// Chrome
void DrawFilm(int x,int y,int h,float t,bool right);
void DrawScanlines();
void DrawGlowRect(Rectangle r,Color col,float str,int layers);
void DrawHeader(const char* subtitle,float t,bool showLogout=false);
void DrawNavBack(float x,float y);

// Widgets
struct Field{Rectangle rect={};std::string buf;bool active=false,isPwd=false;float blink=0,hoverT=0;const char* label="";};
struct Button{Rectangle rect={};float hoverT=0,pressT=0;const char* text="";};
void   FieldInit(Field& f,Rectangle r,const char* lbl,bool pwd);
void   FieldUpdate(Field& f,float dt);
void   FieldDraw(const Field& f);
void   BtnInit(Button& b,Rectangle r,const char* t);
bool   BtnUpdate(Button& b,float dt);
void   BtnDraw(const Button& b);
void   BtnDrawDisabled(const Button& b);

// Logo
void DrawLogo(float cx,float y,float t);
void DrawClapper(float cx,float y,float t);

// Rating stars (interactive: returns new rating if clicked, else current)
void DrawStars(float x,float y,float rating,float sz,Color col);
int  DrawInteractiveStars(float x,float y,int current,float sz); // returns clicked star (1-5) or current
