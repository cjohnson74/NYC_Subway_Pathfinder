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

#include "Transit.cpp"

int main() {
    string STOPS_FILE = "mock_data/stops.txt"; // default: "transit_data/stops.txt"
    string STOP_TIMES_FILE = "mock_data/stop_times_debug.txt"; // default: "transit_data/stop_times.txt"

    Transit transit(STOPS_FILE, STOP_TIMES_FILE);

    transit.printRoutes();

    string stopA_name, stopB_name, stopA_id, stopB_id, exit;

    // Menu CLI
    cout << "Welcome to [name of the application]." << endl;

    while (true) {
        // User Input
        while (true) {
            cout << "Enter your current location:" << endl;
            getline(cin, stopA_name);

            if (transit.stopExists(stopA_name)) {
                stopA_id = transit.getStopID(stopA_name);
                break;
            } else {
                cout << "This stop does not exist." << endl;
            }
        }
        
        while (true) {
            cout << "Enter your destination:" << endl;
            getline(cin, stopB_name);

            if (transit.stopExists(stopB_name)) {
                stopB_id = transit.getStopID(stopB_name);
                break;
            } else {
                cout << "This stop does not exist." << endl;
            }
        }

        // TODO: Calculation & Output
        cout << "Fastest Route: " << endl;
        cout << "Estimated Route Time: " << endl;
        cout << "Dijksta's Algorithm runtime: " << endl;
        cout << "A* Search Algorithm runtime: " << endl;
        cout << endl;

        cout << "Exit application? (y to exit, any other character to find new route)" << endl;
        cin >> exit;

        // Termination
        if (exit == "y") {
            cout << "Exiting application..." << endl; 
            break;
        }
    }
    
    return 0;
}