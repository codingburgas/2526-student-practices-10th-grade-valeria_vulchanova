#pragma once
#include <string>
#include <vector>

// ════════════════════════════════════════════════════════════════
//  DATA ACCESS LAYER  –  Shared data types
//  No dependencies on BLL or UI.
// ════════════════════════════════════════════════════════════════

struct UserRecord {
    std::string username;
    std::string password;
    bool        isAdmin  = false;
};

// Stored in movies.txt
struct MovieEntry {
    char  title   [128] = {};
    char  genre   [ 64] = {};
    char  director[128] = {};
    char  cast    [256] = {};
    char  duration[ 32] = {};
    char  year    [  8] = {};
    float rating        = 7.5f;
    float price         = 12.0f;
    char  desc    [512] = {};
    // Poster colours (RGB, serialised as "R,G,B")
    int   pA_r=20,  pA_g=12,  pA_b=45;
    int   pB_r=80,  pB_g=45,  pB_b=10;
    int   ac_r=212, ac_g=163, ac_b=57;
    int   id        = 0;
};

struct Reservation {
    int         movieIdx;
    int         row;
    int         col;
    std::string username;
};

struct Rating {
    std::string username;
    int         movieId;
    int         stars;     // 1-5
};

struct FavoriteEntry {
    std::string username;
    int         movieId;
};
