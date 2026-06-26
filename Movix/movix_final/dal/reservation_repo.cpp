#include "reservation_repo.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
static const char* RF="data/reservations.txt";
static void ED(){system("if not exist data mkdir data 2>nul || mkdir -p data 2>/dev/null");}
std::vector<Reservation> ResRepo_LoadAll(){
    std::vector<Reservation> o; std::ifstream f(RF); if(!f) return o;
    std::string line;
    while(std::getline(f,line)){
        if(line.empty()) continue;
        std::istringstream ss(line); Reservation r; std::string a,b,c;
        if(!std::getline(ss,a,'|')||!std::getline(ss,b,'|')||
           !std::getline(ss,c,'|')||!std::getline(ss,r.username)) continue;
        try{r.movieIdx=std::stoi(a);r.row=std::stoi(b);r.col=std::stoi(c);}catch(...){continue;}
        o.push_back(r);
    } return o;
}
static void Save(const std::vector<Reservation>& v){
    ED(); std::ofstream f(RF);
    for(auto& r:v) f<<r.movieIdx<<"|"<<r.row<<"|"<<r.col<<"|"<<r.username<<"\n";
}
bool ResRepo_Add(int mi,int row,int col,const std::string& u){
    auto all=ResRepo_LoadAll();
    for(auto& r:all) if(r.movieIdx==mi&&r.row==row&&r.col==col) return false;
    all.push_back({mi,row,col,u}); Save(all); return true;
}
std::set<int> ResRepo_Occupied(int mi,int cols){
    std::set<int> o;
    for(auto& r:ResRepo_LoadAll()) if(r.movieIdx==mi) o.insert(r.row*cols+r.col);
    return o;
}
int ResRepo_CountMovie(int mi){int n=0;for(auto& r:ResRepo_LoadAll()) if(r.movieIdx==mi) n++;return n;}
bool ResRepo_ClearMovie(int mi){
    auto all=ResRepo_LoadAll(); size_t b=all.size();
    all.erase(std::remove_if(all.begin(),all.end(),[&](const Reservation& r){return r.movieIdx==mi;}),all.end());
    Save(all);return all.size()<b;
}
bool ResRepo_ClearAll(){ED();std::ofstream f(RF);return f.good();}
bool ResRepo_Cancel(int mi,int row,int col,const std::string& u){
    auto all=ResRepo_LoadAll(); size_t b=all.size();
    all.erase(std::remove_if(all.begin(),all.end(),[&](const Reservation& r){
        return r.movieIdx==mi&&r.row==row&&r.col==col&&r.username==u;}),all.end());
    if(all.size()==b) return false; Save(all); return true;
}
std::vector<Reservation> ResRepo_ForUser(const std::string& u){
    std::vector<Reservation> o;
    for(auto& r:ResRepo_LoadAll()) if(r.username==u) o.push_back(r);
    return o;
}
