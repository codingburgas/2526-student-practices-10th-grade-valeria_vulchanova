#pragma once
#include <string>
#include "../dal/user_repo.h"

bool AuthLogin   (const std::string& uname,const std::string& pwd);
bool AuthRegister(const std::string& uname,const std::string& pwd);
bool AuthChangePassword(const std::string& uname,const std::string& oldPwd,const std::string& newPwd);
