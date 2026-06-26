#include "rating_repo.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
static const char* RF2="data/ratings.txt";
static void ED(){system("if not exist data mkdir data 2>nul || mkdir -p data 2>/dev/null");}
std::vector<Rating> RatingRepo_LoadAll(){
    std::vector<Rating> o; std::ifstream f(RF2); if(!f) return o;
    std::string line;
    while(std::getline(f,line)){
        if(line.empty()) continue;
        std::istringstream ss(line); Rating r; std::string a,b;
        if(!std::getline(ss,r.username,'|')||!std::getline(ss,a,'|')||!std::getline(ss,b)) continue;
        try{r.movieId=std::stoi(a);r.stars=std::stoi(b);}catch(...){continue;}
        o.push_back(r);
    } return o;
}
static void Save(const std::vector<Rating>& v){
    ED(); std::ofstream f(RF2);
    for(auto& r:v) f<<r.username<<"|"<<r.movieId<<"|"<<r.stars<<"\n";
}
bool RatingRepo_Set(const std::string& u,int mid,int stars){
    stars=std::clamp(stars,1,5);
    auto all=RatingRepo_LoadAll(); bool found=false;
    for(auto& r:all) if(r.username==u&&r.movieId==mid){r.stars=stars;found=true;break;}
    if(!found) all.push_back({u,mid,stars});
    Save(all); return true;
}
int RatingRepo_Get(const std::string& u,int mid){
    for(auto& r:RatingRepo_LoadAll()) if(r.username==u&&r.movieId==mid) return r.stars;
    return 0;
}
float RatingRepo_Avg(int mid){
    auto all=RatingRepo_LoadAll(); float sum=0; int cnt=0;
    for(auto& r:all) if(r.movieId==mid){sum+=r.stars;cnt++;}
    return cnt?sum/cnt:0.f;
}
bool RatingRepo_Delete(const std::string& u,int mid){
    auto all=RatingRepo_LoadAll(); size_t b=all.size();
    all.erase(std::remove_if(all.begin(),all.end(),[&](const Rating& r){return r.username==u&&r.movieId==mid;}),all.end());
    if(all.size()==b) return false; Save(all); return true;
}
