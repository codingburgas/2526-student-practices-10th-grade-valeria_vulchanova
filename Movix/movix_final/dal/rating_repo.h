#pragma once
#include "types.h"
#include <vector>
std::vector<Rating> RatingRepo_LoadAll();
bool RatingRepo_Set   (const std::string& uname,int movieId,int stars); // upsert
int  RatingRepo_Get   (const std::string& uname,int movieId); // 0=not rated
float RatingRepo_Avg  (int movieId);  // average over all users (0=none)
bool RatingRepo_Delete(const std::string& uname,int movieId);
