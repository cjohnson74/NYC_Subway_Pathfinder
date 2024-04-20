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
#include <unordered_set>
#include <unordered_map>
#include<cmath>
#include <queue>
#include <stack>


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

                // TODO: BUG because one stop_name can map to MULTIPLE stop_ids.
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

            // Fix: Sometimes there are routes with identical paths (stopA/stopB), but with varying travel times, leading
            // to duplicate entries for the same path. We decided to choose the SHORTEST travel time amongst them.
            set<pair<string, int>> stopA_routes = routes[stopA];
            for (pair<string, int> stopB_route : stopA_routes) {
                if (stopB_route.first == stopB) {
                    // Duplicate found, only add if shorter.
                    if (time < stopB_route.second) {
                        routes[stopA].erase(stopB_route);
                        routes[stopA].insert(route);
                    }

                    return;
                }
            }
            
            // If no duplicate route found, add it to graph.
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

        void printRoutes() {
            for (auto each: routes) {
                cout << each.first << " : ";
                set<pair<string, int>> adjs = routes[each.first];
                for (auto i: adjs) {
                    cout << "(" << i.first << " " << i.second << ")"<< " ";
                }
                cout << endl;
            }
        }

        bool validPath(string stopA, string stopB) {
            stack<string> s;
            set<string> visited;
            string src = stopA;
            string dest = stopB;
            s.push(src);

            //dfs to find if reachable stop
            while (!s.empty()) {
                string curr = s.top();
                visited.insert(curr);
                s.pop();

                set<pair<string, int>> adjacents = getAdjacents(curr);
                for (auto each: adjacents) {
                    if (each.first == dest) {
                        return true;
                    }
                    if (!visited.count(each.first)) {
                        s.push(each.first);
                    }
                }       
            }

            return false;
        }
        bool stopExists(string& stop_name) {
            return stop_name_map.find(stop_name) != stop_name_map.end();
        }

        void updatePriority(unordered_set<string>& done, unordered_map<string, int>& distances, priority_queue<pair<int, string> , vector<pair<int, string>>, greater<pair<int, string>>>& minHeap) {
            //https://stackoverflow.com/questions/2852140/priority-queue-clear-method
            minHeap = priority_queue<pair<int, string> , vector<pair<int, string>>, greater<pair<int, string>>>(); // reset
            for (auto i: distances) {
                if (!done.count(i.first)) {
                    minHeap.push({i.second, i.first});
                }
            }
        }


        // TODO: Dijksta's Shortest Path Algorithm
        // Determines shortest route, then prints route and the time to perform the algorithm.
        int shortest_path_dijkstra(string stopA, string stopB) {
            //source vertex
            string src = stopA;

            //init d[v] and p[v]
            unordered_map<string, int> dist;
            unordered_map<string, string> pred;
            for (auto each: routes) {
                dist[each.first] = INFINITY;
                //setting each val of pred to the string (null) for validation
                pred[each.first] = "null";
                //duplicates?
            }

            //set d[src] to 0
            dist[src] = 0;

            //init not_done heap and done set;
            unordered_set<string> done = {}; //empty
            //making the distance first for simplicity of sorting ::: pair -> {distance, stop}
            priority_queue<pair<int, string> , vector<pair<int, string>>, greater<pair<int, string>>> not_done;

            updatePriority(done, dist, not_done);
            
            //while not_done is not empty
            while (!not_done.empty()) {
                //get min in not_done
                string currStop = not_done.top().second;
                not_done.pop();
                done.insert(currStop);
                //use unordered set instead of ordered set
                set<pair<string, int>> adjacentStops = getAdjacents(currStop);
                for (auto stop: adjacentStops) {
                    if (dist[stop.first] > dist[currStop] + stop.second) { // relax edges
                        int newDist = dist[currStop] + stop.second;
                        dist[stop.first] = newDist;
                        pred[stop.first] = currStop;                        
                        updatePriority(done, dist, not_done);
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