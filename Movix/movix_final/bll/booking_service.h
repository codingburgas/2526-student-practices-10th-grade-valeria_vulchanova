#pragma once
#include "../dal/reservation_repo.h"
#include <string>
#include <set>
#include <vector>

static const int HALL_ROWS=8, HALL_COLS=10;

std::set<int> BookSvc_OccupiedSeats(int movieIdx);
bool BookSvc_Reserve(int movieIdx,int row,int col,const std::string& uname);
bool BookSvc_Cancel (int movieIdx,int row,int col,const std::string& uname);
std::vector<Reservation> BookSvc_UserHistory(const std::string& uname);
int   BookSvc_CountForMovie(int movieIdx);
float BookSvc_SeatPrice(int row,float basePrice);
const char* BookSvc_RowTier(int row);
