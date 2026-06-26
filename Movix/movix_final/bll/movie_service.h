#pragma once
#include "../dal/movie_repo.h"
#include "../dal/rating_repo.h"
#include <string>
#include <vector>

struct MovieView {
    int         idx;
    MovieEntry  entry;
    float       userRating;  // current user's rating (0=unrated)
    float       avgRating;   // community average
    bool        isFavorite;
};

void   MovieSvc_Init();
int    MovieSvc_Count();
MovieView MovieSvc_Get(int idx, const std::string& uname);

// Returns filtered indices
std::vector<int> MovieSvc_Search(const std::string& query, const std::string& genre);
std::vector<std::string> MovieSvc_AllGenres();

bool MovieSvc_Add   (MovieEntry e);
bool MovieSvc_Delete(int idx);

// Poster management
void MovieSvc_SetPoster(int movieId, const std::string& srcFilePath);
bool MovieSvc_HasPoster(int movieId);

// Rating
bool MovieSvc_Rate(const std::string& uname, int movieId, int stars);
