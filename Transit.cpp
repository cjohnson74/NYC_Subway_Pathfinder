#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <regex>
#include <set>
#include <unordered_set>
#include <unordered_map>
#define INFINITY INT_MAX

using namespace std;


class Transit {
    private:
        map<string, set<pair<string, int>>> routes; // Format: stopA -> set of <stopB, time>
        map<string, string> stop_id_map; // id -> name
        map<string, string> stop_name_map; // name -> id

    public:
        Transit() {

        }

        // GTFS parsing of transit data & map population.
        Transit(string& filepath_stops, string& filepath_times) {
            // 1. Read stops.txt and populate stop_id and stop_name maps.
            ifstream stops_file(filepath_stops);
            string stop_id, stop_name, junk; // junk is for unneeded data

            getline(stops_file, junk); // remove first line

            while(!stops_file.eof()) {
                getline(stops_file, stop_id, ',');
                getline(stops_file, junk, ',');
                getline(stops_file, stop_name, ',');
                getline(stops_file, junk);

                stop_id_map[stop_id] = stop_name;
                stop_name_map[stop_name] = stop_id;
            }

            // 2. Read stop_times.txt, then calculate and populate route adjacency list graph.
            ifstream times_file(filepath_times);
            string stopA_id, stopB_id, stopA_time, stopB_time, stopA_seq, stopB_seq;
            int timeA, timeB, time_total;

            getline(times_file, junk); // remove first line

            while(!times_file.eof()) {
                // Parsing Explanation: From the way the GTFS file is formatted, each stop has a
                // sequence number. If a stop's seq is exactly +1 greater than the line immediately
                // before it, that means it is a route (stopA -> stopB), where the previous line is
                // stopA, and the current line is stopB.
                // Each group (trip) of routes start from sequence number 1. Therefore, all lines that
                // have a sequence number >1 is guaranteed to complete a route, since there will always
                // be a lower (-1) sequence number immediately before it.
                stopA_id = stopB_id;
                stopA_time = stopB_time;
                stopA_seq = stopB_seq;

                getline(times_file, junk, ',');
                getline(times_file, stopB_time, ',');
                getline(times_file, junk, ',');
                getline(times_file, stopB_id, ',');
                getline(times_file, stopB_seq, ',');
                getline(times_file, junk);

                if (stopB_seq != "1") {
                    // Parse and calculate time between stops.
                    timeA = convertToSeconds(stopA_time);
                    timeB = convertToSeconds(stopB_time);
                    time_total = timeB - timeA;

                    // TODO: possible bug if timeA is before midnight and timeB is after (negative time_total)

                    insertRoute(stopA_id, stopB_id, time_total); 
                }
            }

        }

        // Takes a time format: HH:MM:SS, and converts to total seconds.
        int convertToSeconds(string& time) {
            int hours, minutes, seconds;
            regex timeReg("(\\d{2}):(\\d{2}:(\\d{2}))");
            smatch timeComponents;

            if (regex_search(time, timeComponents, timeReg)) {
                hours = stoi(timeComponents[1]);
                minutes = stoi(timeComponents[2]);
                seconds = stoi(timeComponents[3]);
            }

            return (hours * 3600) + (minutes * 60) + (seconds);
        }

        void insertRoute(string& stopA, string& stopB, int& time) {
            pair<string, int> route;
            route.first = stopB;
            route.second = time;

            // Fix: Sometimes there are duplicate routes with identical locations, just at different times of day.
            // But the total route time doesn't change, therefore it isn't necessary to update it, or add a duplicate.
            // That means the vector of adjacents can actually be a set, to avoid duplicates.
            routes[stopA].insert(route);
        }

        set<pair<string, int>> getAdjacents(string& stop) {
            return routes[stop];
        }

        string getStopName(string& stop_id) {
            return stop_id_map[stop_id];
        }

        string getStopID(string& stop_name) {
            return stop_name_map[stop_name];
        }

        unordered_set<string> getSetOfAllStops() {
            unordered_set<string> allStops = {};

            //for every vertex in the graph, place in allStops
            for (auto each: routes) {
                allStops.insert(each.first);
            }

            return allStops;
        }

        string minDistance(unordered_map<string, int> distances, unordered_set<string> not_done) {
            pair<string, int> minPair = {"", INFINITY};
            for (auto stop: not_done) {
                if (distances[stop] < minPair.second) {
                    minPair.first = stop;
                    minPair.second = distances[stop];
                }
            }

            return minPair.first;
        }

        // TODO: Dijksta's Shortest Path Algorithm
        // Determines shortest route, then prints route and the time to perform the algorithm.
        int shortest_path_dijkstra(string& stopA, string& stopB) {
            //source vertex
            string src = stopA;

            //sets -> done and not_done
            unordered_set<string> done = {}; //empty
            unordered_set<string> not_done = getSetOfAllStops(); //all stops

            //init d[v] and p[v]
            unordered_map<string, int> dist;
            unordered_map<string, string> pred;
            for (auto each: not_done) {
                dist[each] = INFINITY;
                //setting each val of pred to the string (null) for validation
                pred[each] = "null";
                //duplicates?
            }
            //set d[src] to 0
            dist[src] = 0;

            //while not_done is not empty
            while (!not_done.empty()) {
                //get min in not_done
                string currStop = minDistance(dist, not_done);
                not_done.erase(currStop);
                done.insert(currStop);
                //use unordered set instead of ordered set
                set<pair<string, int>> adjacentStops = getAdjacents(currStop);
                for (auto stop: adjacentStops) {
                    if (dist[stop.first] > dist[currStop] + stop.second) {
                        dist[stop.first] = dist[currStop] + stop.second;
                        pred[stop.first] = currStop;
                    }
                }

            }

            //return the distance to the stop specified
            return dist[stopB];
        }


        // TODO: A* Search Shortest Path Algorithm
        // Determines shortest route, then prints route and the time to perform the algorithm.
        // void shortest_path_a_star(string& stopA, string& stopB) { }
};