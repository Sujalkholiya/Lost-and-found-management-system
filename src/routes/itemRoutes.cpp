#include <crow.h>


crow::response createItem(
    const crow::request& req
);

crow::response getAllItems(
    const crow::request& req
);

crow::response getItemById(
    const crow::request& req,
    int id
);

crow::response updateItem(
    const crow::request& req,
    int id
);

crow::response deleteItem(
    const crow::request& req,
    int id
);

crow::response getAdminItems(
    const crow::request& req
);

crow::response getUploadedFile(
    const crow::request& req,
    const std::string& filename
);

crow::response getUserItems(
    const crow::request& req
);


void itemRoutes(crow::SimpleApp& app)
{
    CROW_ROUTE(app, "/api/items")
    .methods(crow::HTTPMethod::POST)
    (
        createItem
    );


    CROW_ROUTE(app, "/api/items")
    .methods(crow::HTTPMethod::GET)
    (
        getAllItems
    );

    CROW_ROUTE(app, "/api/users/me/items")
    .methods(crow::HTTPMethod::GET)
    (
        getUserItems
    );


    CROW_ROUTE(app, "/api/items/<int>")
    .methods(crow::HTTPMethod::GET)
    (
        getItemById
    );

    CROW_ROUTE(app, "/api/items/<int>").methods(crow::HTTPMethod::PUT)
(
    updateItem
);

CROW_ROUTE(app, "/api/items/<int>")
.methods(crow::HTTPMethod::Delete)
(
    deleteItem
);

CROW_ROUTE(app, "/api/admin/items")
.methods(crow::HTTPMethod::GET)
(
    getAdminItems
);

    CROW_ROUTE(app, "/uploads/<string>")
    .methods(crow::HTTPMethod::GET)
    (
        getUploadedFile
    );

}