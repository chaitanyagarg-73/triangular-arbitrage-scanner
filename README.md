# High-Frequency Triangular Arbitrage Scanner

A low-latency C++ algorithmic trading scanner that detects real-time triangular arbitrage opportunities across cryptocurrency markets.

## Overview
This engine streams live order book data via the Binance REST API, models the exchange multi-asset pairs as a directed graph, and mathematically detects market inefficiencies. 

To convert the arbitrage problem into a shortest-path graph problem, the exchange rates are transformed using negative logarithms. The Bellman-Ford algorithm is then applied to detect negative-weight cycles, which directly correlate to a profitable arbitrage loop (e.g., ADA -> ETH -> SOL -> BTC -> ADA).

## Tech Stack
* **Language:** C++17
* **Network/API:** libcurl (for low-latency HTTP requests)
* **JSON Parsing:** nlohmann/json
* **Algorithms:** Directed Graphs, Bellman-Ford Cycle Detection

## Mathematical Model
If we trade Coin A -> Coin B -> Coin C -> Coin A, an arbitrage exists if:
(Rate A->B) * (Rate B->C) * (Rate C->A) > 1

By taking the negative logarithm, this inequality becomes:
-log(Rate A->B) - log(Rate B->C) - log(Rate C->A) < 0

This allows the scanner to treat currencies as nodes and exchange rates as edge weights, using cycle detection to find sums less than zero.

## Build Instructions (Windows/MSYS2)
Ensure you have `g++` and `libcurl` installed via MSYS2.
1. `git clone <your-repo-link>`
2. `cd <your-repo-folder>`
3. `g++ -std=c++17 arbitrage.cpp -lcurl -o arbitrage`
4. `./arbitrage.exe`
