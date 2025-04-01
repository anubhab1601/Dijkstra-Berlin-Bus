# Berlin Bus Network Analyzer 🚌

[![GitHub license](https://img.shields.io/github/license/anubhab1601/berlin-bus-analyzer)](LICENSE)
[![C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

A C program that analyzes the Berlin bus network using Dijkstra's algorithm to find shortest paths between nodes in the transportation network.

## Features ✨

- **CSV Parsing**: Reads Berlin bus network data from `berlin_network_bus.csv`
- **Graph Representation**: Builds an adjacency list from the network data
- **Path Finding**: Implements Dijkstra's algorithm to find shortest paths
- **Performance Metrics**: Tracks algorithm performance (comparisons, relaxations, etc.)
- **Multiple Outputs**: Generates:
  - Adjacency list (`berlin_list_bus.csv`)
  - Path results (`berlin_path.csv`)
  - Performance metrics (`performance.csv`)
  - Detailed path information (`output.csv`)

## Prerequisites 📋

- C compiler (gcc recommended)
- Make (optional, for build automation)
- Berlin bus network data file (`berlin_network_bus.csv`)

## Compilation & Running the Program 🚀

- gcc Berlin_new.c -o berlin-bus-analyzer
- .\berlin-bus-analyzer

## File Structure 📂

berlin-bus-analyzer/

├── src/

│   ├── main.c                  # Main program source

├── data/

│   ├── berlin_network_bus.csv  # Input data file

├── Makefile                    # Build automation

├── README.md                   # This file

├── LICENSE                     # License file

└── .gitignore                  # Git ignore rules

## Output Files 📄

- berlin_list_bus.csv: Generated adjacency list
- berlin_path.csv: Shortest paths from start node
- performance.csv: Algorithm performance metrics
- output.csv: Detailed path information with distances and parent nodes

## Algorithm Details 🧠

- The program implements **Dijkstra's algorithm** with:
- Priority queue (using array for simplicity)
- Distance and time metrics
- Performance tracking:
  - Number of comparisons
  - Relaxation operations
  - Parent changes
  - Execution time
 
## License 📜
- This project is licensed under the MIT License - see the LICENSE file for details.
