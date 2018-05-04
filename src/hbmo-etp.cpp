#include <hbmo-etp.h>
#include <cmath>

CourseSolution HbmoEtp::run() {
  using std::exp;
  // number of drones = 40
  // max mating flights = 10'000
  // size of spermtheca = 10
  // number of broods = 10
  // number of selected crossover genes = 8
  // simple descent iterations = 5000
  create_drone_population();
  calculate_drones_conflicts();
  select_queen();
  // mating flight
  for (size_t i = 0; i < matingFlights; ++i) {
    double energy = rand(0.5, 1.0);
    size_t t = 0;

    while (energy > 0 || queen.sperm_count() < queen::spermLimit) {
      auto && [ drone, sdroneConflicts ] = random_select_drone();
      size_t conflicts_diff =
          compare_conflicts(queenConflicts, sdroneConflicts);
      double r = rand(0.0, 1.0);
      if (exp(-(conflicts_diff / energy)) < r) {
        queen.add_sperm(drone);
      }
      ++t;
      energy *= alpha;
    }

    for (auto&& sperm : queen.sperms()) {
      auto&& brood = generate_brood(queen, sperm);
      simple_descent(brood);
      size_t broodConflicts = conflicts(brood);
      if (broodConflicts < queenConflicts) {
        queen.body = brood;
        queenConflicts = broodConflicts;
      } else {
        // to population
        broodPopulation.push_back(brood);
      }
    }
    shake_kemp_chain(broodPopulation);
    dronePopulation.swap(broodPopulation);
    calculate_drones_conflicts();
    broodPopulation.clear();
    queen.clear_sperms();
  }

  return queen.body;
}

size_t HbmoEtp::compare_conflicts(size_t cq, size_t cb) {
  if (cq > cb) {
    return cq - cb;
  } else {
    return cb - cq;
  }
}

std::pair<Bee, size_t> random_select_drone() {
  auto i = rand(dronePopulation.size());
  return {dronePopulation[i], droneConflicts[i]};
}

CourseSolution HbmoEtp::generate_brood(const CourseSolution& a,
                                       const CourseSolution& b) {
  // pick two different timeslots from each parent
  unsigned aT1 = rand(CourseSolution::slotCount);
  unsigned aT2 = rand(CourseSolution::slotCount - 1);
  if (aT2 >= aT1) {
    ++aT2;
  }
  unsigned bT1 = rand(CourseSolution::slotCount);
  unsigned bT2 = rand(CourseSolution::slotCount - 1);
  if (bT2 >= bT1) {
    ++bT2;
  }
  CourseSolution result{a};
  // attempt to move the courses from bT1 to aT1
  slotCrossover(result, a.slots()[aT1], b.slots()[bT1], result.slots()[aT1]);
  // attempt to move the courses from bT2 to aT2
  slotCrossover(result, a.slots()[aT2], b.slots()[bT2], result.slots()[aT2]);

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
    if(conflicts_with(bCourse, aSlot) {
      continue;
		}
		// 2) find a suitable room
		size_t room;
		if (!find_room(bCourse, resultSlot, room)) {
      continue;
		}

		// erase the event from the other timeslot
		for(auto&& slot: result.slots()) {
      for (auto&& course : slot) {
        if (course == bCourse) {
          course = -1;
        }
      }
		}
		// place the event
		resultSlot[room] = bCourse;
		// hack for "continue 2;", please C++20 add this feature
		continueLabel:
  }
}

bool HbmoEtp::find_room(int course, const vector<int>& slot, size_t& result) {
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
    auto&& features = problem.rooms()[i].features();
    for (size_t j = 0; j < features.size(); ++j) {
      if (courseProperties.requiredFeatures()[j] && !features[j]) {
        goto continueLabel;
      }
    }
    result = i;
    return true;
  continueLabel:
  }
  return false;
}

bool HbmoEtp::conflicts_with(int course, const vector<int>& slot) {
  for (auto&& student : problem.students()) {
    if (!(student.attendance()[course])) {
      continue;
    }
    for (auto&& course : slot) {
      if (course == -1) {
        continue;
      }
      if (student.attendance()[course]) {
        return true;
      }
    }
  }
  return false;
}

void HbmoEtp::simple_descent(Bee& brood) {
  // randomly move an event to a random slot if possible; keep the change if it
  // improves the fitness
  for (size_t i = 0; i < dsIterations; ++i) {
    // randomly select a course
    unsigned selectedCourse = rand(problem.courses().size());
    // find the event's timeslot
    unsigned tSlotSource = 0;
    unsigned tRoomSource = 0;
    for (size_t i = 0; i < slotCount; ++i) {
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
    unsigned tSlotTarget = rand(CourseSolution::slotCount - 1);
    if (tSlotTarget >= tSlotSource) {
      ++tSlotTarget;
    }
    // try to assign to that timeslot
    if(conflicts_with(selectedCourse, brood.slots()[tSlotTarget]) {
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