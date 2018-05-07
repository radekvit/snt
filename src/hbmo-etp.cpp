#include <hbmo-etp.h>
#include <random.h>
#include <algorithm>
#include <cmath>
#include <deque>
#include <numeric>
#include <set>

using std::vector;
using std::set;

void HbmoEtp::calculate_course_conflicts() {
  courseConflicts = vector<bool>(problem.nCourses * problem.nCourses, false);
  for (size_t i = 0; i < problem.nCourses; ++i) {
    for (auto&& student : problem.students()) {
      if (!(student.attendance()[i])) {
        continue;
      }
      for (size_t j = 0; j < problem.nCourses; ++j) {
        if (student.attendance()[j]) {
          courseConflicts[i * problem.nCourses + j] = true;
        }
      }
    }
  }
}

CourseSolution HbmoEtp::run() {
  using std::exp;

  size_t lastConflicts = 0;
  create_drone_population();
  broodPopulation.reserve(queenSpermLimit * 4);
  
  std::cerr << "Finished creating initial population\n";
  calculate_drones_conflicts();
  select_queen();
  // mating flight
  for (iteration = 0; iteration < matingFlights && queenConflicts > conflictThreshold; ++iteration) {
    if( lastConflicts != queenConflicts) {
      std::cerr << "mating iteration " << iteration << "; best penalty " << queenConflicts << "\n";
      lastConflicts = queenConflicts;
    }
    //set<size_t> selectedDrones;
    double energy = snt_rand(0.5, 1.0);
    size_t t = 0;

    if(dronePopulation.size() == 0) {
      break;
    }

    while (energy > epsilon && queen.sperm_count() < queenSpermLimit) {
      auto && [ drone, sdroneConflicts, i ] = random_select_drone();
      /*if (selectedDrones.count(i)) {
        // try again
        continue;
      }*/
      size_t conflicts_diff = queenConflicts - sdroneConflicts;
      double r = snt_rand(0.0, 1.0);
      if (exp(-(conflicts_diff / energy)) < r) {
        //selectedDrones.insert(i);
        queen.add_sperm(drone);
      }
      ++t;
      energy *= alpha;
    }
    for (auto&& sperm : queen.sperms) {
      auto&& brood = generate_brood(queen.body, sperm);
      simple_descent(brood);
      size_t broodConflicts = conflicts(brood);
      if (broodConflicts < queenConflicts) {
        std::swap(queen.body, brood);
        std::swap(queenConflicts, broodConflicts);
      }
      // insert the brood to the brood pool
      broodPopulation.push_back(brood);
    }
    // kill all old drones and replace with shaken broods
    // 4 times the broods
    auto copy = broodPopulation;
    std::copy(copy.begin(), copy.end(), std::back_inserter(broodPopulation));
    copy = broodPopulation;
    std::copy(copy.begin(), copy.end(), std::back_inserter(broodPopulation));
    shake_kempe_chain();
    dronePopulation.swap(broodPopulation);
    calculate_drones_conflicts();
    /*
    // replace the selected drones with the new ones
    for (size_t i = 0; i < broodPopulation.size(); ++i) {
      size_t j = *(selectedDrones.begin());
      droneConflicts[j] = conflicts(broodPopulation[i]);
      dronePopulation[j].swap(broodPopulation[i]);
      selectedDrones.erase(selectedDrones.begin());
    }
    */
    broodPopulation.clear();
    queen.clear_sperms();
  }

  std::cerr << "Done. Result penalty is " << queenConflicts << "\n";

  return queen.body;
}

// shake each brood with the kempe chain
void HbmoEtp::shake_kempe_chain() {
  for (auto&& brood : broodPopulation) {
  retry:
    // choose random timeslots
    unsigned T1 = snt_rand(CourseSolution::slotCount);
    unsigned T2 = snt_rand(CourseSolution::slotCount - 1);
    if (T2 >= T1) {
      ++T2;
    }
    // build kempe chain of conflicting lectures from both
    auto && [ T1Lectures, T2Lectures ] = get_kempe_chain(brood, T1, T2);
    auto&& slot1 = brood.slots()[T1];
    auto&& slot2 = brood.slots()[T2];
    // swap lectures
    // erase selected from their timeslots
    for (auto&& room : brood.slots()[T1]) {
      if (T1Lectures.count(room) != 0) {
        room = -1;
      }
    }
    for (auto&& room : brood.slots()[T2]) {
      if (T2Lectures.count(room) != 0) {
        room = -1;
      }
    }
    // place lectures to the new timeslot
    // if a room cannot be assigned, try selecting different timeslots
    for (auto&& course : T1Lectures) {
      size_t room = 0;
      if (!find_room(course, slot2, room)) {
        goto retry;
      }
      slot2[room] = course;
    }

    for (auto&& course : T2Lectures) {
      size_t room = 0;
      if (!find_room(course, slot1, room)) {
        goto retry;
      }
      slot1[room] = course;
    }
  }
}

std::pair<set<int>, set<int>> HbmoEtp::get_kempe_chain(
    const CourseSolution& brood, unsigned T1, unsigned T2) {
  auto&& slot1 = brood.slots()[T1];
  auto&& slot2 = brood.slots()[T2];

  set<int> T1Chain;
  set<int> T2Chain;

  // randomly select from first
  vector<unsigned> T1Selections(slot1.size());
  std::iota(T1Selections.begin(), T1Selections.end(), 0);

  while (true) {
    if (T1Selections.empty()) {
      // slot1 is empty, just place every event from slot2 to slot1
      for (auto&& course : slot2) {
        if (course != -1) {
          T2Chain.insert(course);
        }
      }
      return {T1Chain, T2Chain};
    }
    size_t i = snt_rand(unsigned(T1Selections.size()));
    T1Selections.erase(T1Selections.begin() + i);
    if (slot1[i] != -1) {
      T1Chain.insert(slot1[i]);
      break;
    }
  }

  // keep adding blocking courses until the size settles
  size_t T1Size = 1;
  size_t T2Size = 0;

  while (T1Size != T1Chain.size() || T2Size != T2Chain.size()) {
    T1Size = T1Chain.size();
    T2Size = T2Chain.size();

    for (auto&& course : T1Chain) {
      for (auto&& course2 : slot2) {
        if (course2 == -1) {
          continue;
        }
        if (courseConflicts[course * problem.nCourses + course2]) {
          T2Chain.insert(course2);
        }
      }
    }

    for (auto&& course : T2Chain) {
      for (auto&& course2 : slot1) {
        if (course2 == -1) {
          continue;
        }
        if (courseConflicts[course * problem.nCourses + course2]) {
          T1Chain.insert(course2);
        }
      }
    }
  }

  return {T1Chain, T2Chain};
}

size_t HbmoEtp::compare_conflicts(size_t cq, size_t cb) {
  if (cq > cb) {
    return cq - cb;
  } else {
    return cb - cq;
  }
}

std::tuple<CourseSolution, size_t, size_t> HbmoEtp::random_select_drone() {
  auto i = snt_rand((unsigned)dronePopulation.size());
  return {dronePopulation[i], droneConflicts[i], i};
}

CourseSolution HbmoEtp::generate_brood(const CourseSolution& a,
                                       const CourseSolution& b) {
  vector<unsigned> slotPool1(CourseSolution::slotCount);
  std::iota(slotPool1.begin(), slotPool1.end(), 0);
  vector<unsigned> slotPool2(slotPool1);
  // pick x different timeslots from each parent
  std::array<unsigned, crossoverGenes> T1;
  std::array<unsigned, crossoverGenes> T2;
  for (unsigned& t : T1) {
    size_t i = snt_rand((unsigned)slotPool1.size());
    t = slotPool1[i];
    slotPool1.erase(slotPool1.begin() + i);
  }
  for (unsigned& t : T2) {
    size_t i = snt_rand((unsigned)slotPool2.size());
    t = slotPool2[i];
    slotPool2.erase(slotPool2.begin() + i);
  }
  CourseSolution result{a};

  for (size_t i = 0; i < T1.size(); ++i) {
    slotCrossover(result, a.slots()[T1[i]], b.slots()[T1[i]],
                  result.slots()[T1[i]]);
  }

  return result;
}

void HbmoEtp::slotCrossover(CourseSolution& result, const vector<int>& aSlot,
                            const vector<int>& bSlot, vector<int>& resultSlot) {
  for (auto&& bCourse : bSlot) {
    if (bCourse == -1) {
      continue;
    }
    // do not move if already present
    for (auto&& aCourse : aSlot) {
      if (aCourse == bCourse) {
        goto continueLabel;
      }
    }
    // do not move if causes conflict
    // 1) check for conflicts
    if (conflicts_with(bCourse, aSlot)) {
      continue;
    }
    // 2) find a suitable room
    size_t room;
    if (!find_room(bCourse, resultSlot, room)) {
      continue;
    }

    // erase the event from the other timeslot
    for (auto&& slot : result.slots()) {
      for (auto&& course : slot) {
        if (course == bCourse) {
          course = -1;
        }
      }
    }
    // place the event
    resultSlot[room] = bCourse;
  // hack for "continue 2;", please C++20 add this feature
  continueLabel:;
  }
}

// always finds the smallest possible room with the least features
bool HbmoEtp::find_room(int course, const vector<int>& slot, size_t& result) {
  bool ret = false;
  size_t minCapacity = -1;
  size_t minFeatures = -1;
  auto&& courseProperties = problem.courses()[course];
  for (size_t i = 0; i < slot.size(); ++i) {
    // check if empty
    if (slot[i] != -1) {
      continue;
    }
    // check size
    if (problem.rooms()[i].size() < courseProperties.students) {
      continue;
    }
    // check for features
    size_t roomFeatures = 0;
    auto&& features = problem.rooms()[i].features();
    for (size_t j = 0; j < features.size(); ++j) {
      if (courseProperties.requiredFeatures()[j] && !features[j]) {
        goto continueLabel;
      }
      if (features[j]) {
        ++roomFeatures;
      }
    }
    ret = true;
    if (problem.rooms()[i].size() < minCapacity ||
        (problem.rooms()[i].size() == minCapacity &&
         roomFeatures < minFeatures)) {
      minCapacity = problem.rooms()[i].size();
      minFeatures = roomFeatures;
      result = i;
    }
  continueLabel:;
  }
  return ret;
}

bool HbmoEtp::conflicts_with(int course, const vector<int>& slot) {
  for (auto&& course2 : slot) {
    if (course2 == -1) {
      continue;
    }
    if (courseConflicts[course * problem.nCourses + course2]) {
      return true;
    }
  }
  return false;
}

void HbmoEtp::simple_descent(Bee& brood) {
  // randomly move an event to a random slot if possible; keep the change if it
  // improves the fitness
  for (size_t i = 0; i < simpleDescentIterations; ++i) {
    // randomly select a course
    int selectedCourse = snt_rand(problem.nCourses);
    // find the event's timeslot
    unsigned tSlotSource = 0;
    unsigned tRoomSource = 0;
    for (size_t i = 0; i < CourseSolution::slotCount; ++i) {
      for (size_t j = 0; j < problem.rooms().size(); ++j) {
        int course = brood.slots()[i][j];
        if (course == selectedCourse) {
          tSlotSource = i;
          tRoomSource = j;
          goto breakLabel;
        }
      }
    }
  breakLabel:
    // randomly select a different timeslot
    unsigned tSlotTarget = snt_rand(CourseSolution::slotCount - 1);
    if (tSlotTarget >= tSlotSource) {
      ++tSlotTarget;
    }
    // try to assign to that timeslot
    if (conflicts_with(selectedCourse, brood.slots()[tSlotTarget])) {
      continue;
    }
    // 2) find a suitable room
    size_t room;
    if (!find_room(selectedCourse, brood.slots()[tSlotTarget], room)) {
      continue;
    }
    Bee temp{brood};
    // place the event and erase if from the original place
    temp.slots()[tSlotSource][tRoomSource] = -1;
    temp.slots()[tSlotTarget][room] = selectedCourse;
    // if this is better, store this change
    if (conflicts(temp) < conflicts(brood)) {
      brood.swap(temp);
    }
  }
}

void HbmoEtp::create_drone_population() {
  dronePopulation.clear();
  CourseSolution emptySolution{problem};
  std::deque<int> courses(problem.nCourses, 0);
  std::iota(courses.begin(), courses.end(), 0);

  // we do these only once to save lots of time; potential reordering is insignificant as
  // the number of feasible rooms changes by 1
  // the number of tries is higher, but the speed increase would justify even
  // ~3 tries on average per drone
  heuristic_sort_low_prio(courses);
  heuristic_sort(emptySolution, courses);

  // generate the drones by randomly assigning to a timeslot
  for (size_t i = 0; i < droneNumber; ++i) {
  restartLabel:
    std::cerr << "creating initial solution " << i << "\n";
    CourseSolution s{emptySolution};
    std::deque<int> c{courses};
    while (!c.empty()) {
      auto selected = c.front();
      c.pop_front();
      auto slots = m_feasible_timeslots(s, selected);
      if (slots.empty()) {
        // cannot assign anywhere, try again
        // maybe some rollback?
        goto restartLabel;
      }
      size_t slot = slots[snt_rand((unsigned)slots.size())];
      size_t room = -1;
      // always succeeds
      find_room(selected, s.slots()[slot], room);
      s.slots()[slot][room] = selected;
      heuristic_sort(s, c);
    }
    dronePopulation.push_back(s);
  }
}

int course_conflicts(HbmoEtp& hbmo, int course) {
  return hbmo.m_course_conflicts(course);
}

int feasible_timeslots(HbmoEtp& hbmo, const CourseSolution& sln, int course) {
  return hbmo.m_feasible_timeslots(sln, course).size();
}

void HbmoEtp::heuristic_sort_low_prio(std::deque<int>& courses) {
  // lowest priority sort: largest number of students first
  auto studentCount = [&](int a, int b) {
    return problem.courses()[a].students > problem.courses()[b].students;
  };
  std::sort(courses.begin(), courses.end(), studentCount);
  // middle priority sort: highest number of conflicts first
  // TODO save this locally instead of counting every time
  auto conflictL = [&](int a, int b) {
    return m_course_conflicts(a) > m_course_conflicts(b);
  };
  std::stable_sort(courses.begin(), courses.end(), conflictL);
}

void HbmoEtp::heuristic_sort(const CourseSolution& sln,
                             std::deque<int>& courses) {
  // high priority sort: lowest number of feasible timeslots
  std::stable_sort(courses.begin(), courses.end(), [&](int a, int b) {
    return feasible_timeslots(*this, sln, a) <
           feasible_timeslots(*this, sln, b);
  });
}

int HbmoEtp::m_course_conflicts(int course) {
  // count the number of events this event has common students with
  set<int> conflictingClasses;
  for (auto&& student : problem.students()) {
    if (!student.attendance()[course]) {
      continue;
    }
    for (size_t i = 0; i < student.attendance().size(); ++i) {
      if (student.attendance()[i]) {
        conflictingClasses.insert(i);
      }
    }
  }
  // always conflicts with itself
  return conflictingClasses.size() - 1;
}

vector<size_t> HbmoEtp::m_feasible_timeslots(const CourseSolution& sln,
                                             int course) {
  vector<size_t> r;
  for (size_t i = 0; i < sln.slots().size(); ++i) {
    size_t dummy;
    auto&& timeslot = sln.slots()[i];
    if (!conflicts_with(course, timeslot) &&
        find_room(course, timeslot, dummy)) {
      r.push_back(i);
    }
  }
  return r;
}