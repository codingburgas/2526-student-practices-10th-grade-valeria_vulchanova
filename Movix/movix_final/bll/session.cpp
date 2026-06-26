#include "session.h"
std::string gCurrentUser;
bool        gIsAdmin=false;
void SessionLogin(const std::string& u,bool a){gCurrentUser=u;gIsAdmin=a;}
void SessionLogout(){gCurrentUser="";gIsAdmin=false;}
bool SessionActive(){return !gCurrentUser.empty();}
