/**
 * @file mkindex.cpp
 * @author Marc S. Ressl
 * @brief Makes a database index
 * @version 0.3
 *
 * @copyright Copyright (c) 2022-2024 Marc S. Ressl
 */

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "CommandLineParser.h"

#include <sqlite3.h>

using namespace std;

static string removeTags(string &text);

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

int main(int argc, const char *argv[])
{
    CommandLineParser parser(argc, argv);
    char *databaseFile = "index.db";
    sqlite3 *database = NULL;
    char *databaseErrorMessage;
    if (!parser.hasOption("-h"))
    {
        cout << "Error: must specify a path!" << endl;
        return 1;
    }
    std::filesystem::path wikiPath = parser.getOption("-h") + "/wiki";

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
                     "CREATE VIRTUAL TABLE fulltext USING fts5 (title, path, body);",
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

    // Add entries (without HTML tags) to the table.
    for (auto &entry : filesystem::directory_iterator(wikiPath))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        ifstream entryFile(entry.path());
        if (!entryFile.is_open())
        {
            continue;
        }
        string entryPath = entry.path();
        string fileName = entry.path().filename();
        string entryTitle = removeTags(fileName);
        cout << "Addind entry: " << entryTitle << endl;
        string entryLine, entryText;
        while (getline(entryFile, entryLine))
        {
            entryText.append(removeTags(entryLine));
        }
        entryFile.close();
        // Add new entry to table.
        std::string SQLcommand = "INSERT INTO fulltext (title, path, body) VALUES";
        SQLcommand.append("('");
        SQLcommand.append(entryTitle);
        SQLcommand.append("','");
        SQLcommand.append(removeTags(entryPath));
        SQLcommand.append("','");
        SQLcommand.append(entryText);
        SQLcommand.append("');");
        if (sqlite3_exec(database,
                         SQLcommand.c_str(),
                         NULL,
                         0,
                         &databaseErrorMessage) != SQLITE_OK)
            cout << "Error: " << sqlite3_errmsg(database) << endl;
    }
    // Close database
    cout << "Closing database..." << endl;
    sqlite3_close(database);
}

/**
 * @brief Remove HTML tags from .html file text.
 * @note Also used on titles to double '.
 *
 * @param text The text from the HTML file.
 * @return The text without tags.
 */
static string removeTags(string &text)
{
    string newText;
    bool inTag = false;
    for (char c : text)
    {
        if (c == '<')
        {
            inTag = true;
            continue;
        }
        if (c == '>')
        {
            inTag = false;
            continue;
        }
        if (!inTag)
        {
            newText += c;
            if (c == '\'')
            {
                newText += c;
            }
        }
    }
    return newText;
}
