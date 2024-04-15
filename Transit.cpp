#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include "Stop.cpp"
using namespace std;


class Transit {
    private:
        map<Stop, vector<pair<Stop, int>>> routes; // Format: Stop -> vector of <Stop, time>

    public:
        Transit() {

        }

        // GTFS parsing of transit data & map population.
        Transit(string& filepath) {
            
        }

        void insertRoute(string& stopA, string& stopB, int& time) {

        }

        vector<pair<Stop, int>> getAdjacents(Stop& stop) {
            return routes[stop];
        }

        string getStopName(Stop& stop) {
            return stop.getStopName();
        }

        string getStopID(Stop& stop) {
            return stop.getStopID();
        }

        // TODO: Dijksta's Shortest Path Algorithm
        // Determines shortest route, prints stops, prints time to perform calculation.
        // vector<Stop> shortest_path_dijksta(Stop stopA, Stop stopB) {

        // }

        // TODO: A* Search Shortest Path Algorithm
        // Determines shortest route, prints stops, prints time to perform calculation.
        // vector<Stop> shortest_path_a_star(Stop stopA, Stop stopB) {

        // }
};