#ifndef HBMO_ETP_H
#define HBMO_ETP_H

#include <course_timetabling.h>
#include <deque>
#include <set>
#include <tuple>
#include <utility>

class Queen {
 public:
  using Bee = CourseSolution;

  Queen(const TimetablingProblem& ttp) : body(ttp) {}
  size_t sperm_count() const { return sperms.size(); }
  void add_sperm(Bee& sperm) { sperms.push_back(sperm); }
  void clear_sperms() { sperms.clear(); }
  vector<Bee> sperms;
  Bee body;
};

class HbmoEtp {
 public:
  using Bee = Queen::Bee;

  HbmoEtp(const TimetablingProblem& problem)
      : queen(problem), problem(problem) {
    calculate_course_conflicts();
  }

  CourseSolution run();

  vector<size_t> m_feasible_timeslots(const CourseSolution& sln, int course);
  int m_course_conflicts(int course);

  Queen queen;
  size_t iteration = 0;
 private:
  const TimetablingProblem& problem;
  size_t queenConflicts;

  vector<Bee> dronePopulation;
  vector<size_t> droneConflicts;
  vector<bool> courseConflicts;

  vector<Bee> broodPopulation;

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

  void calculate_course_conflicts();
  void create_drone_population();
  size_t compare_conflicts(size_t cq, size_t cb);
  CourseSolution generate_brood(const CourseSolution& a,
                                const CourseSolution& b);
  void simple_descent(Bee& brood);
  void shake_kempe_chain();
  std::pair<std::set<int>, std::set<int>> get_kempe_chain(
      const CourseSolution& brood, unsigned T1, unsigned T2);
  bool find_room(int course, const vector<int>& slot, size_t& result);
  std::tuple<CourseSolution, size_t, size_t> random_select_drone();
  void slotCrossover(CourseSolution& result, const vector<int>& aSlot,
                     const vector<int>& bSlot, vector<int>& resultSlot);
  bool conflicts_with(int course, const vector<int>& slot);
  void heuristic_sort(const CourseSolution& sln, std::deque<int>& courses);
  void heuristic_sort_low_prio(std::deque<int>& courses);
};

#endif