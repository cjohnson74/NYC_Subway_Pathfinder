/* Problem Statement: Given a transit system, determine fastest route from stop A to stop B.


Design Decisions:
input read from .txt file in GTFS format (eg. https://gtfs.org/schedule/reference/#stopstxt)

1. Map #1: Routes represented as an Adjacency List Data Structure
Transit System => map<Stop, vector<pair<Stop, int>>>
Stops => Stop class (stop_id, stop_name)
Route Time => int (seconds)
- chosen for ideal time and space efficiency

2. Stop class: Stop data (stop_id, stop_name)
- A Stop class is chosen to store each stop's data. While this requires more storage space and also needs complete
map traversals to find selected stops, it offers the benefit of returning Stops with all stop information needed.
The efficiency shines after the result is obtained. Otherwise, stop_id referencing would be needed again to get names.

3. All transit functionality within Transit class
- insertRoute, getAdjacent, getStopName, getStopID, shortest path algorithms (Dijksta, A* Search)
- constructor Transit(filepath) handles GTFS parsing and map populating

4. CLI menu
This will remain rooted in the main function.

Hierarchy:
main (CLI) -> Transit -> Map#1/Map#2

*/

#include "Transit.cpp"
#include "Stop.cpp"