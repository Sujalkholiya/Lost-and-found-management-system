#include <crow.h>
#include <sqlite3.h>
#include <iostream>
#include <random>
#include <sstream>
#include <chrono>
#include <atomic>
#include <iomanip>

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