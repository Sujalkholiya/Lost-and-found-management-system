#include <crow.h>
#include <cstdlib>
#include <string>
#include <sqlite3.h>

extern sqlite3* db;

bool promoteUserToAdmin(const char* email);

crow::response setupAdmin(const crow::request& req)
{
    const char* setupKey = std::getenv("ADMIN_SETUP_KEY");
    const char* adminEmail = std::getenv("ADMIN_EMAIL");

    if (setupKey == nullptr || adminEmail == nullptr ||
        std::string(setupKey).empty() ||
        std::string(adminEmail).empty())
    {
        return crow::response(503, "Admin setup is not configured");
    }

    std::string providedKey = req.get_header_value("X-Admin-Setup-Key");

    if (providedKey.empty() || providedKey != setupKey)
    {
        return crow::response(403, "Invalid setup key");
    }

    if (promoteUserToAdmin(adminEmail))
    {
        return crow::response(200, "Admin account promoted successfully");
    }

    return crow::response(
        409,
        "Promotion failed. Confirm the configured email exists in the database."
    );
}


// Function from userController.cpp
crow::response registerUser(const crow::request& req);

crow::response loginUser(const crow::request& req);

crow::response getCurrentUser(const crow::request& req);

crow::response updateCurrentUser(const crow::request& req);


void userRoutes(crow::SimpleApp& app)
{
CROW_ROUTE(app, "/api/admin/setup")
    .methods(crow::HTTPMethod::POST)(setupAdmin);
    
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
