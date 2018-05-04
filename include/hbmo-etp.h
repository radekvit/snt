#ifndef HBMO_ETP_H
#define HBMO_ETP_H

#include <course_timetabling.h>
#include <utility>

class Queen {
 public:
  using std::vector;
  using Bee = CourseSolution;
  static constexpr spermLimit = 10;

  Queen() {}
  size_t sperm_count() const { return sperms.size(); }
  void add_sperm(Bee& sperm) { sperms.push_back(&sperm); }
  clear_sperms() { sperms.clear(); }
  vector<Bee> sperms() {
    vector<Bee> result;
    result.reserve(sperms.size());
    for (auto beePtr : sperms) {
      result.push_back(*beePtr);
    }
    return result;
  }

  Bee body;

 private:
  vector<Bee*> sperms;
};

class HbmoEtp {
 public:
  using Bee = Queen::Bee;
  using std::vector;

  HbmoEtp(const TimetablingProblem& problem) : problem(problem) {}

  CourseSolution run();

 private:
  const TimetablingProblem& problem;
  Queen queen;
  size_t queenConflicts;

  vector<Bee> dronePopulation;
  vector<size_t> droneConflicts;

  vector<Bee> broodPopulation;
  constexpr double alpha = 0.9;
  constexpr size_t sdIterations = 5000;

  size_t conflicts(const CourseSolution& s) { return problem.conflicts(s); }

  void calculate_drones_conflicts() {
    droneConflicts.clear();

    for (auto&& drone : dronePopulation) {
      droneConflicts.push_back(conflicts(drone));
    }
  }

  void select_queen() {
    Bee* best = nullptr;
    queenConflicts = -1;
    for (size_t i = 0; i < dronePopulation.size(); ++i) {
      if (droneConflicts[i] < queenConflicts) {
        queenConflicts = droneConflicts[i];
        best = &(dronePopulation[i]);
      }
    }
    queen.body = *best;
  }
};

#endif