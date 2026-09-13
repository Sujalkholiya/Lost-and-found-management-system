#include <crow.h>

bool connectDatabase();
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

    app.port(18080)
       .multithreaded()
       .run();

    return 0;
}