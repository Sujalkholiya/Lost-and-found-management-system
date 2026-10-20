
#include <crow.h>
#include <cstdlib>
#include <iostream>
#include <string>

bool connectDatabase();
bool promoteUserToAdmin(const char* email);
void userRoutes(crow::SimpleApp& app);
void itemRoutes(crow::SimpleApp& app);
void claimRoutes(crow::SimpleApp& app);

int main()
{
    if (!connectDatabase())
    {
        return 1;
    }

        const char* adminEmail = std::getenv("ADMIN_EMAIL");
    const char* bootstrapEnabled = std::getenv("ADMIN_BOOTSTRAP_ENABLED");

    if (adminEmail != nullptr &&
        bootstrapEnabled != nullptr &&
        std::string(bootstrapEnabled) == "true")
    {
        if (promoteUserToAdmin(adminEmail))
        {
            std::cout << "Admin account promoted successfully." << std::endl;
        }
        else
        {
            std::cout << "Admin promotion failed. Check that the email exists." << std::endl;
        }
    }

    crow::SimpleApp app;

    userRoutes(app);
    itemRoutes(app);
    claimRoutes(app);

    CROW_ROUTE(app, "/")
    (
        []()
        {
            return "Lost and Found Backend is running!";
        }
    );

    int port = 18080;

    const char* envPort = std::getenv("PORT");

    if (envPort != nullptr)
    {
        port = std::atoi(envPort);
    }

    app.bindaddr("0.0.0.0")
       .port(port)
       .multithreaded()
       .run();

    return 0;
}
