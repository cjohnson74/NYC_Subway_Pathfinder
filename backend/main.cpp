/* Problem Statement: Given a transit system, determine fastest route from stop A to stop B.


Design Decisions:
input read from .txt file in GTFS format (eg. https://gtfs.org/schedule/reference/#stopstxt)

1. Map #1: Routes represented as an Adjacency List Data Structure
Transit System => map<string, set<pair<string, int>>>
Stops => string (stop_id)
Route Time => int (seconds)
- Chosen for ideal time and space efficiency
- Set was chosen over vector to avoid duplicates in added routes, a set fixes this elegantly.
  The reasoning is explained in more detail in Transit.cpp -> insertRoute()

2. Map #2: Stops data (stop_id, name)
stop_id (string) => name (string)
- A separate map will be created to index names to stop_ids. This makes insertion fast and easy.
  (Creating a Node class with stop_id/stop_name makes it more tedious to perform insertion due to
  having to create a Node each time before a uniqueness check).

3. Map #3: Stops data flipped (name, stop_id)
name (string) => stop_id (string)
- Again, a separate map is necessary for fast and easy access to existing stops from the CLI string input.

4. All transit functionality within Transit class
- insertRoute, getAdjacent, getStopName, getStopID, shortest path algorithms (Dijksta, A* Search)
- constructor Transit(filepath) handles GTFS parsing and map populating

5. CLI menu
This will remain rooted in the main function.

- Design change: map -> unordered_map
  Removing the unneeded ordered map should improve performance.
  Unordered sets cause errors, so omitting that.

*/

#include <iostream>
#include "Transit.cpp"
#include "include/httplib.h" // Include the httplib library
#include "include/json.hpp"

void constructTransit(const httplib::Request& req, httplib::Response& res, Transit& transit) {
    // Set a longer timeout duration (in seconds)
    res.set_header("Connection", "Keep-Alive");
    res.set_header("Keep-Alive", "timeout=600"); // Set timeout to 10 minutes (600 seconds)

    // string STOPS_FILE = "mock_data/stops.txt"; // default: "transit_data/stops.txt"
    string STOPS_FILE = "backend/transit_data/stops.txt";
    // string STOP_TIMES_FILE = "mock_data/stop_times_debug.txt"; // default: "transit_data/stop_times.txt"
    string STOP_TIMES_FILE = "backend/transit_data/stop_times.txt";

    // Generate JSON response
    nlohmann::json responseJson;
    responseJson["res"] = "started building transit";

    // Start building transit in a separate thread to avoid blocking the request handler
    std::thread transitBuilder([&transit, &res]() {
        transit.buildTransit();
        // Once construction is complete, notify the client
        nlohmann::json completeJson;
        completeJson["res"] = "transite construction completed";
        res.set_content(completeJson.dump(), "application/json");
    });

    transitBuilder.detach(); // Detach the thread to let it run independently

    // Set response content type and body
    res.set_content(responseJson.dump(), "application/json");
}

// Define a function to handle POST requests
void handlePost(const httplib::Request& req, httplib::Response& res, Transit& transit) {
    // Parse JSON data from the request body
    // Extract start and end stations
    // Call A* algorithm to find shortest path
    // Generate JSON response containing shortest path
    std::string start = req.get_param_value("start");
    std::string end = req.get_param_value("end");
    
    // Execute A* algorithm and find shortest path
    tuple<vector<string>, float, chrono::microseconds, unordered_map<string, pair<double, double>>> a_star_res = transit.shortest_path_a_star(start, end);

    // Execute Dijkstra's algorithms and find shortest path
    int dijkstra_res = transit.shortest_path_dijkstra(start, end);

    // Generate JSON response
    nlohmann::json responseJson;
    unordered_map<string, pair<double, double>> stop_pos_map = get<3>(a_star_res);
    vector<pair<double, double>> stops_pos;

    for (auto stop : get<0>(a_star_res)) {
        stops_pos.push_back(stop_pos_map[stop]);
    }

    responseJson["shortestPath"] = stops_pos;

    // Set response content type and body
    res.set_content(responseJson.dump(), "application/json");
}

int main() {
    // BELOW IS SERVER CODE

    // Getting port environment variable from Heroku
    const char* port_str = getenv("PORT");
    int port = (port_str != nullptr) ? stoi(port_str) : 8080;

    // Create an instance of the HTTP server
    httplib::Server server;

    // Serve static files from the "frontend" directory
    server.set_mount_point("/", "frontend");

    Transit transit;

    // POST request handler to initiate construction of transit
    server.Post("/constructTransit", [&transit](const httplib::Request& req, httplib::Response& res) {
        constructTransit(req, res, transit);
    });

    // POST request handler for A Star
    server.Post("/findShortestPath", [&transit](const httplib::Request& req, httplib::Response& res) {
        handlePost(req, res, transit);
    });

    // Start the server and listen on port 8000
    server.listen("0.0.0.0", port);

    // ABOVE IS SERVER CODE

    // debugging
    transit.printRoutes();
    cout << endl;

    string stopA_name, stopB_name, stopA_id, stopB_id, exit;

    // Menu CLI
    cout << "Welcome to the NYC Subway Pathfinder!" << endl;
    cout << endl;

    while (true) {
        // User Input
        while (true) {
            cout << "Enter your current location:" << endl;
            getline(cin, stopA_name);

            if (transit.stopExists(stopA_name)) {
                stopA_id = transit.getStopID(stopA_name);
                break;
            } else {
                cout << "ERROR: This stop does not exist." << endl;
            }
        }
        
        while (true) {
            cout << "Enter your destination:" << endl;
            getline(cin, stopB_name);

            if (transit.stopExists(stopB_name)) {
                stopB_id = transit.getStopID(stopB_name);
                break;
            } else {
                cout << "ERROR: This stop does not exist." << endl;
            }
        }

        // A* Calculation
        // returns pair<path, time>
        tuple<vector<string>, float, chrono::microseconds, unordered_map<string, pair<double, double>>> a_star_calc = transit.shortest_path_a_star(stopA_id, stopB_id);

        string shortest_a_star_path = "";

        for (auto stop : get<0>(a_star_calc)) {
            shortest_a_star_path.append(stop + " ");
        }

        // Calculation & Output
        cout << endl;
        cout << "Fastest Route: " << "<insert path here>" << endl;
        if (!shortest_a_star_path.empty()) {
            cout << "A* Fastest Route: Path does not exist" << endl;
        } else {
            cout << "A* Fastest Route: " << shortest_a_star_path << endl;
        }
        cout << "Estimated Route Time: " << transit.shortest_path_dijkstra(stopA_id, stopB_id) << endl;
        cout << "A* Estimated Route Time: " << get<1>(a_star_calc) << endl;
        cout << "Dijksta's Algorithm runtime: " << endl;
        cout << "A* Search Algorithm runtime: " << get<2>(a_star_calc).count() << "microseconds" << endl;
        cout << endl;

        cout << "Exit application? (y to exit, any other character to find new route)" << endl;
        getline(cin, exit);

        // Termination
        if (exit == "y") {
            cout << "Exiting application..." << endl; 
            break;
        }
    }
    
    return 0;
}