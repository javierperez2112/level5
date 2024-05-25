/**
 * @file mkindex.cpp
 * @author Marc S. Ressl
 * @brief Makes a database index
 * @version 0.3
 *
 * @copyright Copyright (c) 2022-2024 Marc S. Ressl
 */

#include <iostream>
#include <string>
#include <filesystem>
#include <string>
#include "CommandLineParser.h"

#include <sqlite3.h>

using namespace std;

static int onDatabaseEntry(void *userdata,
                           int argc,
                           char **argv,
                           char **azColName)
{
    cout << "--- Entry" << endl;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i])
            cout << azColName[i] << ": " << argv[i] << endl;
        else
            cout << azColName[i] << ": " << "NULL" << endl;
    }

    return 0;
}

int main(int argc,
         const char *argv[])
{
    char *databaseFile = "index.db";
    sqlite3 *database = NULL;
    char *databaseErrorMessage;
    std::string wwwDir = "./../www";

    // Open database file
    cout << "Opening database..." << endl;
    if (sqlite3_open(databaseFile, &database) != SQLITE_OK)
    {
        cout << "Can't open database: " << sqlite3_errmsg(database) << endl;

        return 1;
    }

    // Create a sample table
    cout << "Creating table..." << endl;
    if (sqlite3_exec(database,
                     "CREATE VIRTUAL TABLE fulltext USING fts5 ("
                     "title TEXT, "
                     "link TEXT, "
                     "body TEXT);",
                     NULL,
                     NULL,
                     &databaseErrorMessage) != SQLITE_OK)
        cout << "Error: " << sqlite3_errmsg(database) << endl;

    // Delete previous entries if table already existed
    cout << "Deleting previous entries..." << endl;
    if (sqlite3_exec(database,
                     "DELETE FROM fulltext;",
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK)
        cout << "Error: " << sqlite3_errmsg(database) << endl;

    // Create sample entries
    cout << "Creating sample entries..." << endl;
    if (sqlite3_exec(database,
                     "INSERT INTO fulltext (title, link, body) VALUES "
                     "('PORNHUB','pornhub.com','Pornoooo');",
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK)
        cout << "Error: " << sqlite3_errmsg(database) << endl;

    if (sqlite3_exec(database,
                     "INSERT INTO fulltext (title, link, body) VALUES "
                     "('XVIDEOS','xvideos.com','Más pornooo');",
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK)
        cout << "Error: " << sqlite3_errmsg(database) << endl;

    // Fetch entries
    cout << "Fetching entries..." << endl;
    if (sqlite3_exec(database,
                     "SELECT * from fulltext;",
                     onDatabaseEntry,
                     0,
                     &databaseErrorMessage) != SQLITE_OK)
        cout << "Error: " << sqlite3_errmsg(database) << endl;

    // Close database
    cout << "Closing database..." << endl;
    sqlite3_close(database);
}
