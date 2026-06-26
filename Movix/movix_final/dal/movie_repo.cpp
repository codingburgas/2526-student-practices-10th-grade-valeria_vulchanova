#include "movie_repo.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>

const ColourTheme COLOUR_THEMES[8]={
    {20,12,45,80,45,10,212,163,57,"GOLDEN"},
    {10,10,35,45,20,60,255,140,40,"EMBER"},
    {8,10,22,30,10,10,80,160,220,"STEEL"},
    {5,30,55,8,60,80,40,200,200,"OCEAN"},
    {10,20,50,50,10,10,180,200,255,"SKY"},
    {5,8,25,25,18,50,212,163,57,"COSMIC"},
    {15,10,30,40,25,55,180,120,255,"VIOLET"},
    {35,8,8,8,8,45,220,60,60,"CRIMSON"}
};

static std::vector<MovieEntry> gMovies;
static int gNextId=1;
static const char* MFILE="data/movies.txt";

static void EnsureDir(){system("if not exist data mkdir data 2>nul || mkdir -p data 2>/dev/null");}

static void Seed(){
    struct D{const char*t,*g,*di,*ca,*du,*y,*de;float r,p;int pAr,pAg,pAb,pBr,pBg,pBb,acr,acg,acb;};
    static const D S[]={
        {"DUNE: PART TWO","Sci-Fi / Epic","Denis Villeneuve","Timothee Chalamet, Zendaya, Austin Butler","2h 46min","2024","Paul Atreides unites with the Fremen, seeking revenge against those who destroyed his family.",8.5f,14.f,20,12,45,80,45,10,212,163,57},
        {"OPPENHEIMER","Biographical / Drama","Christopher Nolan","Cillian Murphy, Emily Blunt, Robert Downey Jr.","3h 00min","2023","The story of Oppenheimer and his role in the development of the atomic bomb.",8.9f,14.f,10,10,35,45,20,60,255,140,40},
        {"THE BATMAN","Action / Thriller","Matt Reeves","Robert Pattinson, Zoe Kravitz, Paul Dano","2h 56min","2022","Batman deciphers the Riddler's sinister riddles as murders shake Gotham.",7.9f,12.f,8,10,22,30,10,10,80,160,220},
        {"AVATAR: THE WAY OF WATER","Sci-Fi / Adventure","James Cameron","Sam Worthington, Zoe Saldana, Sigourney Weaver","3h 12min","2022","Jake Sully explores the regions of Pandora to protect his family.",7.6f,16.f,5,30,55,8,60,80,40,200,200},
        {"TOP GUN: MAVERICK","Action / Drama","Joseph Kosinski","Tom Cruise, Miles Teller, Jennifer Connelly","2h 10min","2022","Maverick trains a new generation of pilots for a dangerous mission.",8.3f,12.f,10,20,50,50,10,10,180,200,255},
        {"INTERSTELLAR","Sci-Fi / Drama","Christopher Nolan","Matthew McConaughey, Anne Hathaway, Jessica Chastain","2h 49min","2014","A crew travels through a wormhole to ensure humanity's survival.",8.7f,10.f,5,8,25,25,18,50,212,163,57},
        {"INCEPTION","Sci-Fi / Thriller","Christopher Nolan","Leonardo DiCaprio, Joseph Gordon-Levitt, Elliot Page","2h 28min","2010","A thief plants an idea into a CEO's mind through dream-stealing.",8.8f,10.f,15,10,30,40,25,55,180,120,255},
        {"SPIDER-MAN: NO WAY HOME","Action / Sci-Fi","Jon Watts","Tom Holland, Zendaya, Benedict Cumberbatch","2h 28min","2021","Peter Parker's identity reveal tears open the multiverse.",8.3f,13.f,35,8,8,8,8,45,220,60,60},
    };
    for(auto& d:S){
        MovieEntry e; e.id=gNextId++;
        snprintf(e.title,sizeof(e.title),"%s",d.t);
        snprintf(e.genre,sizeof(e.genre),"%s",d.g);
        snprintf(e.director,sizeof(e.director),"%s",d.di);
        snprintf(e.cast,sizeof(e.cast),"%s",d.ca);
        snprintf(e.duration,sizeof(e.duration),"%s",d.du);
        snprintf(e.year,sizeof(e.year),"%s",d.y);
        snprintf(e.desc,sizeof(e.desc),"%s",d.de);
        e.rating=d.r; e.price=d.p;
        e.pA_r=d.pAr;e.pA_g=d.pAg;e.pA_b=d.pAb;
        e.pB_r=d.pBr;e.pB_g=d.pBg;e.pB_b=d.pBb;
        e.ac_r=d.acr;e.ac_g=d.acg;e.ac_b=d.acb;
        gMovies.push_back(e);
    }
}

void MovieRepo_Init(){
    gMovies.clear(); gNextId=1; EnsureDir();
    std::ifstream f(MFILE); if(!f.is_open()){Seed();MovieRepo_Save();return;}
    std::string line;
    while(std::getline(f,line)){
        if(line.empty()||line[0]=='#') continue;
        std::vector<std::string> tok; std::istringstream ss(line); std::string t;
        while(std::getline(ss,t,'|')) tok.push_back(t);
        if((int)tok.size()<16) continue;
        MovieEntry e;
        try{e.id=std::stoi(tok[0]);}catch(...){continue;}
        if(e.id>=gNextId) gNextId=e.id+1;
        snprintf(e.title,sizeof(e.title),"%s",tok[1].c_str());
        snprintf(e.genre,sizeof(e.genre),"%s",tok[2].c_str());
        snprintf(e.director,sizeof(e.director),"%s",tok[3].c_str());
        snprintf(e.cast,sizeof(e.cast),"%s",tok[4].c_str());
        snprintf(e.duration,sizeof(e.duration),"%s",tok[5].c_str());
        snprintf(e.year,sizeof(e.year),"%s",tok[6].c_str());
        try{e.rating=std::stof(tok[7]);e.price=std::stof(tok[8]);}catch(...){}
        snprintf(e.desc,sizeof(e.desc),"%s",tok[9].c_str());
        try{e.pA_r=std::stoi(tok[10]);e.pA_g=std::stoi(tok[11]);e.pA_b=std::stoi(tok[12]);
            e.pB_r=std::stoi(tok[13]);e.pB_g=std::stoi(tok[14]);e.pB_b=std::stoi(tok[15]);}catch(...){}
        if((int)tok.size()>=19)
            try{e.ac_r=std::stoi(tok[16]);e.ac_g=std::stoi(tok[17]);e.ac_b=std::stoi(tok[18]);}catch(...){}
        gMovies.push_back(e);
    }
    if(gMovies.empty()){Seed();MovieRepo_Save();}
}
void MovieRepo_Save(){
    EnsureDir(); std::ofstream f(MFILE);
    for(auto& e:gMovies)
        f<<e.id<<"|"<<e.title<<"|"<<e.genre<<"|"<<e.director<<"|"<<e.cast<<"|"
         <<e.duration<<"|"<<e.year<<"|"<<e.rating<<"|"<<e.price<<"|"<<e.desc<<"|"
         <<e.pA_r<<"|"<<e.pA_g<<"|"<<e.pA_b<<"|"
         <<e.pB_r<<"|"<<e.pB_g<<"|"<<e.pB_b<<"|"
         <<e.ac_r<<"|"<<e.ac_g<<"|"<<e.ac_b<<"\n";
}
int MovieRepo_Count(){return (int)gMovies.size();}
const MovieEntry& MovieRepo_Get(int i){return gMovies[i];}
bool MovieRepo_Add(MovieEntry e){e.id=gNextId++;gMovies.push_back(e);MovieRepo_Save();return true;}
bool MovieRepo_Delete(int i){
    if(i<0||i>=(int)gMovies.size()) return false;
    gMovies.erase(gMovies.begin()+i);MovieRepo_Save();return true;
}
