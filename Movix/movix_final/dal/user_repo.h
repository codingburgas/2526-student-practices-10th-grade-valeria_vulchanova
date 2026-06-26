#pragma once
#include "types.h"
#include <vector>
#include <string>

std::vector<UserRecord> UserRepo_LoadAll();
bool UserRepo_Add   (const std::string& uname, const std::string& pwd);
bool UserRepo_Delete(const std::string& uname);
bool UserRepo_ChangePassword(const std::string& uname, const std::string& newPwd);
