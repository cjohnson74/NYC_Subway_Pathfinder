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
#include <utility>
#include <chrono>
#include <cmath>

using namespace std;


class Transit {
    private:
        unordered_map<string, set<pair<string, int>>> routes; // Format: stopA -> set of <stopB, time>
        unordered_map<string, string> stop_id_map; // id -> name
        unordered_map<string, string> stop_name_map; // name -> id
        unordered_map<string, float> stop_time_map;
        unordered_map<string, pair<double, double>> stop_pos_map; // id -> pair of <latitude, longitude>
        bool constructionComplete = false; // Flag to track transit construction status

    public:
        Transit() {

        }

        // GTFS parsing of transit data & map population.
        void buildTransit(string& filepath_stops, string& filepath_times) {
            // 1. Read stops.txt and populate stop_id and stop_name maps.
            ifstream stops_file(filepath_stops);
            std::ifstream file("transit_data/stops.txt");
            if (!file.is_open()) {
                std::cerr << "Error opening file" << std::endl;
            }
            string stop_id, stop_name, stop_lat, stop_lon, junk; // junk is for unneeded data
            int currLine = 1;

            cout << "Reading file: " << filepath_stops << endl;

            getline(stops_file, junk); // remove first line

            while(!stops_file.eof()) {
                getline(stops_file, stop_id, ',');
                getline(stops_file, junk, ',');
                getline(stops_file, stop_name, ',');
                getline(stops_file, junk, ',');
                getline(stops_file, stop_lat, ',');
                getline(stops_file, stop_lon, ',');
                getline(stops_file, junk);
                currLine++;

                // TODO: BUG because one stop_name can map to MULTIPLE stop_ids.
                stop_id_map[stop_id] = stop_name;
                stop_name_map[stop_name] = stop_id;
                stop_pos_map[stop_id] = make_pair(stod(stop_lat), stod(stop_lon));
                // cout << stop_id << ": " << stop_name << endl;

                displayLoadingBar(currLine, 1505);
                cout << "Reading line " << currLine << " of " << 1505 << "\r";
                cout.flush();
            }
            cout << endl << filepath_stops << " file reading completed!" << endl;

            // 2. Read stop_times.txt, then calculate and populate route adjacency list graph.
            ifstream times_file(filepath_times);
            string stopA_id, stopB_id, stopA_time, stopB_time, stopA_seq, stopB_seq;
            int timeA, timeB, time_total;
            currLine = 1;

            cout << "Reading file: " << filepath_times << endl;

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
                currLine++;

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

                    insertRoute(stopA_id, stopB_id, timeA, timeB);
                }
                displayLoadingBar(currLine, 554833);
                cout << "Reading line " << currLine << " of " << 554833 << "\r";
                cout.flush();
            }
            cout << endl << filepath_times << "file reading completed!" << endl;
        }

        // aesthetic function that outputs a loading bar in CLI since it takes so long to read the stop_times.txt file
        void displayLoadingBar(int current, int total) {
            int barWidth = 70;
            float progress = (float)current / total;
            int pos = barWidth * progress;

            // Clear previous output
            cout << "\r";

            // Output loading bar
            cout << "[\033[32m";
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) cout << "#";
                else if (i == pos) cout << "#";
                else cout << " ";
            }
            cout << "\033[0m] " << setw(3) << int(progress * 100.0) << "% ";
            cout.flush();
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

        void insertRoute(string& stopA, string& stopB, int& timeA, int& timeB) {
            pair<string, int> route;
            route.first = stopB;
            route.second = timeB - timeA;

            // Fix: Sometimes there are routes with identical paths (stopA/stopB), but with varying travel times, leading
            // to duplicate entries for the same path. We decided to choose the SHORTEST travel time amongst them.
            set<pair<string, int>> stopA_routes = routes[stopA];
            for (pair<string, int> stopB_route : stopA_routes) {
                if (stopB_route.first == stopB) {
                    // Duplicate found, only add if shorter.
                    if (route.second < stopB_route.second) {
                        routes[stopA].erase(stopB_route);
                        routes[stopA].insert(route);
                    }
                    stop_time_map[stopA] = float(timeA);
                    stop_time_map[stopB] = float(timeB);
                    return;
                }
            }
            
            // If no duplicate route found, add it to graph.
            routes[stopA].insert(route);
            stop_time_map[stopA] = float(timeA);
            stop_time_map[stopB] = float(timeB);
        }

        set<pair<string, int>> getAdjacents(string& stop) {
            return routes[stop];
        }

        string getStopName(string stop_id) {
            return stop_id_map[stop_id];
        }

        string getStopID(string stop_name) {
            return stop_name_map[stop_name];
        }

        void printRoutes() {
            for (auto each: routes) {
                cout << getStopName(each.first) << " : ";
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

        // get heuristic by using stop time to calculate how long it will take to get from start to end
        float get_time_heuristic(string& start_stop, string& end_stop) {
            return stop_time_map[end_stop] - stop_time_map[start_stop];
        }

        // get heuristic by using position (lat, lon) to calculate the manhattan distance or "L distance"
        float get_pos_heuristic(string& start_stop, string& end_stop) {
            // x -> longitude, y -> latitude
            float x1 = get<1>(stop_pos_map[start_stop]);
            float y1 = get<0>(stop_pos_map[start_stop]);
            float x2 = get<1>(stop_pos_map[end_stop]);
            float y2 = get<0>(stop_pos_map[end_stop]);

            // calculate and return manhattan distance
            return abs(x1 - x2) + abs(y1 - y2);
        }

        // Using positon (lat, lon) and the Haversine Formula to calculate the Heuristic
        float get_pos_haversine_heuristic(string& start_stop, string& end_stop) {
            const double R = 6371000.0; // Earth radius in meters
            const double pi = 3.14159265358979323866;

            // Get lat and long
            double start_lat = get<0>(stop_pos_map[start_stop]);
            double start_lon = get<1>(stop_pos_map[start_stop]);
            double end_lat = get<0>(stop_pos_map[end_stop]);
            double end_lon = get<1>(stop_pos_map[end_stop]);

            // Convert lat and long from degrees to radians
            double phi1 = start_lat * pi / 180.0;
            double phi2 = end_lat * pi / 180.0;
            double deltaPhi = (end_lat - start_lat) * pi / 180.0;
            double deltaLambda = (end_lon - start_lon) * pi / 180.0;

            // Calculate Haversine formula components
            double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
                        cos(phi1) * cos(phi2) *
                        sin(deltaLambda / 2) * sin(deltaLambda / 2);
            double c = 2 * atan2(sqrt(a), sqrt(1 - a));

            // Calculate distance
            double distance = R * c;
            return distance;
        }

        vector<string> get_shortest_path(unordered_map<string, string>& preds, string& curr_stop, string& start_stop) {
            vector<string> shortest_path = {curr_stop};
            while (curr_stop != start_stop) {
                shortest_path.push_back(preds[curr_stop]);
                curr_stop = preds[curr_stop];
            }

            reverse(shortest_path.begin(), shortest_path.end());

            return shortest_path;
        }

        // TODO: A* Search Shortest Path Algorithm
        // Determines shortest route, then prints route and the time to perform the algorithm.
        // returns tuple<path, time, run_time>
        tuple<vector<string>, float, chrono::microseconds, unordered_map<string, pair<double, double>>> shortest_path_a_star(string& stopA, string& stopB) {
            auto start = chrono::high_resolution_clock::now(); 
            int count = 0; // use count for tie breaker if something is already in the pq it takes precidence
            string start_stop = stopA;
            string end_stop = stopB;

            //init distance[v] and predecessor[v] and heuristic[v]
            unordered_map<string, float> time;
            unordered_map<string, string> pred;
            unordered_map<string, float> heur;
            priority_queue<tuple<int, int, string> , vector<tuple<int, int, string>>, greater<tuple<int, int, string>>> not_done;
            not_done.push(make_tuple(0, count, start_stop)); // (time to get to stop, count, stop)

            unordered_set<string> done = {};

            for (auto route : routes) {
                time[route.first] = INFINITY; // time to get to stop set to infinity
                pred[route.first] = "null"; // string (null) since we don't know it's predecessor yet
                heur[route.first] = INFINITY; // heuristic score set to inifinity
            }

            time[start_stop] = 0;
            heur[start_stop] = get_pos_heuristic(start_stop, end_stop);
            unordered_set<string> pq_ref_set = {start_stop}; // helps us see what is in the priority queue

            while (!pq_ref_set.empty()) {
                string curr_stop = get<2>(not_done.top());
                not_done.pop();
                pq_ref_set.erase(pq_ref_set.find(curr_stop));

                if (curr_stop == end_stop) {
                    auto stop = chrono::high_resolution_clock::now(); 
                    auto run_time = chrono::duration_cast<chrono::microseconds>(stop - start); 
                    return make_tuple(get_shortest_path(pred, end_stop, start_stop), time[curr_stop], run_time, stop_pos_map);
                }

                for (auto route : routes[curr_stop]) {
                    float curr_time = time[curr_stop] + route.second;

                    if (curr_time < time[route.first]) {
                        pred[route.first] = curr_stop;
                        time[route.first] = curr_time;
                        heur[route.first] = curr_time + get_pos_haversine_heuristic(route.first, end_stop);
                        if (pq_ref_set.find(route.first) == pq_ref_set.end()) {
                            count++; // increment count to keep track of order put into pq
                            not_done.push(make_tuple(heur[route.first], count, route.first)); // add neighbor to pq
                            pq_ref_set.insert(route.first); // add neighbor to pq ref
                        }
                    }
                }

                if (curr_stop != start_stop) 
                    done.insert(curr_stop);
            }

            auto stop = chrono::high_resolution_clock::now(); 
            auto run_time = chrono::duration_cast<chrono::microseconds>(stop - start);
            vector<string> shortest_path;
            return make_tuple(shortest_path, 0.0, run_time, stop_pos_map);   
        };
};