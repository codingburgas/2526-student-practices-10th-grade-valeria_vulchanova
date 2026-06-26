#include "movie_service.h"
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <set>

void MovieSvc_Init(){ MovieRepo_Init(); }
int  MovieSvc_Count(){ return MovieRepo_Count(); }

MovieView MovieSvc_Get(int idx,const std::string& uname){
    MovieView v; v.idx=idx; v.entry=MovieRepo_Get(idx);
    v.userRating=(float)RatingRepo_Get(uname,v.entry.id);
    v.avgRating =RatingRepo_Avg(v.entry.id);
    v.isFavorite=false; // handled by FavSvc
    return v;
}
std::vector<int> MovieSvc_Search(const std::string& q,const std::string& genre){
    std::vector<int> r;
    for(int i=0;i<MovieRepo_Count();i++){
        const auto& e=MovieRepo_Get(i);
        bool matchQ=q.empty();
        if(!matchQ){
            std::string t(e.title),g(e.genre),lo=q;
            auto low=[](std::string s){for(auto& c:s) c=tolower(c);return s;};
            if(low(t).find(low(lo))!=std::string::npos||
               low(g).find(low(lo))!=std::string::npos) matchQ=true;
        }
        bool matchG=genre.empty()||genre==e.genre;
        if(matchQ&&matchG) r.push_back(i);
    }
    return r;
}
std::vector<std::string> MovieSvc_AllGenres(){
    std::set<std::string> s;
    for(int i=0;i<MovieRepo_Count();i++) s.insert(MovieRepo_Get(i).genre);
    return std::vector<std::string>(s.begin(),s.end());
}
bool MovieSvc_Add(MovieEntry e){ return MovieRepo_Add(e); }
bool MovieSvc_Delete(int idx){ return MovieRepo_Delete(idx); }

void MovieSvc_SetPoster(int movieId,const std::string& src){
    char cmd[512],dst[128];
    snprintf(dst,sizeof(dst),"posters/movie_%d.jpg",movieId);
#ifdef _WIN32
    char wcmd[512]; snprintf(wcmd,sizeof(wcmd),"copy /Y \"%s\" \"%s\" >nul 2>&1",src.c_str(),dst);
    system(wcmd);
#else
    snprintf(cmd,sizeof(cmd),"cp \"%s\" \"%s\" 2>/dev/null",src.c_str(),dst);
    system(cmd);
#endif
}
bool MovieSvc_HasPoster(int movieId){
    char p[64]; snprintf(p,sizeof(p),"posters/movie_%d.jpg",movieId);
    FILE* f=fopen(p,"rb"); if(f){fclose(f);return true;}
    snprintf(p,sizeof(p),"posters/movie_%d.png",movieId);
    f=fopen(p,"rb"); if(f){fclose(f);return true;}
    return false;
}
bool MovieSvc_Rate(const std::string& u,int movieId,int stars){
    return RatingRepo_Set(u,movieId,stars);
}
