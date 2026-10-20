
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
