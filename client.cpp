#include <iostream>
#include <signal.h>

// include path to files if not in current directory
#include "cpp-httplib-master/httplib.h" // for HTTP
#include "json-develop/single_include/nlohmann/json.hpp" // for JSON

using json = nlohmann::ordered_json;
using namespace std;

#define HOST "localhost" // use if simulator is running on localhost:80

// parses string and returns vector of substrings
vector<string> parser(string s, char c) {
    string intermediate;
    vector<string> tokens;
    stringstream check1(s);
    while (getline(check1, intermediate, c)) {
        tokens.push_back(intermediate);
    }
    return tokens;
}

// returns vector of light ids
vector<string> getIds(vector<string> lights) {
    vector<string> ids;

    // iterate through the vector containing each light's id and name as one string
    for (int i = 0; i < lights.size(); i++) {

        // parse the string and add the light's id to ids vector
        // parseLights[0] = "" since substring starts with "
        vector<string> parsedLights = parser(lights[i], '"');
        ids.push_back(parsedLights[1]);
    }
    return ids;
}

// returns vector of light names
vector<string> getNames(vector<string> lights) {
    vector<string> names;

    // iterate through the vector containing each light's id and name as one string
    for (int i = 0; i < lights.size(); i++) {
        
        // find substring that starts after "name":
        string key = "\"name\":";
        int pos = lights[i].find(key);
        string substr = lights[i].substr(pos+key.length(), lights[i].length() - (pos+key.length()));

        // parse substring to add light's name to names vector
        // parseLights[0] = "" since substring starts with "
        vector<string> parsedLights = parser(substr, '"');
        names.push_back(parsedLights[1]);
    }
    return names;
}

// finds if light is on/off and adds it to the switches vector
void addSwitch(vector<bool> &switches, string state) {

    // find substring that starts after "on":
    string key = "\"on\":";
    int pos = state.find(key);
    string substr = state.substr(pos+key.length(), state.length() - (pos+key.length()));

    // parse substring to get light's on/off state
    vector<string> parsedState = parser(substr, ',');

    // convert string to boolean and add to switches vector
    if (parsedState[0] == "false" || parsedState[0] == "0")
        switches.push_back(false);
    else if (parsedState[0] == "true" || parsedState[0] == "1")
        switches.push_back(true);
    else {
        cerr << "Error: \"on\" state is not set to a boolean.";
        exit(1);
    }
}

// finds the brightness of a light and adds it to the brightness vector
void addBrightness(vector<int> &brightness, string state) {

    // find substring that starts after "bri":
    string key = "\"bri\":";
    int pos = state.find(key);
    string substr = state.substr(pos+key.length(), state.length() - (pos+key.length()));

    // parse substring to get light's brightness
    vector<string> parsedState = parser(substr, ',');

    // check if brightness is set to an integer
    for (int i = 0; i < parsedState[0].length(); i++) {
        if (!isdigit(parsedState[0][i])) {
            cerr << "Error: \"bri\" state is not set to an integer." << endl;
            exit(1);
        }
    }

    // convert brightness to a percentage -> brightness range: (0, 254)
    int bri = stoi(parsedState[0]) / 2.54;

    // send error if brightness is too high or low
    if (bri < 0) {
        cerr << "Error: brightness is set below 0%." << endl;
        exit(1);
    } else if (bri > 100) {
        cerr << "Error: brightness is set above 100%." << endl;
        exit(1);
    }

    // add brightness to brightness vector
    brightness.push_back(bri);
}

// get the on/off states of the lights
vector<bool> getSwitches(vector<string> lights, vector<string> ids) {
    httplib::Client cli(HOST);
    vector<bool> switches;
    for (int i = 0; i < lights.size(); i++) {

        // construct url that holds the light's states
        string url = "/api/newdeveloper/lights/" + ids[i];
        int n = url.length();
        char url_arr[n+1];
        strcpy(url_arr, url.c_str());

        // get all the light's states as a string
        auto res = cli.Get(url_arr);
        string state;
        if (res) {
            state = res->body;
        } else {
            cerr << "error: " << httplib::to_string(res.error()) << endl;
            exit(1);
        }

        // call function that finds the on/off state and adds it to switches vector
        addSwitch(switches, state);
    }
    return switches;
}

// get the brightness states of the lights
vector<int> getBrightness(vector<string> lights, vector<string> ids) {
    httplib::Client cli(HOST);
    vector<int> brightness;
    for (int i = 0; i < lights.size(); i++) {

        // construct url that holds the light's states
        string url = "/api/newdeveloper/lights/" + ids[i];
        int n = url.length();
        char url_arr[n+1];
        strcpy(url_arr, url.c_str()); 
        
        // get all the light's states as a string
        auto res = cli.Get(url_arr);
        string state;
        if (res) {
            state = res->body;
        } else {
            cerr << "error: " << httplib::to_string(res.error()) << endl;
            exit(1);
        }

        // call function that finds the brightness state and adds it to brightness vector
        addBrightness(brightness, state);
    }
    return brightness;
}

// gets strings each containing both ids and names of all the lights
vector<string> getLights() {
    httplib::Client cli(HOST);
    auto res1 = cli.Get("/api/newdeveloper/lights");
    string s;
    if (res1) {
        s = res1->body;
    } else {
        cerr << "error: " << httplib::to_string(res1.error()) << endl;
        exit(1);
    }

    // remove the braces at start and front of string and separate it into individual lights
    vector<string> lights = parser(s.substr(1, s.length()-2), ',');
    
    return lights;
}

// prints all lights and their states formatted as json
void printLights(vector<string> names, vector<string> ids, vector<bool> switches, vector<int> brightness) {
    
    // create json to hold all lights and states
    json allLights;
    for (int i = 0; i < ids.size(); i++) {

        // create json for each individual light
        string name = names[i];
        string id = ids[i];
        bool onOff = switches[i];
        int bri = brightness[i];
        json eachLight = {
            {"name", name},
            {"id", id},
            {"on", onOff},
            {"brightness", bri}
        };

        // add the json for individual lights to the json that holds all the lights
        allLights.push_back(eachLight);  
    }

    // pretty print
    cout << setw(4) << allLights << endl;
}

// prints update to on/off state of a light
void printSwitchUpdate(vector<string> ids, vector<bool> switches, int i) {
    string id = ids[i];
    bool onOff = switches[i];
    json update = {
        {"id", id},
        {"on", onOff},
    };
    cout << setw(4) << update << endl;
}

// prints update to brightness state of a light
void printBrightnessUpdate(vector<string> ids, vector<int> brightness, int i) {
    string id = ids[i];
    int bri = brightness[i];
    json update = {
        {"id", id},
        {"brightness", bri},
    };
    cout << setw(4) << update << endl;
}

// exits program gracefully
void signalHandler(int num) {
    cout << endl << "closing connection..." << endl;
    exit(num);
}

// checks for updates to on/off or brightness state of all the lights
void checkForUpdates(vector<string> lights, vector<string> ids, vector<bool> switches, vector<int> brightness) {

    // can exit infinite while loop gracefully with ctrl+c
    signal(SIGINT, signalHandler);

    // infinite while loop that checks for updates every second
    while (1) {
        sleep(1);

        // checks if on/off state has been modified
        vector<bool> newSwitches = getSwitches(lights, ids);
        for (int i = 0; i < switches.size(); i++) {
            if (newSwitches[i] != switches[i]) {
                switches = newSwitches;
                printSwitchUpdate(ids, switches, i);
            }
        }

        // checks if brightness state has been modified
        vector<int> newBrightness = getBrightness(lights, ids);
        for (int i = 0; i < brightness.size(); i++) {
            if (newBrightness[i] != brightness[i]) {
                brightness = newBrightness;
                printBrightnessUpdate(ids, brightness, i);
            }
        }
    }
}

int main() {

    // get all lights
    vector<string> lights = getLights();
    
    // get ids of all lights
    vector<string> ids = getIds(lights);

    // get names of all lights
    vector<string> names = getNames(lights);

    // get the states of the lights
    vector<bool> switches = getSwitches(lights, ids);
    vector<int> brightness = getBrightness(lights, ids);

    // print all lights and their states
    printLights(names, ids, switches, brightness);

    // check for updates to the lights' states and print them
    checkForUpdates(lights, ids, switches, brightness);

    return 0;
}