#ifndef AUTH_MIDDLEWARE_H
#define AUTH_MIDDLEWARE_H

#include <crow.h>
#include <string>


struct AuthUser
{
    int id;
    std::string role;
};


bool authenticateUser(
    const crow::request& req,
    AuthUser& user
);

#endif