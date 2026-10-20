#include <sqlite3.h>
#include <iostream>

sqlite3* db = nullptr;

bool executeQuery(const char* query)
{
    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        query,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        std::cout << "SQL Error: ";

        if (errorMessage != nullptr)
        {
            std::cout << errorMessage;
            sqlite3_free(errorMessage);
        }

        std::cout << std::endl;

        return false;
    }

    return true;
}


bool connectDatabase()
{
    int result = sqlite3_open("lost_found.db", &db);

    if (result != SQLITE_OK)
    {
        std::cout << "Database connection failed: "
                  << sqlite3_errmsg(db)
                  << std::endl;

        return false;
    }

    std::cout << "Database connected successfully!"
              << std::endl;


    // USERS TABLE
    const char* createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'student'
        );
    )";


    // ITEMS TABLE
    const char* createItemsTable = R"(
        CREATE TABLE IF NOT EXISTS items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT NOT NULL,
            type TEXT NOT NULL,
            location TEXT NOT NULL,
            status TEXT DEFAULT 'active',
            user_id INTEGER,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            image_path TEXT DEFAULT '',

            FOREIGN KEY (user_id) REFERENCES users(id)
        );
    )";

    const char* createSessionsTable = R"(
    CREATE TABLE IF NOT EXISTS sessions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        token TEXT NOT NULL UNIQUE,

        FOREIGN KEY (user_id) REFERENCES users(id)
    );
)";

    const char* createClaimsTable = R"(
    CREATE TABLE IF NOT EXISTS claims (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        claim_reference TEXT NOT NULL UNIQUE,
        item_id INTEGER NOT NULL,
        user_id INTEGER NOT NULL,
        status TEXT NOT NULL DEFAULT 'pending',
        last_seen_location TEXT NOT NULL,
        unique_identifying_details TEXT NOT NULL,
        item_contents TEXT NOT NULL,
        additional_details TEXT DEFAULT '',
        rejection_reason TEXT DEFAULT '',
        pickup_location TEXT DEFAULT 'Admin Block',
        reviewed_by INTEGER,
        handover_admin_id INTEGER,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        reviewed_at DATETIME,
        approved_at DATETIME,
        handed_over_at DATETIME,

        FOREIGN KEY (item_id) REFERENCES items(id),
        FOREIGN KEY (user_id) REFERENCES users(id),
        FOREIGN KEY (reviewed_by) REFERENCES users(id),
        FOREIGN KEY (handover_admin_id) REFERENCES users(id)
    );
)";

    if (!executeQuery(createUsersTable))
    {
        return false;
    }


    if (!executeQuery(createItemsTable))
    {
        return false;
    }

    if (!executeQuery(createSessionsTable))
    {
        return false;
    }

    if (!executeQuery(createClaimsTable))
    {
        return false;
    }

    // Auto-migrate: check if image_path exists in existing items table
    sqlite3_stmt* checkStmt;
    bool hasImagePath = false;
    if (sqlite3_prepare_v2(db, "PRAGMA table_info(items);", -1, &checkStmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(checkStmt) == SQLITE_ROW)
        {
            const char* colName = reinterpret_cast<const char*>(sqlite3_column_text(checkStmt, 1));
            if (colName && std::string(colName) == "image_path")
            {
                hasImagePath = true;
                break;
            }
        }
        sqlite3_finalize(checkStmt);
    }
    if (!hasImagePath)
    {
        executeQuery("ALTER TABLE items ADD COLUMN image_path TEXT DEFAULT '';");
    }


    std::cout << "Database tables are ready!"
              << std::endl;

    return true;
}


bool promoteUserToAdmin(const char* email)
{
    sqlite3_stmt* stmt = nullptr;

    const char* sql =
        "UPDATE users SET role = 'admin' WHERE email = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "SQL prepare failed: "
                  << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);

    int result = sqlite3_step(stmt);

    if (result != SQLITE_DONE)
    {
        std::cerr << "SQL update failed: "
                  << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    int changedRows = sqlite3_changes(db);

    sqlite3_finalize(stmt);

    if (changedRows != 1)
    {
        std::cerr << "Admin promotion affected "
                  << changedRows
                  << " rows. Email may not exist in this database."
                  << std::endl;
        return false;
    }

    return true;
}
