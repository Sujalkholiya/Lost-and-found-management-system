#include <crow.h>

// Function from userController.cpp
crow::response registerUser(const crow::request& req);

crow::response loginUser(const crow::request& req);

crow::response getCurrentUser(const crow::request& req);

crow::response updateCurrentUser(const crow::request& req);


void userRoutes(crow::SimpleApp& app)
{
    CROW_ROUTE(app, "/api/users/register").methods(crow::HTTPMethod::POST)(
        registerUser
    );

    CROW_ROUTE(app, "/api/users/login").methods(crow::HTTPMethod::POST)(
        loginUser
    );

    CROW_ROUTE(app, "/api/users/me").methods(crow::HTTPMethod::GET)(
        getCurrentUser
    );

    CROW_ROUTE(app, "/api/users/me").methods(crow::HTTPMethod::PUT)(
        updateCurrentUser
    );
}