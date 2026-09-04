#include "authMiddleware.h"
#include <sqlite3.h>

extern sqlite3* db;

bool authenticateUser(
    const crow::request& req,
    AuthUser& user
)
{
    std::string authHeader =
        req.get_header_value("Authorization");


    if (authHeader.empty())
    {
        return false;
    }


    const std::string prefix = "Bearer ";


    if (authHeader.substr(0, prefix.size()) != prefix)
    {
        return false;
    }


    std::string token =
        authHeader.substr(prefix.size());


    const char* query =
        "SELECT users.id, users.role "
        "FROM sessions "
        "JOIN users ON sessions.user_id = users.id "
        "WHERE sessions.token = ?;";


    sqlite3_stmt* statement;


    int result = sqlite3_prepare_v2(
        db,
        query,
        -1,
        &statement,
        nullptr
    );


    if (result != SQLITE_OK)
    {
        return false;
    }


    sqlite3_bind_text(
        statement,
        1,
        token.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    result = sqlite3_step(statement);


    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(statement);

        return false;
    }


    user.id =
        sqlite3_column_int(
            statement,
            0
        );


    user.role =
        reinterpret_cast<const char*>(
            sqlite3_column_text(
                statement,
                1
            )
        );


    sqlite3_finalize(statement);


    return true;
}