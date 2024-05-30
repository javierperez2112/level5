/**
 * @file HttpRequestHandler.h
 * @author Marc S. Ressl
 * @author Ignacio Rojana
 * @author Javier Pérez
 * @author Rocco Gastaldi
 * @brief EDAoggle search engine
 * @version 0.3
 *
 * @copyright Copyright (c) 2022-2024 Marc S. Ressl
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>

#include <sqlite3.h>

#include "HttpRequestHandler.h"

using namespace std;

typedef struct
{
    string name;
    filesystem::path path;
} Article;

static int onDatabaseEntry(void *userdata,
                           int argc,
                           char **argv,
                           char **azColName)
{
    cout << "--- Entry found:" << endl;
    string fileName = argv[0];
    fileName = fileName.substr(0, fileName.find_last_of('.'));
    string::iterator iter = fileName.begin();
    while (++iter != fileName.end())
        if (*iter == '_')
            *iter = ' ';
    cout << fileName << endl;
    filesystem::path filePath = argv[1];
    ((vector<Article> *)userdata)->push_back({fileName, filePath});
    return 0;
}

HttpRequestHandler::HttpRequestHandler(string homePath)
{
    this->homePath = homePath;
}

/**
 * @brief Serves a webpage from file
 *
 * @param url The URL
 * @param response The HTTP response
 * @return true URL valid
 * @return false URL invalid
 */
bool HttpRequestHandler::serve(string url, vector<char> &response)
{
    // Blocks directory traversal
    // e.g. https://www.example.com/show_file.php?file=../../MyFile
    // * Builds absolute local path from url
    // * Checks if absolute local path is within home path
    auto homeAbsolutePath = filesystem::absolute(homePath);
    auto relativePath = homeAbsolutePath / url.substr(1);
    string path = filesystem::absolute(relativePath.make_preferred()).string();

    if (path.substr(0, homeAbsolutePath.string().size()) != homeAbsolutePath)
        return false;

    // Serves file
    ifstream file(path);
    if (file.fail())
        return false;

    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);

    response.resize(fileSize);
    file.read(response.data(), fileSize);

    return true;
}

bool HttpRequestHandler::handleRequest(string url,
                                       HttpArguments arguments,
                                       vector<char> &response)
{
    auto start = chrono::high_resolution_clock::now();
    string searchPage = "/search";
    if (url.substr(0, searchPage.size()) == searchPage)
    {
        string searchString;
        if (arguments.find("q") != arguments.end())
        {
            searchString = arguments["q"];
            string::iterator iter = searchString.begin();
            ;
            // Eliminate reserved characters.
            while (iter != searchString.end())
            {
                if (*iter == '\'' || *iter == ';')
                {
                    iter = searchString.erase(iter);
                }
                else
                {
                    iter++;
                }
            }
        }

        string::iterator iter;
        // Header
        string responseString = string("<!DOCTYPE html>\
<html>\
\
<head>\
    <meta charset=\"utf-8\" />\
    <title>EDAoogle</title>\
    <link rel=\"preload\" href=\"https://fonts.googleapis.com\" />\
    <link rel=\"preload\" href=\"https://fonts.gstatic.com\" crossorigin />\
    <link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@400;800&display=swap\" rel=\"stylesheet\" />\
    <link rel=\"preload\" href=\"../css/style.css\" />\
    <link rel=\"stylesheet\" href=\"../css/style.css\" />\
</head>\
\
<body>\
    <article class=\"edaoogle\">\
        <div class=\"title\"><a href=\"/\">EDAoogle</a></div>\
        <div class=\"search\">\
            <form action=\"/search\" method=\"get\">\
                <input type=\"text\" name=\"q\" value=\"" +
                                       searchString + "\" autofocus>\
            </form>\
        </div>\
        ");

        // YOUR JOB: fill in results
        // Open database.
        char *databaseFile = "index.db";
        sqlite3 *database = NULL;
        char *databaseErrorMessage;
        sqlite3_open(databaseFile, &database);
        string searchCommand = "SELECT * from fulltext WHERE fulltext MATCH '" + searchString + "' ORDER BY rank;";
        cout << "\nSearching: " << searchString << endl;
        vector<Article> results;

        if (sqlite3_exec(database,
                         searchCommand.c_str(),
                         onDatabaseEntry,
                         (void *)&results,
                         &databaseErrorMessage) != SQLITE_OK)
        {
            cout << "Error: " << sqlite3_errmsg(database) << endl;
        }

        auto stop = chrono::high_resolution_clock::now();
        float searchTime = chrono::duration_cast<chrono::microseconds>(stop - start).count();
        searchTime /= 10.0E6F;

        // Print search results
        responseString += "<div class=\"results\">" + to_string(results.size()) +
                          " results (" + to_string(searchTime) + " seconds):</div>";
        for (auto &result : results)
        {
            string articlePath = filesystem::relative(result.path, this->homePath).string();
            responseString += "<div class=\"result\"><a href=\"" +
                              articlePath + "\">" + result.name + "</a></div>";
        }
        // Trailer
        responseString += "    </article>\
</body>\
</html>";

        response.assign(responseString.begin(), responseString.end());

        return true;
    }
    else
        return serve(url, response);

    return false;
}