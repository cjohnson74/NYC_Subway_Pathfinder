#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <regex>
#include <set>
using namespace std;


class Transit {
    private:
        unordered_map<string, set<pair<string, int>>> routes; // Format: stopA -> set of <stopB, time>
        unordered_map<string, string> stop_id_map; // id -> name
        unordered_map<string, string> stop_name_map; // name -> id

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

            int debug_count = 0;

            while(!times_file.eof()) {
                debug_count += 1;
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

                    // bug fix: timeA is before midnight, but timeB is after midnight,
                    //          so offset timeB by 24 hours (86400 seconds).
                    if (time_total < 0) {
                        timeB += 86400;
                        time_total = timeB - timeA;
                    }

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

        bool stopExists(string& stop_name) {
            return stop_name_map.find(stop_name) != stop_name_map.end();
        }


        // TODO: Dijksta's Shortest Path Algorithm
        // Determines shortest route, then prints route and the time to perform the algorithm.
        // void shortest_path_dijksta(string& stopA, string& stopB) { }


        // TODO: A* Search Shortest Path Algorithm
        // Determines shortest route, then prints route and the time to perform the algorithm.
        // void shortest_path_a_star(string& stopA, string& stopB) { }
};