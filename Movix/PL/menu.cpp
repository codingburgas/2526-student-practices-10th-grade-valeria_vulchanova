#include "menu.h"
#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>


static const Movie MOVIES[] = {
    {
        "DUNE: PART TWO",
        "Sci-Fi / Epic",
        "Denis Villeneuve",
        "Timothée Chalamet, Zendaya, Austin Butler",
        "2ч 46мин", "2024", 8.5f, 14.00f,
        "Paul Atreides teams up with Chani and the Fremen as he seeks revenge against the conspirators who destroyed his family.",
        {  20,  12,  45, 255 }, {  80,  45,  10, 255 }, { 212, 163,  57, 255 }
    },
    {
        "OPPENHEIMER",
        "Biographical / Drama",
        "Christopher Nolan",
        "Cillian Murphy, Emily Blunt, Robert Downey Jr.",
        "3ч 00мин", "2023", 8.9f, 14.00f,
        "The story of J. Robert Oppenheimer and his role in the development of the atomic bomb during World War II.",
        {  10,  10,  35, 255 }, {  45,  20,  60, 255 }, { 255, 140,  40, 255 }
    },
    {
        "THE BATMAN",
        "Action / Thriller",
        "Matt Reeves",
        "Robert Pattinson, Zoë Kravitz, Paul Dano",
        "2ч 56мин", "2022", 7.9f, 12.00f,
        "When a series of brutal murders shakes Gotham, Batman must decipher the sinister riddles of the Riddler.",
        {   8,  10,  22, 255 }, {  30,  10,  10, 255 }, {  80, 160, 220, 255 }
    },
    {
        "AVATAR: THE WAY OF WATER",
        "Sci-Fi / Adventure",
        "James Cameron",
        "Sam Worthington, Zoe Saldaña, Sigourney Weaver",
        "3ч 12мин", "2022", 7.6f, 16.00f,
        "Jake Sully and Neytiri must leave their home and explore the regions of Pandora to protect their family.",
        {   5,  30,  55, 255 }, {   8,  60,  80, 255 }, {  40, 200, 200, 255 }
    },
    {
        "TOP GUN: MAVERICK",
        "Action / Drama",
        "Joseph Kosinski",
        "Tom Cruise, Miles Teller, Jennifer Connelly",
        "2ч 10мин", "2022", 8.3f, 12.00f,
        "After more than 30 years of service, Maverick is called to train a group of pilots for a special mission.",
        {  10,  20,  50, 255 }, {  50,  10,  10, 255 }, { 180, 200, 255, 255 }
    },
    {
        "INTERSTELLAR",
        "Sci-Fi / Drama",
        "Christopher Nolan",
        "Matthew McConaughey, Anne Hathaway, Jessica Chastain",
        "2ч 49мин", "2014", 8.7f, 10.00f,
        "An exploration team travels through a wormhole in space in an attempt to ensure humanity's survival.",
        {   5,   8,  25, 255 }, {  25,  18,  50, 255 }, { 212, 163,  57, 255 }
    },
    {
        "INCEPTION",
        "Sci-Fi / Thriller",
        "Christopher Nolan",
        "Leonardo DiCaprio, Joseph Gordon-Levitt, Elliot Page",
        "2ч 28мин", "2010", 8.8f, 10.00f,
        "A thief who steals corporate secrets through dreams is tasked with planting an idea into a CEO's mind.",
        {  15,  10,  30, 255 }, {  40,  25,  55, 255 }, { 180, 120, 255, 255 }
    },
    {
        "SPIDER-MAN: NO WAY HOME",
        "Action / Sci-Fi",
        "Jon Watts",
        "Tom Holland, Zendaya, Benedict Cumberbatch",
        "2ч 28мин", "2021", 8.3f, 13.00f,
        "Peter Parker asks Doctor Strange to erase knowledge of his identity, which opens the multiverse.",
        {  35,   8,   8, 255 }, {   8,   8,  45, 255 }, { 220,  60,  60, 255 }
    },
};

static const int MOVIE_COUNT = (int)(sizeof(MOVIES) / sizeof(MOVIES[0]));

// ── Layout constants ─────────────────────────────────────────────
static const float CARD_W = 195.f;
static const float CARD_H = 275.f;
static const float CARD_GAP = 22.f;
static const float COLS = 3.f;
static const float GRID_X = 30.f;
static const float GRID_TOP = 95.f;
static const float DETAIL_X = GRID_X + COLS * (CARD_W + CARD_GAP) + 20.f;
static const float DETAIL_W = SW - DETAIL_X - 22.f;

// ── Per-card animation state ─────────────────────────────────────
struct CardAnim {
    float hoverT = 0.f;
    float selectT = 0.f;
};

// ── Helper: draw star rating ─────────────────────────────────────
static void DrawStars(float x, float y, float rating, float sz, Color col) {
    float stars = rating / 2.f;
    for (int i = 0; i < 5; i++) {
        float filled = std::clamp(stars - (float)i, 0.f, 1.f);
        Color c = CA(col, 0.25f + filled * 0.75f);
        float cx2 = x + (float)i * (sz + 3.f) + sz * 0.5f;
        float cy2 = y + sz * 0.5f;
        float outer = sz * 0.5f;
        float inner = outer * 0.4f;

        for (int p = 0; p < 5; p++) {
            float a0 = -1.5707963f + (float)p * 1.2566370f;
            float a1 = a0 + 0.6283185f;
            float am = a0 + 0.3141592f;

            Vector2 v0 = { cx2 + cosf(a0) * outer, cy2 + sinf(a0) * outer };
            Vector2 vm = { cx2 + cosf(am) * inner, cy2 + sinf(am) * inner };
            Vector2 v1 = { cx2 + cosf(a1) * outer, cy2 + sinf(a1) * outer };

            DrawTriangle(v0, { cx2, cy2 }, vm, CA(col, filled * 0.9f));
            DrawTriangle(vm, { cx2, cy2 }, v1, CA(col, filled * 0.9f));
        }

        if (filled < 1.f)
            DrawCircleLines((int)cx2, (int)cy2, outer * 0.85f, CA(col, 0.25f));
    }
}

// ── Helper: draw movie poster art ────────────────────────────────
static void DrawPoster(float x, float y, float w, float h, const Movie& m, float t) {
    DrawRectangleGradientV((int)x, (int)y, (int)w, (int)h, m.posterA, m.posterB);

    float pulse = sinf(t * 1.4f) * 0.5f + 0.5f;
    DrawCircle((int)(x + w * 0.75f), (int)(y + h * 0.28f),
        (int)(w * 0.38f), CA(m.accent, 0.08f + pulse * 0.04f));

    DrawCircleLines((int)(x + w * 0.75f), (int)(y + h * 0.28f),
        (int)(w * 0.38f), CA(m.accent, 0.18f));

    DrawCircleLines((int)(x + w * 0.75f), (int)(y + h * 0.28f),
        (int)(w * 0.22f), CA(m.accent, 0.12f));

    for (int sy = (int)y; sy < (int)(y + h); sy += 4)
        DrawLine((int)x, sy, (int)(x + w), sy, CA(C_BG, 0.10f));

    for (int g = 0; g < 18; g++) {
        float gx = x + fmodf((float)(g * 137 + (int)(t * 7)), w);
        float gy = y + fmodf((float)(g * 89 + (int)(t * 5)), h);
        DrawPixel((int)gx, (int)gy, CA(C_WHITE, 0.08f));
    }

    DrawRectangleGradientV((int)x, (int)(y + h * 0.62f),
        (int)w, (int)(h * 0.38f), CA(C_BG, 0.f), CA(C_BG, 0.88f));

    DrawRectangle((int)(x + 8), (int)(y + 10), 60, 14, CA(m.accent, 0.25f));
    DrawRectangleLinesEx({ x + 8, y + 10, 60, 14 }, 0.8f, CA(m.accent, 0.55f));

    char priceBuf[16];
    snprintf(priceBuf, sizeof(priceBuf), "%.0f BGN", m.price);

    DrawRectangle((int)(x + w - 48), (int)(y + 8), 44, 18, CA(C_GOLD, 0.9f));
    DrawTextEx(fUIBold, priceBuf,
        { x + w - 44, y + 12 }, 9.f, 1, C_BG);
}

// ── Helper: wrap text ────────────────────────────────────────────
static std::vector<std::string> WrapText(const std::string& text,
    Font f, float sz, float sp, float maxW)
{
    std::vector<std::string> lines;
    std::istringstream ss(text);
    std::string word, line;

    while (ss >> word) {
        std::string test = line.empty() ? word : line + " " + word;
        if (MeasureTextEx(f, test.c_str(), sz, sp).x > maxW && !line.empty()) {
            lines.push_back(line);
            line = word;
        }
        else {
            line = test;
        }
    }

    if (!line.empty()) lines.push_back(line);
    return lines;
}


void RunMovieScreen() {
    float fadeIn = 0.f;
    CardAnim anims[MOVIE_COUNT] = {};
    int selected = 0;
    float scrollY = 0.f;
    float scrollTarget = 0.f;

    Button bookBtn;
    BtnInit(bookBtn, { DETAIL_X, SH - 76.f, DETAIL_W, 52.f }, "BOOK TICKET");

    float confirmT = 0.f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float t = GetTime();

        fadeIn = std::min(1.f, fadeIn + dt * 1.6f);

        float wheel = GetMouseWheelMove();
        scrollTarget -= wheel * 55.f;

        float maxScroll = std::max(0.f,
            std::ceilf((float)MOVIE_COUNT / COLS) * (CARD_H + CARD_GAP) - (SH - GRID_TOP - 20.f));

        scrollTarget = std::clamp(scrollTarget, 0.f, maxScroll);
        scrollY += (scrollTarget - scrollY) * std::min(1.f, dt * 12.f);

        UpdateParticles(dt);

        for (int i = 0; i < MOVIE_COUNT; i++) {
            int col = i % (int)COLS;
            int row = i / (int)COLS;

            float cx2 = GRID_X + (float)col * (CARD_W + CARD_GAP);
            float cy2 = GRID_TOP + (float)row * (CARD_H + CARD_GAP) - scrollY;

            Rectangle r = { cx2, cy2, CARD_W, CARD_H };

            bool inGrid = cy2 > GRID_TOP - CARD_H && cy2 < SH;
            bool hover = inGrid && CheckCollisionPointRec(GetMousePosition(), r);

            anims[i].hoverT += dt * (hover ? 8.f : -8.f);
            anims[i].hoverT = std::clamp(anims[i].hoverT, 0.f, 1.f);

            anims[i].selectT += dt * ((selected == i) ? 6.f : -6.f);
            anims[i].selectT = std::clamp(anims[i].selectT, 0.f, 1.f);

            if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                selected = i;
        }

        bool booked = BtnUpdate(bookBtn, dt);
        if (booked) confirmT = 3.f;
        if (confirmT > 0.f) confirmT -= dt;

        BeginDrawing();
        ClearBackground(C_BG);

        DrawParticles();
        DrawFilm(0, 0, SH, t, false);
        DrawFilm(SW - 28, 0, SH, t, true);

        DrawRectangle(28, 0, SW - 56, 80, CA(C_PANEL, 0.97f));
        DrawRectangle(28, 78, SW - 56, 2, CA(C_GOLDDIM, 0.5f));

        DrawTextC(fTitle, "MOVIX", SW * 0.5f, 12, 38, 4, C_GOLD2);
        DrawTextEx(fUI, "SELECT A MOVIE", { 48, 28 }, 12, 3, CA(C_GOLD, 0.6f));

        {
            char cnt[32];
            snprintf(cnt, sizeof(cnt), "%d MOVIES IN PROGRAM", MOVIE_COUNT);
            DrawTextEx(fUI, cnt,
                { (float)(SW - 220), 28 }, 11, 2, CA(C_GREY, 0.5f));
        }

        BeginScissorMode((int)GRID_X - 4, (int)GRID_TOP,
            (int)(COLS * (CARD_W + CARD_GAP) + 8), SH - (int)GRID_TOP);

        for (int i = 0; i < MOVIE_COUNT; i++) {
            int col = i % (int)COLS;
            int row = i / (int)COLS;

            float cx2 = GRID_X + (float)col * (CARD_W + CARD_GAP);
            float cy2 = GRID_TOP + (float)row * (CARD_H + CARD_GAP) - scrollY;

            if (cy2 + CARD_H < GRID_TOP || cy2 > SH) continue;

            float hov = anims[i].hoverT;
            float sel = anims[i].selectT;
            bool isSel = (selected == i);

            float lift = hov * 6.f + sel * 4.f;
            float cardY = cy2 - lift;

            DrawRectangle((int)(cx2 + 4), (int)(cardY + 6),
                (int)CARD_W, (int)CARD_H, CA(C_BG, 0.6f));

            DrawPoster(cx2, cardY, CARD_W, CARD_H * 0.64f, MOVIES[i], t);

            DrawRectangle((int)cx2, (int)(cardY + CARD_H * 0.64f),
                (int)CARD_W, (int)(CARD_H * 0.36f), CA(C_PANEL, 0.98f));

            DrawRectangleLinesEx({ cx2, cardY, CARD_W, CARD_H }, isSel ? 1.8f : 0.9f,
                CA(C_GOLDDIM, 0.3f + hov * 0.4f));

            float bx = cx2 + 10.f;
            float by = cardY + CARD_H * 0.64f + 8.f;

            DrawTextEx(fUIBold, MOVIES[i].title,
                { bx, by }, 10.5f, 1, C_WHITE);

            DrawTextEx(fUI, MOVIES[i].genre,
                { bx, by + 15.f }, 9.f, 1, CA(C_GREYLT, 0.7f));
        }

        EndScissorMode();

        const Movie& mv = MOVIES[selected];

        float dx = DETAIL_X;
        float dy = GRID_TOP + 10.f;
        float dw = DETAIL_W;

        float ph = 200.f;
        DrawPoster(dx, dy, dw, ph, mv, t);

        dy += ph + 16.f;
        DrawTextEx(fUIBold, mv.title, { dx, dy }, 16.f, 2, C_GOLD2);
        dy += 22.f;

        DrawTextEx(fUI, mv.genre, { dx, dy }, 11.f, 1, CA(C_GREYLT, 0.8f));

        dy += 18.f;

        DrawTextEx(fUIBold, "DIRECTOR", { dx, dy }, 9.5f, 2, CA(C_GOLD, 0.6f));
        DrawTextEx(fUI, mv.director, { dx + 90.f, dy }, 11.f, 1, C_WHITE);

        dy += 18.f;

        DrawTextEx(fUIBold, "CAST", { dx, dy }, 9.5f, 2, CA(C_GOLD, 0.6f));

        dy += 14.f;
        auto castLines = WrapText(mv.cast, fUI, 10.5f, 1, dw);

        for (auto& cl : castLines) {
            DrawTextEx(fUI, cl.c_str(), { dx + 10.f, dy }, 10.5f, 1, CA(C_GREYLT, 0.85f));
            dy += 14.f;
        }

        dy += 4.f;

        DrawTextEx(fUIBold, "SYNOPSIS", { dx, dy }, 9.5f, 2, CA(C_GOLD, 0.6f));

        dy += 14.f;
        auto descLines = WrapText(mv.desc, fUI, 10.5f, 1, dw);

        for (auto& dl : descLines) {
            DrawTextEx(fUI, dl.c_str(), { dx, dy }, 10.5f, 1, CA(C_GREYLT, 0.75f));
            dy += 14.f;
        }

        dy += 6.f;

        char priceFull[32];
        snprintf(priceFull, sizeof(priceFull), "TICKET PRICE: %.2f BGN", mv.price);

        DrawRectangle((int)dx, (int)dy, (int)dw, 30, CA(C_GOLD, 0.12f));
        DrawRectangleLinesEx({ dx, dy, dw, 30 }, 1.f, CA(C_GOLD, 0.35f));

        float pw = MeasureTextEx(fUIBold, priceFull, 13.f, 2).x;
        DrawTextEx(fUIBold, priceFull,
            { dx + (dw - pw) * 0.5f, dy + 8.f }, 13.f, 2, C_GOLD2);

        dy += 36.f;

        bookBtn.rect = { dx, (float)(SH - 72), dw, 48.f };
        BtnDraw(bookBtn);

        if (confirmT > 0.f) {
            float ca = std::min(1.f, confirmT / 0.35f) *
                std::min(1.f, confirmT);

            DrawRectangle((int)dx - 2, (int)(SH - 78), (int)dw + 4, 60,
                CA(C_PANEL, ca * 0.97f));

            DrawRectangleLinesEx({ dx - 2.f, (float)(SH - 78), dw + 4.f, 60.f },
                1.5f, CA(mv.accent, ca));

            char conf[80];
            snprintf(conf, sizeof(conf), "✓ Ticket for \"%s\" has been reserved!", mv.title);

            float cw = MeasureTextEx(fUIBold, conf, 11.f, 1).x;

            DrawTextEx(fUIBold, conf,
                { dx + (dw - cw) * 0.5f, (float)(SH - 62) }, 11.f, 1,
                CA(mv.accent, ca));
        }

        DrawScanlines();

        if (fadeIn < 1.f)
            DrawRectangle(0, 0, SW, SH, CA(C_BG, 1.f - fadeIn));

        DrawTextC(fUI, "© 2025 Movix – Cinema Management Suite",
            SW * 0.5f, SH - 14.f, 10.f, 1, CA(C_GREY, 0.30f));

        EndDrawing();
    }
}