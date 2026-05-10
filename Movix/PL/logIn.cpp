#include "logIn.h"
#include "menu.h"  
#include <cmath>
#include <algorithm>

const Color C_BG = { 5,   5,  10, 255 };
const Color C_PANEL = { 12,  11,  20, 255 };
const Color C_GOLD = { 212, 163,  57, 255 };
const Color C_GOLD2 = { 255, 210, 100, 255 };
const Color C_GOLDDIM = { 100,  76,  22, 255 };
const Color C_WHITE = { 235, 232, 225, 255 };
const Color C_GREY = { 95,  92, 100, 255 };
const Color C_GREYLT = { 155, 152, 162, 255 };
const Color C_INPUTBG = { 18,  17,  28, 255 };
const Color C_RED = { 210,  60,  60, 255 };
const Color C_ACCENT = { 80, 160, 220, 255 };

Font fTitle;
Font fUI;
Font fUIBold;

void LoadFonts() {
    if (FileExists("Cinzel-Bold.ttf"))
        fTitle = LoadFontEx("Cinzel-Bold.ttf", 80, nullptr, 0);
    else fTitle = GetFontDefault();

    if (FileExists("Raleway-Regular.ttf"))
        fUI = LoadFontEx("Raleway-Regular.ttf", 32, nullptr, 0);
    else fUI = GetFontDefault();

    if (FileExists("Raleway-Bold.ttf"))
        fUIBold = LoadFontEx("Raleway-Bold.ttf", 32, nullptr, 0);
    else fUIBold = GetFontDefault();

    SetTextureFilter(fTitle.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fUI.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fUIBold.texture, TEXTURE_FILTER_BILINEAR);
}

void UnloadFonts() {
    UnloadFont(fTitle);
    UnloadFont(fUI);
    UnloadFont(fUIBold);
}

Color CA(Color c, float a) {
    c.a = (unsigned char)(std::clamp(a, 0.f, 1.f) * 255.f);
    return c;
}

Color LerpC(Color a, Color b, float t) {
    return {
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

void DrawTextC(Font f, const char* t, float cx, float y,
    float sz, float sp, Color c) {
    Vector2 s = MeasureTextEx(f, t, sz, sp);
    DrawTextEx(f, t, { cx - s.x * 0.5f, y }, sz, sp, c);
}


std::vector<Particle> gParts;

void SpawnParticle() {
    float cx = SW * 0.5f;
    float cy = SH * 0.5f;
    float angle = GetRandomValue(0, 628) * 0.01f;
    float speed = GetRandomValue(5, 30) * 0.1f;
    Color cols[] = { C_GOLD, C_GOLD2, C_WHITE, C_ACCENT };

    Particle p;
    p.x = cx + cosf(angle) * GetRandomValue(0, 200) * 1.f;
    p.y = cy + sinf(angle) * GetRandomValue(0, 150) * 1.f;
    p.vx = cosf(angle) * speed;
    p.vy = sinf(angle) * speed;
    p.r = GetRandomValue(1, 3) * 0.5f;
    p.maxLife = GetRandomValue(60, 180) * 0.016f;
    p.life = p.maxLife;
    p.col = cols[GetRandomValue(0, 3)];
    gParts.push_back(p);
}

void UpdateParticles(float dt) {
    if ((int)gParts.size() < 120 && GetRandomValue(0, 3) == 0)
        SpawnParticle();
    for (auto& p : gParts) { p.x += p.vx * dt; p.y += p.vy * dt; p.life -= dt; }
    gParts.erase(
        std::remove_if(gParts.begin(), gParts.end(),
            [](const Particle& p) { return p.life <= 0; }),
        gParts.end());
}

void DrawParticles() {
    for (auto& p : gParts)
        DrawCircleV({ p.x, p.y }, p.r, CA(p.col, (p.life / p.maxLife) * 0.55f));
}

void DrawFilm(int x, int y, int h, float t, bool rightEdge) {
    int sw = 28, sh = 20, sg = 10;
    DrawRectangle(x, y, sw, h, CA(C_GOLDDIM, 0.12f));
    int off = (int)(fmodf(t * 30.f, (float)(sh + sg)));
    for (int sy = y - (sh + sg) + off; sy < y + h + sh; sy += sh + sg) {
        DrawRectangle(x + 4, sy, sw - 8, sh, CA(C_GOLD, 0.18f));
        DrawRectangleLinesEx(
            { (float)(x + 4), (float)sy, (float)(sw - 8), (float)sh },
            0.8f, CA(C_GOLDDIM, 0.4f));
    }
    int ex = rightEdge ? x + sw - 2 : x;
    DrawRectangle(ex, y, 2, h, CA(C_GOLD, 0.25f));
}

void DrawScanlines() {
    for (int y = 0; y < SH; y += 3)
        DrawRectangle(0, y, SW, 1, CA(C_BG, 0.18f));
}

void DrawGlowRect(Rectangle r, Color col, float str, int layers) {
    for (int i = layers; i >= 1; i--) {
        float e = (float)i * 3.f;
        DrawRectangleLinesEx(
            { r.x - e, r.y - e, r.width + e * 2, r.height + e * 2 },
            1.f, CA(col, str * (1.f - (float)i / (float)(layers + 1)) * 0.5f));
    }
}
void FieldInit(Field& f, Rectangle r, const char* lbl, bool pwd) {
    f.rect = r;
    f.buf = "";
    f.active = false;
    f.isPwd = pwd;
    f.blink = 0;
    f.hoverT = 0;
    f.label = lbl;
}

void FieldUpdate(Field& f, float dt) {
    f.blink += dt;
    if (f.active) {
        int k = GetCharPressed();
        while (k > 0) {
            if (k >= 32 && k <= 126 && (int)f.buf.size() < 32)
                f.buf += (char)k;
            k = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !f.buf.empty())
            f.buf.pop_back();
    }
    bool hover = CheckCollisionPointRec(GetMousePosition(), f.rect);
    f.hoverT += dt * (hover ? 6.f : -6.f);
    f.hoverT = std::clamp(f.hoverT, 0.f, 1.f);
}

void FieldDraw(const Field& f) {
    float glow = f.active ? 1.f : f.hoverT * 0.4f;
    Color border = f.active
        ? C_GOLD
        : LerpC(CA(C_GREY, 0.5f), CA(C_GOLDDIM, 0.8f), f.hoverT);

    if (glow > 0.01f) DrawGlowRect(f.rect, C_GOLD, glow * 0.6f, 5);
    DrawRectangleRec(f.rect, C_INPUTBG);
    DrawRectangleLinesEx(f.rect, f.active ? 1.8f : 1.2f, border);

    // floating label
    bool  up = f.active || !f.buf.empty();
    float lsz = up ? 11.f : 15.f;
    float ly = up
        ? f.rect.y - 18.f
        : f.rect.y + (f.rect.height - 16.f) * 0.5f;
    Color lc = up ? CA(C_GOLD, 0.9f) : CA(C_GREY, 0.8f);
    DrawTextEx(fUIBold, f.label, { f.rect.x + 2, ly }, lsz, 2, lc);

    // content
    std::string shown = f.isPwd ? std::string(f.buf.size(), '*') : f.buf;
    float tsz = 18.f;
    float ty = f.rect.y + (f.rect.height - tsz) * 0.5f;
    DrawTextEx(fUI, shown.c_str(), { f.rect.x + 14, ty }, tsz, 1, C_WHITE);

    // cursor
    if (f.active && fmodf(f.blink, 1.f) < 0.55f) {
        Vector2 ms = MeasureTextEx(fUI, shown.c_str(), tsz, 1);
        DrawRectangle((int)(f.rect.x + 14 + ms.x + 2), (int)ty, 2, (int)tsz, C_GOLD);
    }
}

void BtnInit(Button& b, Rectangle r, const char* t) {
    b.rect = r;
    b.hoverT = 0;
    b.pressT = 0;
    b.text = t;
}

bool BtnUpdate(Button& b, float dt) {
    bool hover = CheckCollisionPointRec(GetMousePosition(), b.rect);
    bool clicked = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    b.hoverT += dt * (hover ? 8.f : -8.f);
    b.hoverT = std::clamp(b.hoverT, 0.f, 1.f);
    if (clicked) b.pressT = 0.12f;
    if (b.pressT > 0) b.pressT -= dt;
    return clicked;
}

void BtnDraw(const Button& b) {
    float pulse = sinf(GetTime() * 2.5f) * 0.5f + 0.5f;
    Color base = LerpC(C_GOLD, C_GOLD2, b.hoverT);
    float sc = b.pressT > 0 ? 0.97f : 1.f;
    Rectangle r = {
        b.rect.x + b.rect.width * (1.f - sc) * 0.5f,
        b.rect.y + b.rect.height * (1.f - sc) * 0.5f,
        b.rect.width * sc,
        b.rect.height * sc
    };
    DrawGlowRect(r, C_GOLD, 0.3f + b.hoverT * 0.5f + pulse * 0.15f, 7);
    DrawRectangleRec(r, base);
    DrawRectangleGradientV(
        (int)r.x, (int)r.y, (int)r.width, (int)r.height / 2,
        CA(C_WHITE, 0.14f), CA(C_WHITE, 0.f));
    float    sz = 17.f;
    Vector2  ts = MeasureTextEx(fUIBold, b.text, sz, 3);
    DrawTextEx(fUIBold, b.text,
        { r.x + (r.width - ts.x) * 0.5f, r.y + (r.height - ts.y) * 0.5f },
        sz, 3, C_BG);
}


void DrawLogo(float cx, float y, float t) {
    float   pulse = 0.55f + 0.2f * sinf(t * 1.8f);
    Vector2 ts = MeasureTextEx(fTitle, "MOVIX", 58, 5);
    for (int i = 8; i >= 1; i--) {
        float e = (float)i * 5.f;
        DrawRectangle(
            (int)(cx - ts.x * 0.5f - e), (int)(y - e),
            (int)(ts.x + e * 2), (int)(58 + e * 2),
            CA(C_GOLD, pulse * 0.035f));
    }
    DrawTextC(fTitle, "MOVIX", cx + 2, y + 2, 58, 5, CA(C_BG, 0.8f));
    DrawTextC(fTitle, "MOVIX", cx, y, 58, 5, C_GOLD2);
    DrawTextC(fUI, "CINEMA MANAGEMENT SYSTEM", cx, y + 70, 13, 5, CA(C_GREYLT, 0.7f));
    float lw = 200.f;
    DrawLineEx({ cx - lw * 0.5f, y + 90 }, { cx + lw * 0.5f, y + 90 }, 1.f, CA(C_GOLDDIM, 0.6f));
    DrawCircle((int)cx, (int)(y + 90), 3, CA(C_GOLD, 0.7f));
}

void DrawClapper(float cx, float y, float t) {
    float w = 56, h = 42, x = cx - w * 0.5f;
    DrawRectangle((int)x, (int)(y + 14), (int)w, (int)h, CA(C_GOLDDIM, 0.2f));
    DrawRectangleLinesEx({ x, (float)(y + 14), w, h }, 1.5f, CA(C_GOLD, 0.5f));
    for (int i = 0; i < 5; i++) {
        float sx = x + (float)i * 12.f;
        DrawLine((int)sx, (int)(y + 14), (int)(sx + 8), (int)y, CA(C_GOLD, 0.55f));
    }
    DrawLine((int)x, (int)(y + 14), (int)(x + w), (int)(y + 14), CA(C_GOLD, 0.55f));
    DrawCircleLinesV({ cx, y + 14 + h * 0.5f }, 10, CA(C_ACCENT, 0.35f));
    DrawCircle((int)cx, (int)(y + 14 + h * 0.5f), 5, CA(C_ACCENT, 0.2f));
    (void)t;
}

// ── RunLoginScreen ────────────────────────────────────────────────
void RunLoginScreen() {
    float panelW = 460.f, panelH = 530.f;
    float panelX = (SW - panelW) * 0.5f;
    float panelY = (SH - panelH) * 0.5f;
    float cx = SW * 0.5f;

    Field userF, passF;
    FieldInit(userF, { panelX + 40, panelY + 260, panelW - 80, 50 }, "USERNAME", false);
    FieldInit(passF, { panelX + 40, panelY + 348, panelW - 80, 50 }, "PASSWORD", true);

    Button loginBtn;
    BtnInit(loginBtn, { panelX + 40, panelY + 432, panelW - 80, 52 }, "SIGN IN");

    std::string msg = "";
    Color       msgCol = C_RED;
    float       msgAlpha = 0.f, msgTimer = 0.f;
    bool        loggedIn = false;
    float       loginAnim = 0.f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float t = GetTime();

        
        UpdateParticles(dt);
        FieldUpdate(userF, dt);
        FieldUpdate(passF, dt);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            userF.active = CheckCollisionPointRec(GetMousePosition(), userF.rect);
            passF.active = CheckCollisionPointRec(GetMousePosition(), passF.rect);
        }
        if (IsKeyPressed(KEY_TAB)) {
            if (userF.active) { userF.active = false; passF.active = true; }
            else { passF.active = false; userF.active = true; }
        }

        bool clicked = BtnUpdate(loginBtn, dt);
        bool enter = IsKeyPressed(KEY_ENTER) && (userF.active || passF.active);

        if ((clicked || enter) && !loggedIn) {
            if (userF.buf == "admin" && passF.buf == "1234") {
                loggedIn = true;
            }
            else {
                msg = "Невалидни данни  ·  опитай admin / 1234";
                msgCol = C_RED; msgAlpha = 1.f; msgTimer = 3.5f;
            }
        }
        if (msgTimer > 0) { msgTimer -= dt; msgAlpha = std::min(1.f, msgTimer / 0.4f); }
        if (loggedIn) {
            loginAnim = std::min(1.f, loginAnim + dt * 1.8f);
            if (loginAnim >= 1.f) { RunMovieScreen(); return; } 
        }

        BeginDrawing();
        ClearBackground(C_BG);

        DrawParticles();
        DrawFilm(0, 0, SH, t, false);
        DrawFilm(SW - 28, 0, SH, t, true);

        DrawRectangle(38, 24, SW - 76, 1, CA(C_GOLDDIM, 0.45f));
        DrawRectangle(38, SH - 26, SW - 76, 1, CA(C_GOLDDIM, 0.45f));
        DrawTextEx(fUI, "MOVIX", { 50, 12 }, 11, 3, CA(C_GOLD, 0.45f));
        DrawTextEx(fUI, "v2.0", { (float)(SW - 80), 12 }, 11, 2, CA(C_GREY, 0.40f));

        for (int i = 6; i >= 1; i--) {
            float e = (float)i * 8.f;
            DrawRectangle((int)(panelX - e), (int)(panelY - e),
                (int)(panelW + e * 2), (int)(panelH + e * 2), CA(C_GOLD, 0.012f));
        }
        DrawRectangleRec({ panelX, panelY, panelW, panelH }, C_PANEL);

        float cs = 18.f;
        auto corner = [&](float x, float y, float dx, float dy) {
            DrawLineEx({ x,y }, { x + dx * cs,y }, 1.5f, CA(C_GOLD, 0.7f));
            DrawLineEx({ x,y }, { x,y + dy * cs }, 1.5f, CA(C_GOLD, 0.7f));
            };
        corner(panelX, panelY, 1, 1);
        corner(panelX + panelW, panelY, -1, 1);
        corner(panelX, panelY + panelH, 1, -1);
        corner(panelX + panelW, panelY + panelH, -1, -1);
        DrawRectangleLinesEx({ panelX,panelY,panelW,panelH }, 0.8f, CA(C_GOLDDIM, 0.3f));

        float pulse = sinf(t * 2.f) * 0.3f + 0.7f;
        DrawRectangleGradientH((int)panelX, (int)panelY, (int)(panelW / 2), 3,
            CA(C_BG, 0), CA(C_GOLD, pulse));
        DrawRectangleGradientH((int)(panelX + panelW / 2), (int)panelY, (int)(panelW / 2), 3,
            CA(C_GOLD, pulse), CA(C_BG, 0));

        DrawClapper(cx, panelY + 20, t);
        DrawLogo(cx, panelY + 80, t);
        FieldDraw(userF);
        FieldDraw(passF);
        BtnDraw(loginBtn);

        if (msgAlpha > 0.01f) {
            Vector2 ms = MeasureTextEx(fUI, msg.c_str(), 13, 1);
            DrawTextEx(fUI, msg.c_str(),
                { cx - ms.x * 0.5f, panelY + 497 }, 13, 1, CA(msgCol, msgAlpha));
        }
        else {
            DrawTextC(fUI, "TAB — смени поле  ·  ENTER — влез",
                cx, panelY + 497, 11, 1, CA(C_GREY, 0.45f));
        }

        DrawScanlines();

        if (loggedIn) {
            float a = loginAnim;
            DrawRectangle(0, 0, SW, SH, CA(C_BG, a * 0.92f));
            if (a > 0.4f) {
                float fa = (a - 0.4f) / 0.6f;
                DrawGlowRect({ cx - 160, SH * 0.5f - 60, 320, 120 }, C_GOLD, fa * 0.8f, 8);
                DrawRectangle((int)(cx - 160), (int)(SH * 0.5f - 60), 320, 120, CA(C_PANEL, fa));
                DrawRectangleGradientH((int)(cx - 160), (int)(SH * 0.5f - 60), 160, 3,
                    CA(C_BG, 0), CA(C_GOLD, fa));
                DrawRectangleGradientH((int)cx, (int)(SH * 0.5f - 60), 160, 3,
                    CA(C_GOLD, fa), CA(C_BG, 0));
                DrawTextC(fTitle, "ДОБРЕ ДОШЪЛ", cx, SH * 0.5f - 50, 28, 4, CA(C_GOLD2, fa));
                DrawTextC(fUI, "Movix Cinema Suite", cx, SH * 0.5f + 2, 16, 2, CA(C_WHITE, fa * 0.8f));
                DrawTextC(fUI, "Влизането е успешно", cx, SH * 0.5f + 28, 12, 1, CA(C_GREY, fa * 0.6f));
            }
        }

        DrawTextC(fUI, "© 2025 Movix  –  Cinema Management Suite",
            cx, SH - 18, 10, 1, CA(C_GREY, 0.35f));

        EndDrawing();
    }
}