#include "user_repo.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
static const char* FILE_PATH = "data/users.txt";
static void EnsureDir(){ system("if not exist data mkdir data 2>nul || mkdir -p data 2>/dev/null"); }

std::vector<UserRecord> UserRepo_LoadAll(){
    std::vector<UserRecord> out;
    std::ifstream f(FILE_PATH); if(!f) return out;
    std::string line;
    while(std::getline(f,line)){
        if(line.empty()||line[0]=='#') continue;
        std::istringstream ss(line); UserRecord u; std::string flag;
        if(!std::getline(ss,u.username,'|')) continue;
        if(!std::getline(ss,u.password,'|')) continue;
        std::getline(ss,flag); u.isAdmin=(flag=="1");
        if(!u.username.empty()) out.push_back(u);
    }
    return out;
}
static void SaveAll(const std::vector<UserRecord>& v){
    EnsureDir(); std::ofstream f(FILE_PATH);
    for(auto& u:v) f<<u.username<<"|"<<u.password<<"|"<<(u.isAdmin?1:0)<<"\n";
}
bool UserRepo_Add(const std::string& uname,const std::string& pwd){
    if(uname=="admin"||uname.empty()||pwd.empty()) return false;
    auto all=UserRepo_LoadAll();
    for(auto& u:all) if(u.username==uname) return false;
    all.push_back({uname,pwd,false}); SaveAll(all); return true;
}
bool UserRepo_Delete(const std::string& uname){
    if(uname=="admin") return false;
    auto all=UserRepo_LoadAll(); size_t b=all.size();
    all.erase(std::remove_if(all.begin(),all.end(),[&](const UserRecord& u){return u.username==uname;}),all.end());
    if(all.size()==b) return false; SaveAll(all); return true;
}
bool UserRepo_ChangePassword(const std::string& uname,const std::string& newPwd){
    auto all=UserRepo_LoadAll(); bool found=false;
    for(auto& u:all) if(u.username==uname){u.password=newPwd;found=true;break;}
    if(!found) return false; SaveAll(all); return true;
}
