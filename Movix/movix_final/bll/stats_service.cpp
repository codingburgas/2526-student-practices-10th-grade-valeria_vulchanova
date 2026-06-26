#include "stats_service.h"
#include "../dal/user_repo.h"
#include "../dal/movie_repo.h"
#include "../dal/reservation_repo.h"
#include "booking_service.h"
AppStats StatsSvc_Compute(){
    AppStats s;
    s.totalUsers  =(int)UserRepo_LoadAll().size()+1; // +1 for admin
    s.totalMovies = MovieRepo_Count();
    auto allRes   = ResRepo_LoadAll();
    s.totalBookings=(int)allRes.size();
    s.totalRevenue=0;
    for(int i=0;i<MovieRepo_Count();i++){
        const auto& e=MovieRepo_Get(i);
        MovieStat ms; ms.movieId=e.id;
        ms.title=e.title; ms.bookings=0; ms.revenue=0;
        for(auto& r:allRes) if(r.movieIdx==i){
            ms.bookings++;
            ms.revenue+=BookSvc_SeatPrice(r.row,e.price);
        }
        s.totalRevenue+=ms.revenue;
        s.byMovie.push_back(ms);
    }
    return s;
}
