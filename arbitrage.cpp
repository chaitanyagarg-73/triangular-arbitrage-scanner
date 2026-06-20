#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <curl/curl.h>
#include "json.hpp" // The single-header library we downloaded

using json = nlohmann::json;
using namespace std;

// Callback function required by libcurl to write the response into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch data from Binance
string getBinanceData() {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.binance.com/api/v3/ticker/price");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // Essential for Windows/SSL environments
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// Struct to represent the graph
typedef unordered_map<string, unordered_map<string, double>> Graph;

void runBellmanFord(const Graph& graph, const string& source) {
    unordered_map<string, double> distances;
    unordered_map<string, string> predecessors;
    
    // Initialize distances
    for (const auto& pair : graph) {
        distances[pair.first] = INFINITY;
        predecessors[pair.first] = "";
    }
    distances[source] = 0.0;

    int V = graph.size();

    // Relax edges V-1 times
    for (int i = 0; i < V - 1; i++) {
        for (const auto& u_pair : graph) {
            string u = u_pair.first;
            for (const auto& v_pair : u_pair.second) {
                string v = v_pair.first;
                double weight = v_pair.second;
                
                if (distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    predecessors[v] = u;
                }
            }
        }
    }

    // Step 3: Check for negative-weight cycles
    for (const auto& u_pair : graph) {
        string u = u_pair.first;
        for (const auto& v_pair : u_pair.second) {
            string v = v_pair.first;
            double weight = v_pair.second;
            
            // 1e-8 tolerance for floating point errors
            if (distances[u] + weight < distances[v] - 1e-8) { 
                cout << "\n🚨 ARBITRAGE OPPORTUNITY DETECTED! 🚨\n";
                
                vector<string> cycle;
                cycle.push_back(v);
                cycle.push_back(u);
                
                string curr = predecessors[u];
                while (find(cycle.begin(), cycle.end(), curr) == cycle.end()) {
                    cycle.push_back(curr);
                    curr = predecessors[curr];
                }
                
                // Extract just the cycle loop
                auto it = find(cycle.begin(), cycle.end(), curr);
                vector<string> final_cycle(it, cycle.end());
                reverse(final_cycle.begin(), final_cycle.end());
                final_cycle.push_back(final_cycle[0]); // Complete the loop
                
                for (size_t i = 0; i < final_cycle.size(); i++) {
                    cout << final_cycle[i];
                    if (i != final_cycle.size() - 1) cout << " -> ";
                }
                cout << endl;
                return;
            }
        }
    }
    cout << "No arbitrage found starting from " << source << ".\n";
}

int main() {
    cout << "Fetching live market data from Binance..." << endl;
    string responseString = getBinanceData();
    
    if (responseString.empty()) {
        cerr << "Failed to fetch data." << endl;
        return 1;
    }

    auto jsonData = json::parse(responseString);
    Graph marketGraph;
    unordered_set<string> allowedAssets = {"BTC", "ETH", "USDT", "BNB", "SOL", "XRP", "ADA", "DOGE"};

    // Parse JSON and build the graph
    for (const auto& item : jsonData) {
        string symbol = item["symbol"];
        double price = stod(item["price"].get<string>());
        
        if (price == 0) continue;

        for (const string& asset : allowedAssets) {
            // Check if symbol ends with the asset (Quote)
            if (symbol.length() > asset.length() && 
                symbol.compare(symbol.length() - asset.length(), asset.length(), asset) == 0) {
                
                string base = symbol.substr(0, symbol.length() - asset.length());
                string quote = asset;
                
                if (allowedAssets.count(base) && allowedAssets.count(quote)) {
                    marketGraph[base][quote] = -log(price);
                    marketGraph[quote][base] = -log(1.0 / price);
                }
                break;
            }
        }
    }

    cout << "Built graph with " << marketGraph.size() << " currencies." << endl;
    runBellmanFord(marketGraph, "USDT");

    return 0;
}