#ifndef SNT_CONFIG_H
#define SNT_CONFIG_H

#include <cstdlib>

// suggested values:
// number of drones = 40
// max mating flights = 10'000
// size of spermtheca = 10
// number of broods = 10
// number of selected crossover genes = 8
// simple descent iterations = 5000

inline constexpr double alpha = 0.9;
inline constexpr double epsilon = 0.0000001;

inline constexpr size_t droneNumber = 40;
// I don't have 4 hours for each run
inline constexpr size_t matingFlights = 2'000;
inline constexpr int queenSpermLimit = 10;
inline constexpr size_t crossoverGenes = 8;
inline constexpr size_t simpleDescentIterations = 5000;

inline constexpr size_t conflictThreshold = 0;

#endif