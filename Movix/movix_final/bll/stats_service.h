#pragma once
#include <string>
#include <vector>
struct MovieStat { int movieId; std::string title; int bookings; float revenue; };
struct AppStats  { int totalUsers,totalMovies,totalBookings; float totalRevenue; std::vector<MovieStat> byMovie; };
AppStats StatsSvc_Compute();
