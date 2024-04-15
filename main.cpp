/* Problem Statement: Given a transit system, determine fastest route from stop A to stop B.


Design Decisions:
input read from .txt file in GTFS format (eg. https://gtfs.org/schedule/reference/#stopstxt)

1. Map #1: Routes represented as an Adjacency List Data Structure
Transit System => map<string, vector<pair<string, int>>>
Stops => string (stop_id)
Route Time => int (seconds)
- chosen for ideal time and space efficiency

2. Map #2: Stops data (stop_id, name)
name => stop_id (string)
- A separate map will be created to index names to stop_ids. This makes it so that it will be simple and quick
  to search for the stopA after the user inputs it on the CLI. (Creating a Node class with stop_id makes it harder
  because have to iterate through all Nodes to find it).

3. All transit functionality within Transit class
- map creation (insertion), getAdjacent, getTime, getStopName, getStopID, shortest path algorithms

4. CLI menu
This will remain rooted in the main function.

Hierarchy:
main (CLI) -> Transit -> Map#1/Map#2

*/
