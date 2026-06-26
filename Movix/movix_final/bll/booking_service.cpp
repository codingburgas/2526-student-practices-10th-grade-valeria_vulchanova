#include "booking_service.h"
std::set<int> BookSvc_OccupiedSeats(int mi){ return ResRepo_Occupied(mi,HALL_COLS); }
bool BookSvc_Reserve(int mi,int row,int col,const std::string& u){ return ResRepo_Add(mi,row,col,u); }
bool BookSvc_Cancel (int mi,int row,int col,const std::string& u){ return ResRepo_Cancel(mi,row,col,u); }
std::vector<Reservation> BookSvc_UserHistory(const std::string& u){ return ResRepo_ForUser(u); }
int   BookSvc_CountForMovie(int mi){ return ResRepo_CountMovie(mi); }
float BookSvc_SeatPrice(int row,float base){
    if(row<=1) return base*0.80f;
    if(row>=6) return base*1.25f;
    return base;
}
const char* BookSvc_RowTier(int row){
    if(row<=1) return "FRONT";
    if(row>=6) return "VIP";
    return "STD";
}
