#include "seat.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdio>

static const int ROWS = 8;
static const int COLS_SEATS = 10;
static const int TOTAL_SEATS = ROWS * COLS_SEATS;

// Simulate some pre-taken seats (could be loaded from DB later)
static bool s_taken[TOTAL_SEATS] = {
    0,1,0,0,1,1,0,0,1,0,
    0,0,1,0,0,0,1,0,0,0,
    1,0,0,0,1,0,0,1,0,0,
    0,0,0,1,0,0,0,0,1,0,
    0,1,0,0,0,1,0,0,0,1,
    0,0,0,0,1,0,1,0,0,0,
    1,0,0,1,0,0,0,0,1,0,
    0,0,1,0,0,0,0,1,0,0,
};

bool RunSeatScreen(const Movie& mv) {
    // Layout
    const float SEAT_W = 38.f;
    const float SEAT_H = 32.f;
    const float SEAT_GAP = 10.f;
    const float AISLE_W = 22.f;   // extra gap after col 4
    const float TOTAL_W = COLS_SEATS * (SEAT_W + SEAT_GAP) + AISLE_W - SEAT_GAP;
    const float START_X = (SW - TOTAL_W) * 0.5f;
    const float START_Y = 180.f;
    const float ROW_H = SEAT_H + SEAT_GAP;
    const float SCREEN_W = TOTAL_W * 0.72f;
    const float SCREEN_X = START_X + (TOTAL_W - SCREEN_W) * 0.5f;

    // Row labels
    const char* ROW_LABELS = "ABCDEFGH";

    bool selected[TOTAL_SEATS] = {};
    float hoverAnims[TOTAL_SEATS] = {};
    float fadeIn = 0.f;
    float confirmT = 0.f;
    int  selectedCount = 0;
    bool confirmed = false;
    bool wentBack = false;

    Button confirmBtn, backBtn;
    BtnInit(confirmBtn, { SW * 0.5f - 140.f, SH - 64.f, 280.f, 48.f }, "CONFIRM BOOKING");
    BtnInit(backBtn, { 30.f,               SH - 64.f, 130.f, 48.f }, "< BACK");

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float t = GetTime();
        fadeIn = std::min(1.f, fadeIn + dt * 2.f);

        UpdateParticles(dt);
        if (confirmT > 0.f) confirmT -= dt;

        Vector2 mouse = GetMousePosition();

        // Update seat hover/click
        selectedCount = 0;
        for (int i = 0; i < TOTAL_SEATS; i++) {
            if (selected[i]) selectedCount++;

            int c = i % COLS_SEATS;
            int r = i / COLS_SEATS;

            float aisle = (c >= 5) ? AISLE_W : 0.f;
            float sx = START_X + c * (SEAT_W + SEAT_GAP) + aisle;
            float sy = START_Y + r * ROW_H;

            Rectangle sr = { sx, sy, SEAT_W, SEAT_H };
            bool hover = !s_taken[i] && CheckCollisionPointRec(mouse, sr);

            hoverAnims[i] += dt * (hover ? 9.f : -9.f);
            hoverAnims[i] = std::clamp(hoverAnims[i], 0.f, 1.f);

            if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && confirmT <= 0.f) {
                selected[i] = !selected[i];
            }
        }

        // Buttons
        if (BtnUpdate(confirmBtn, dt) && selectedCount > 0 && confirmT <= 0.f) {
            confirmT = 3.f;
            confirmed = true;
        }
        if (BtnUpdate(backBtn, dt)) {
            wentBack = true;
        }

        if (confirmed && confirmT <= 0.f) return true;
        if (wentBack) return false;

        // ── Draw ────────────────────────────────────────────────────
        BeginDrawing();
        ClearBackground(C_BG);
        DrawParticles();
        DrawFilm(0, 0, SH, t, false);
        DrawFilm(SW - 28, 0, SH, t, true);

        // Header bar
        DrawRectangle(28, 0, SW - 56, 80, CA(C_PANEL, 0.97f));
        DrawRectangle(28, 78, SW - 56, 2, CA(C_GOLDDIM, 0.5f));
        DrawTextC(fTitle, "MOVIX", SW * 0.5f, 12, 38, 4, C_GOLD2);

        char hdrBuf[128];
        snprintf(hdrBuf, sizeof(hdrBuf), "SELECT SEATS for  %s", mv.title);
        DrawTextEx(fUI, hdrBuf, { 48, 28 }, 12, 3, CA(C_GOLD, 0.6f));

        // ── Screen arc ──────────────────────────────────────────────
        float scY = START_Y - 50.f;
        DrawRectangleRounded({ SCREEN_X, scY, SCREEN_W, 14.f }, 0.5f, 6,
            CA(mv.accent, 0.35f));
        DrawRectangleRoundedLinesEx({ SCREEN_X, scY, SCREEN_W, 14.f }, 0.5f, 6, 1.5f,
            CA(mv.accent, 0.75f));
        float sw2 = MeasureTextEx(fUI, "SCREEN", 10.f, 2).x;
        DrawTextEx(fUI, "SCREEN",
            { SCREEN_X + (SCREEN_W - sw2) * 0.5f, scY + 1.f },
            10.f, 2, CA(C_BG, 0.9f));

        // ── Seats ───────────────────────────────────────────────────
        for (int i = 0; i < TOTAL_SEATS; i++) {
            int c = i % COLS_SEATS;
            int r = i / COLS_SEATS;

            float aisle = (c >= 5) ? AISLE_W : 0.f;
            float sx = START_X + c * (SEAT_W + SEAT_GAP) + aisle;
            float sy = START_Y + r * ROW_H;
            float hv = hoverAnims[i];

            Color fillCol, borderCol;
            if (s_taken[i]) {
                fillCol = CA(C_GREY, 0.18f);
                borderCol = CA(C_GREY, 0.30f);
            }
            else if (selected[i]) {
                fillCol = CA(mv.accent, 0.55f + hv * 0.2f);
                borderCol = mv.accent;
            }
            else {
                fillCol = CA(C_PANEL, 0.55f + hv * 0.25f);
                borderCol = CA(C_GOLDDIM, 0.35f + hv * 0.45f);
            }

            // Seat body
            DrawRectangleRounded({ sx, sy + 6.f, SEAT_W, SEAT_H - 6.f },
                0.28f, 4, fillCol);
            // Seat back
            DrawRectangleRounded({ sx + 4.f, sy, SEAT_W - 8.f, 10.f },
                0.4f, 4, CA(borderCol, 0.85f));
            // Border
            DrawRectangleRoundedLinesEx({ sx, sy + 6.f, SEAT_W, SEAT_H - 6.f },
                0.28f, 4, 1.2f, borderCol);

            // Seat number label
            if (!s_taken[i]) {
                char lbl[4];
                snprintf(lbl, sizeof(lbl), "%c%d", ROW_LABELS[r], c + 1);
                float lw = MeasureTextEx(fUI, lbl, 8.f, 1).x;
                DrawTextEx(fUI, lbl,
                    { sx + (SEAT_W - lw) * 0.5f, sy + SEAT_H * 0.35f },
                    8.f, 1,
                    selected[i] ? C_BG : CA(C_GREYLT, 0.65f));
            }
            else {
                // X for taken
                float cx2 = sx + SEAT_W * 0.5f;
                float cy2 = sy + SEAT_H * 0.6f;
                DrawLine((int)(cx2 - 5), (int)(cy2 - 5), (int)(cx2 + 5), (int)(cy2 + 5), CA(C_GREY, 0.5f));
                DrawLine((int)(cx2 + 5), (int)(cy2 - 5), (int)(cx2 - 5), (int)(cy2 + 5), CA(C_GREY, 0.5f));
            }
        }

        // Row labels on left
        for (int r = 0; r < ROWS; r++) {
            float ry = START_Y + r * ROW_H + SEAT_H * 0.3f;
            char rl[2] = { ROW_LABELS[r], 0 };
            DrawTextEx(fUIBold, rl, { START_X - 22.f, ry }, 13.f, 1,
                CA(C_GOLD, 0.55f));
        }

        // ── Legend ──────────────────────────────────────────────────
        float legY = START_Y + ROWS * ROW_H + 18.f;
        float legX = START_X;

        auto LegItem = [&](float x, Color fc, Color bc, const char* lbl) {
            DrawRectangleRounded({ x, legY, 22.f, 16.f }, 0.3f, 4, fc);
            DrawRectangleRoundedLinesEx({ x, legY, 22.f, 16.f }, 0.3f, 4, 1.f, bc);
            DrawTextEx(fUI, lbl, { x + 28.f, legY + 2.f }, 10.f, 1, CA(C_GREYLT, 0.75f));
            };

        LegItem(legX, CA(C_PANEL, 0.55f), CA(C_GOLDDIM, 0.5f), "Available");
        LegItem(legX + 130.f, CA(mv.accent, 0.55f), mv.accent, "Selected");
        LegItem(legX + 260.f, CA(C_GREY, 0.18f), CA(C_GREY, 0.35f), "Taken");

        // ── Info panel ──────────────────────────────────────────────
        float infoX = START_X + TOTAL_W + 28.f;
        float infoW = SW - infoX - 28.f;
        if (infoW > 60.f) {
            float iy = START_Y;
            DrawTextEx(fUIBold, mv.title, { infoX, iy }, 13.f, 2, C_GOLD2);     iy += 20.f;
            DrawTextEx(fUI, mv.genre, { infoX, iy }, 10.f, 1, CA(C_GREYLT, 0.7f)); iy += 16.f;
            DrawTextEx(fUI, mv.duration, { infoX, iy }, 10.f, 1, CA(C_GREYLT, 0.6f)); iy += 24.f;

            char selBuf[40];
            snprintf(selBuf, sizeof(selBuf), "Seats chosen: %d", selectedCount);
            DrawTextEx(fUIBold, selBuf, { infoX, iy }, 11.f, 1,
                selectedCount ? C_GOLD2 : CA(C_GREY, 0.5f));     iy += 18.f;

            if (selectedCount > 0) {
                float total = selectedCount * mv.price;
                char totBuf[40];
                snprintf(totBuf, sizeof(totBuf), "Total: %.2f BGN", total);
                DrawRectangle((int)infoX, (int)iy, (int)infoW, 28, CA(C_GOLD, 0.10f));
                DrawRectangleLinesEx({ infoX, iy, (float)infoW, 28.f }, 1.f, CA(C_GOLD, 0.35f));
                DrawTextEx(fUIBold, totBuf, { infoX + 8.f, iy + 7.f }, 12.f, 2, C_GOLD2);
                iy += 36.f;

                // List chosen seats
                DrawTextEx(fUI, "Your seats:", { infoX, iy }, 9.5f, 1, CA(C_GOLDDIM, 0.65f));
                iy += 14.f;
                for (int i = 0; i < TOTAL_SEATS && iy < SH - 80.f; i++) {
                    if (!selected[i]) continue;
                    char sb[8];
                    snprintf(sb, sizeof(sb), "%c%d",
                        ROW_LABELS[i / COLS_SEATS], i % COLS_SEATS + 1);
                    DrawRectangle((int)infoX, (int)iy, 32, 16, CA(mv.accent, 0.3f));
                    DrawRectangleLinesEx({ infoX, iy, 32.f, 16.f }, 0.8f, CA(mv.accent, 0.7f));
                    DrawTextEx(fUI, sb, { infoX + 4.f, iy + 3.f }, 9.f, 1, C_WHITE);
                    iy += 20.f;
                }
            }
        }

        // ── Buttons ─────────────────────────────────────────────────
        BtnDraw(backBtn);
        if (selectedCount > 0) BtnDraw(confirmBtn);

        // ── Confirm toast ────────────────────────────────────────────
        if (confirmT > 0.f) {
            float ca2 = std::min(1.f, confirmT / 0.35f) * std::min(1.f, confirmT);
            float tw = SW * 0.55f, th = 56.f;
            float tx = (SW - tw) * 0.5f, ty = SH * 0.45f;

            DrawRectangle((int)tx, (int)ty, (int)tw, (int)th, CA(C_PANEL, ca2 * 0.97f));
            DrawRectangleLinesEx({ tx, ty, tw, th }, 1.5f, CA(mv.accent, ca2));

            char conf[100];
            snprintf(conf, sizeof(conf),
                " %d seat(s) for \"%s\" booked!", selectedCount, mv.title);
            float cw2 = MeasureTextEx(fUIBold, conf, 11.f, 1).x;
            DrawTextEx(fUIBold, conf,
                { tx + (tw - cw2) * 0.5f, ty + 18.f }, 11.f, 1,
                CA(mv.accent, ca2));
        }

        DrawScanlines();
        if (fadeIn < 1.f)
            DrawRectangle(0, 0, SW, SH, CA(C_BG, 1.f - fadeIn));
        DrawTextC(fUI, "© 2025 Movix – Cinema Management Suite",
            SW * 0.5f, SH - 14.f, 10.f, 1, CA(C_GREY, 0.30f));

        EndDrawing();
    }
    return false;
}