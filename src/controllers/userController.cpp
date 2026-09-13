#include <crow.h>
#include <sqlite3.h>
#include <iostream>
#include <random>
#include <sstream>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <algorithm>
#include "../middleware/authMiddleware.h"

// Function and database from database.cpp
extern sqlite3* db;


// Register User Controller
crow::response registerUser(const crow::request& req)
{
    auto body = crow::json::load(req.body);

    // Check if JSON is valid
    if (!body)
    {
        return crow::response(
            400,
            "Invalid JSON data"
        );
    }

    // Check required fields
    if (!body.has("name") ||
        !body.has("email") ||
        !body.has("password"))
    {
        return crow::response(
            400,
            "Name, email and password are required"
        );
    }

    std::string name = body["name"].s();
    std::string email = body["email"].s();
    std::string password = body["password"].s();


    // SQL query
    const char* query =
        "INSERT INTO users (name, email, password) "
        "VALUES (?, ?, ?);";


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
        return crow::response(
            500,
            "Database query preparation failed"
        );
    }


    // Bind values safely
    sqlite3_bind_text(
        statement,
        1,
        name.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        2,
        email.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        3,
        password.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    // Execute query
    result = sqlite3_step(statement);

    sqlite3_finalize(statement);


    if (result != SQLITE_DONE)
    {
        return crow::response(
            400,
            "User already exists or registration failed"
        );
    }


    crow::json::wvalue response;

    response["message"] = "User registered successfully";
    response["name"] = name;
    response["email"] = email;

    return crow::response(201, response);
}

std::string generateToken()
{
    static std::atomic<uint64_t> counter{0};
    uint64_t c = ++counter;
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    std::random_device rd;
    std::seed_seq seq{
        static_cast<uint32_t>(now & 0xFFFFFFFF),
        static_cast<uint32_t>((now >> 32) & 0xFFFFFFFF),
        static_cast<uint32_t>(c & 0xFFFFFFFF),
        static_cast<uint32_t>((c >> 32) & 0xFFFFFFFF),
        static_cast<uint32_t>(rd())
    };

    std::mt19937_64 generator(seq);
    std::uniform_int_distribution<unsigned long long> distribution;

    std::stringstream token;
    token << std::hex << std::setfill('0')
          << std::setw(16) << distribution(generator)
          << std::setw(16) << distribution(generator);

    return token.str();
}

crow::response loginUser(const crow::request& req)
{
    auto body = crow::json::load(req.body);

    if (!body)
    {
        return crow::response(
            400,
            "Invalid JSON data"
        );
    }

    if (!body.has("email") ||
        !body.has("password"))
    {
        return crow::response(
            400,
            "Email and password are required"
        );
    }

    std::string email = body["email"].s();
    std::string password = body["password"].s();


   const char* query =
    "SELECT id, name, email, password, role "
    "FROM users "
    "WHERE email = ?;";


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
    std::string errorMessage = sqlite3_errmsg(db);

    return crow::response(
        500,
        errorMessage
    );
}


    sqlite3_bind_text(
        statement,
        1,
        email.c_str(),
        -1,
        SQLITE_TRANSIENT
    );


    result = sqlite3_step(statement);

    if (result != SQLITE_ROW)
    {
        sqlite3_finalize(statement);

        return crow::response(
            401,
            "Invalid email or password"
        );
    }


    int userId = sqlite3_column_int(
        statement,
        0
    );

    const char* rawName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
    const char* rawPass = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
    const char* rawRole = reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));

    std::string name = rawName ? rawName : "";
    std::string storedPassword = rawPass ? rawPass : "";
    std::string role = rawRole ? rawRole : "student";

    sqlite3_finalize(statement);


    if (password != storedPassword)
    {
        return crow::response(
            401,
            "Invalid email or password"
        );
    }

    std::string token = generateToken();

    const char* sessionQuery =
    "INSERT INTO sessions (user_id, token) "
    "VALUES (?, ?);";

sqlite3_stmt* sessionStatement;

result = sqlite3_prepare_v2(
    db,
    sessionQuery,
    -1,
    &sessionStatement,
    nullptr
);

if (result != SQLITE_OK)
{
    std::string errMsg = sqlite3_errmsg(db);
    std::cout << "[ERROR] loginUser: Failed to prepare session query! Error: " << errMsg << std::endl;
    return crow::response(
        500,
        "Failed to create session: " + errMsg
    );
}

sqlite3_bind_int(
    sessionStatement,
    1,
    userId
);

sqlite3_bind_text(
    sessionStatement,
    2,
    token.c_str(),
    -1,
    SQLITE_TRANSIENT
);

result = sqlite3_step(sessionStatement);

sqlite3_finalize(sessionStatement);

if (result != SQLITE_DONE)
{
    std::string errMsg = sqlite3_errmsg(db);
    std::cout << "[ERROR] loginUser: Failed to save session! sqlite3_step returned: " << result 
              << ", errmsg: " << errMsg << std::endl;
    return crow::response(
        500,
        "Failed to save session: " + errMsg
    );
}

    crow::json::wvalue response;

response["message"] = "Login successful";

response["user"]["id"] = userId;
response["user"]["name"] = name;
response["user"]["email"] = email;
response["user"]["role"] = role;
response["token"] = token;

    return crow::response(200, response);
}

crow::response getCurrentUser(const crow::request& req)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        crow::json::wvalue err;
        err["message"] = "Unauthorized";
        return crow::response(401, err);
    }

    const char* query = "SELECT id, name, email, role FROM users WHERE id = ?;";
    sqlite3_stmt* statement;
    if (sqlite3_prepare_v2(db, query, -1, &statement, nullptr) != SQLITE_OK)
    {
        crow::json::wvalue err;
        err["message"] = "Database query preparation failed";
        return crow::response(500, err);
    }

    sqlite3_bind_int(statement, 1, user.id);

    if (sqlite3_step(statement) != SQLITE_ROW)
    {
        sqlite3_finalize(statement);
        crow::json::wvalue err;
        err["message"] = "User not found";
        return crow::response(404, err);
    }

    int id = sqlite3_column_int(statement, 0);
    const char* nameCol = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
    const char* emailCol = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
    const char* roleCol = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));

    crow::json::wvalue response;
    response["user"]["id"] = id;
    response["user"]["name"] = nameCol ? nameCol : "";
    response["user"]["email"] = emailCol ? emailCol : "";
    response["user"]["role"] = roleCol ? roleCol : "student";

    sqlite3_finalize(statement);
    return crow::response(200, response);
}

crow::response updateCurrentUser(const crow::request& req)
{
    AuthUser user;
    if (!authenticateUser(req, user))
    {
        crow::json::wvalue err;
        err["message"] = "Unauthorized";
        return crow::response(401, err);
    }

    auto body = crow::json::load(req.body);
    if (!body || !body.has("name"))
    {
        crow::json::wvalue err;
        err["message"] = "Field 'name' is required";
        return crow::response(400, err);
    }

    std::string newName = body["name"].s();
    // Trim whitespace
    newName.erase(newName.begin(), std::find_if(newName.begin(), newName.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    newName.erase(std::find_if(newName.rbegin(), newName.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), newName.end());

    if (newName.empty())
    {
        crow::json::wvalue err;
        err["message"] = "Name cannot be empty";
        return crow::response(400, err);
    }
    if (newName.length() > 100)
    {
        crow::json::wvalue err;
        err["message"] = "Name cannot exceed 100 characters";
        return crow::response(400, err);
    }

    const char* updateQuery = "UPDATE users SET name = ? WHERE id = ?;";
    sqlite3_stmt* statement;
    if (sqlite3_prepare_v2(db, updateQuery, -1, &statement, nullptr) != SQLITE_OK)
    {
        crow::json::wvalue err;
        err["message"] = "Database query preparation failed";
        return crow::response(500, err);
    }

    sqlite3_bind_text(statement, 1, newName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 2, user.id);

    int stepResult = sqlite3_step(statement);
    sqlite3_finalize(statement);

    if (stepResult != SQLITE_DONE)
    {
        crow::json::wvalue err;
        err["message"] = "Failed to update profile";
        return crow::response(500, err);
    }

    // Return updated user
    const char* selectQuery = "SELECT id, name, email, role FROM users WHERE id = ?;";
    if (sqlite3_prepare_v2(db, selectQuery, -1, &statement, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(statement, 1, user.id);
        if (sqlite3_step(statement) == SQLITE_ROW)
        {
            crow::json::wvalue resp;
            resp["message"] = "Profile updated successfully";
            resp["user"]["id"] = sqlite3_column_int(statement, 0);
            const char* n = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
            const char* e = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
            const char* r = reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
            resp["user"]["name"] = n ? n : "";
            resp["user"]["email"] = e ? e : "";
            resp["user"]["role"] = r ? r : "student";
            sqlite3_finalize(statement);
            return crow::response(200, resp);
        }
        sqlite3_finalize(statement);
    }

    crow::json::wvalue resp;
    resp["message"] = "Profile updated successfully";
    resp["user"]["id"] = user.id;
    resp["user"]["name"] = newName;
    return crow::response(200, resp);
}