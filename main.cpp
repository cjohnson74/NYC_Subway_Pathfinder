/* Problem Statement: Given a transit system, determine fastest route from stop A to stop B.


Design Decisions:
input read from .txt file in GTFS format (eg. https://gtfs.org/schedule/reference/#stopstxt)

1. Map #1: Routes represented as an Adjacency List Data Structure
Transit System => map<string, vector<pair<string, int>>>
Stops => string (stop_id)
Route Time => int (seconds)
- chosen for ideal time and space efficiency

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

Hierarchy:
main (CLI) -> Transit -> Map#1/Map#2

*/

#include "Transit.cpp"

int main() {
    string STOPS_FILE = "transit_data/stops.txt"; // default: "transit_data/stops.txt"
    string STOP_TIMES_FILE = "transit_data/stop_times_debug.txt"; // default: "transit_data/stop_times.txt"

    Transit transit(STOPS_FILE, STOP_TIMES_FILE);

    // TODO: CLI


    return 0;
}