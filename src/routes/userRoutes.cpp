#include <crow.h>

// Function from userController.cpp
crow::response registerUser(const crow::request& req);

crow::response loginUser(const crow::request& req);


void userRoutes(crow::SimpleApp& app)
{
    CROW_ROUTE(app, "/api/users/register").methods(crow::HTTPMethod::POST)(
        registerUser
    );

CROW_ROUTE(app, "/api/users/login").methods(crow::HTTPMethod::POST)(
    loginUser
);
}