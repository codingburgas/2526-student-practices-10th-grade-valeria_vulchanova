#pragma once
#include "types.h"
#include <vector>
#include <set>
std::vector<Reservation> ResRepo_LoadAll();
bool          ResRepo_Add        (int movieIdx,int row,int col,const std::string& uname);
std::set<int> ResRepo_Occupied   (int movieIdx,int cols);
int           ResRepo_CountMovie (int movieIdx);
bool          ResRepo_ClearMovie (int movieIdx);
bool          ResRepo_ClearAll   ();
bool          ResRepo_Cancel     (int movieIdx,int row,int col,const std::string& uname);
std::vector<Reservation> ResRepo_ForUser(const std::string& uname);
