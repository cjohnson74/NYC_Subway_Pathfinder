#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <iomanip>
using namespace std;


class Stop {
    private:
        string stop_id;
        string stop_name;
    public:
        Stop() {

        }

        Stop(string& id, string& name) {
            stop_id = id;
            stop_name = name;
        }

        string getStopID() {
            return stop_id;
        }

        string getStopName() {
            return stop_name;
        }
};