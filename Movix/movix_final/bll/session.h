#pragma once
#include <string>
extern std::string gCurrentUser;
extern bool        gIsAdmin;
void SessionLogin (const std::string& uname, bool isAdmin);
void SessionLogout();
bool SessionActive();
