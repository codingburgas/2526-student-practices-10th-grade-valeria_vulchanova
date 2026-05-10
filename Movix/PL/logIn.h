#pragma once
#include "raylib.h"
#include <string>
#include <vector>

static const int SW = 1100;
static const int SH = 680;

extern const Color C_BG;
extern const Color C_PANEL;
extern const Color C_GOLD;
extern const Color C_GOLD2;
extern const Color C_GOLDDIM;
extern const Color C_WHITE;
extern const Color C_GREY;
extern const Color C_GREYLT;
extern const Color C_INPUTBG;
extern const Color C_RED;
extern const Color C_ACCENT;

extern Font fTitle;
extern Font fUI;
extern Font fUIBold;

void LoadFonts();
void UnloadFonts();


Color CA(Color c, float a);
Color LerpC(Color a, Color b, float t);
void  DrawTextC(Font f, const char* t, float cx, float y,
    float sz, float sp, Color c);

struct Particle {
    float x, y, vx, vy, r, life, maxLife;
    Color col;
};

extern std::vector<Particle> gParts;

void SpawnParticle();
void UpdateParticles(float dt);
void DrawParticles();

struct Field {
    Rectangle   rect;
    std::string buf;
    bool        active, isPwd;
    float       blink, hoverT;
    const char* label;
};

void FieldInit(Field& f, Rectangle r, const char* lbl, bool pwd);
void FieldUpdate(Field& f, float dt);
void FieldDraw(const Field& f);

struct Button {
    Rectangle   rect;
    float       hoverT, pressT;
    const char* text;
};

void BtnInit(Button& b, Rectangle r, const char* t);
bool BtnUpdate(Button& b, float dt);
void BtnDraw(const Button& b);


void DrawFilm(int x, int y, int h, float t, bool rightEdge);
void DrawScanlines();
void DrawGlowRect(Rectangle r, Color col, float str, int layers = 6);
void DrawLogo(float cx, float y, float t);
void DrawClapper(float cx, float y, float t);


void RunLoginScreen();