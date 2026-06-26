#include "auth_service.h"
#include "session.h"
bool AuthLogin(const std::string& u,const std::string& p){
    if(u=="admin"&&p=="1234"){SessionLogin("admin",true);return true;}
    for(auto& ur:UserRepo_LoadAll())
        if(ur.username==u&&ur.password==p){SessionLogin(u,ur.isAdmin);return true;}
    return false;
}
bool AuthRegister(const std::string& u,const std::string& p){
    if(u.size()<3||p.size()<4) return false;
    return UserRepo_Add(u,p);
}
bool AuthChangePassword(const std::string& u,const std::string& old,const std::string& np){
    if(np.size()<4) return false;
    for(auto& ur:UserRepo_LoadAll())
        if(ur.username==u&&ur.password==old) return UserRepo_ChangePassword(u,np);
    if(u=="admin"&&old=="1234") return false; // cannot change admin pwd this way
    return false;
}
