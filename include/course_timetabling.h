#ifndef SNT_COURSE_TIMETABLING_H
#define SNT_COURSE_TIMETABLING_H
#include <array>
#include <cstdint>
#include <vector>

class CourseSolution {
 public:
  static constexpr int dayCount = 5;
  static constexpr int slotsPerDay = 9;
  static constexpr int slotCount = dayCount * slotsPerDay;
  // index represents a room, integer represents a course
  using std::array;
  using std::vector;
  // -1 signifies empty room
  using Slot = vector<int>;
  using Slots = array<Slot, slotCount>;

  CourseSolution() {}
  CourseSolution(const Slots& s) : slots_(s) {}
  CourseSolution(Slots&& s) : slots_(s) {}

  Slots& slots() { return slots_; }
  const Slots& slots() const { return slots_; }

  void swap(CourseSolution& other) { slots_.swap(other.slots_); }

 private:
  Slots slots_;
};

class Room {
 public:
  using std::vector;

  Room(size_t s) : size_(s) {}

  size_t size() const { return size_; }
  vector<bool>& features() { return features_; }
  const vector<bool>& features() const { return features_; }

 private:
  size_t size_;
  vector<bool> features_;
};

class Student {
 public:
  using std::vector;

  Student(){};
  Student(const vector<bool>& a) : attendance_(a) {}
  Student(vector<bool>&& a) : attendance_(a) {}

  const vector<bool>& attendance() const { return attendance_; }
  vector<bool>& attendance() { return attendance_; }

 private:
  vector<bool> attendance_;
};

class Course {
 public:
  using std::vector;

  Course(const vector<bool>& f) : requiredFeatures_(f) {}

  vector<bool>& requiredFeatures() { return requiredFeatures_; }
  const vector<bool>& requiredFeatures() const { return requiredFeatures_; }
  size_t students;

 private:
  vector<bool> requiredFeatures_;
};

class TimetablingProblem {
 public:
  using std::vector;
  using std::cout;
  using std::cin;

  TimetablingProblem() { read_from_stdin(); }

  bool feasible(const CourseSolution& s) const {
    return true;  // TODO?
  }

  // helper for studentAvailibility indexing
  inline bool available(const vector<bool>& a, int slot, size_t student,
                        size_t students) const {
    return a[slot * students + student];
  }

  // only considering feasible solutions
  // modified implementation of checksln.cpp
  size_t conflicts(const CourseSolution& s) const {
    size_t softConflicts = 0;
    vector<bool> studentAvailability { students_.size() * slotCount, true }
    // get student availibility
    for (size_t s = 0; s < slotCount; ++s) {
      auto&& slot = s.slots()[s];
      for (size_t st = 0; st < students_.size(); ++st) {
        auto&& schedule = students_[st].attendance();
        for (auto&& room : slots) {
          if (room == -1) {
            continue;
          }
          if (schedule[room]) {
            studentAvailability[s * students_.size() + st] = false;
          }
        }
      }
    }
    // more than two consecutive lectures
    for (size_t g = 0; g < students_.size(); ++g) {
      for (int day = 0; d < dayCount; ++d) {
        int count = 0;

        for (int t = 0; t < slotsPerDay; ++t) {
          int slot = d * slotsPerDay + t;
          if (available(studentAvailability, slot, g, students_.size()) ==
              false)
            count++;
          else
            count = 0;
          if (count >= 3) {
            ++softConflicts;
          }
        }
      }
    }

    // single lecture in a day
    for (size_t g = 0; g < students_.size(); g++) {
      for (int d = 0; d < 5; d++) {
        int count = 0;
        for (int t = 0; t < 9; t++) {
          int slot = d * 9 + t;
          if (available(studentAvailability, slot, g, students_.size()) ==
              false) {
            count++;
          }
        }
        if (count == 1) {
          ++softConflicts;
        }
      }
    }

    // end of day lectures
    for (size_t g = 0; g < students_.size(); g++) {
      if (available(studentAvailability, 8, g, students_.size()) == false) {
        ++softConflicts;
      }
      if (available(studentAvailability, 17, g, students_.size()) == false) {
        ++softConflicts;
      }
      if (available(studentAvailability, 26, g, students_.size()) == false) {
        ++softConflicts;
      }
      if (available(studentAvailability, 35, g, students_.size()) == false) {
        ++softConflicts;
      }
      if (available(studentAvailability, 44, g, students_.size()) == false) {
        ++softConflicts;
      }
    }

    return softConflicts;
  }

  const vector<Course>& courses() const { return courses_; }
  const vector<Room>& rooms() const {return rooms_};

 private:
  vector<Room> rooms_;
  vector<Student> students_;
  vector<Course> courses_;
  size_t features_;

  void read_from_stdin() {
    using std::cin;
    size_t rooms;
    size_t students;
    size_t courses;
    cin >> courses >> rooms >> features_ >> students;
    // room sizes
    size_t roomSize;
    for (size_t i = 0; i < rooms; ++i) {
      cin >> roomSize;
      rooms_.emplace_back(roomSize);
    }
    // student-course attendance
    bool attending;
    vector<size_t> coursesAttendance{students, 0};
    for (size_t i = 0; i < students; ++i) {
      std::vector<bool> attendance;
      for (size_t j = 0; j < courses; ++j) {
        cin >> attending;
        coursesAttendance[j] += attending;
        attendance.push_back(attending);
      }
      students_.emplace_back(attendance);
    }
    // room-feature
    bool hasFeature;
    for (size_t i = 0; i < rooms; ++i) {
      auto&& features = rooms_[i].features();
      for (size_t j = 0; j < features; ++j) {
        cin >> hasFeature;
        features.push_back(hasFeature);
      }
    }
    // course-feature
    bool requiresFeature;
    for (size_t i = 0; i < courses; ++i) {
      std::vector<bool> requiredFeatures;
      for (size_t j = 0; j < features; ++j) {
        cin >> requiresFeature;
        requiredFeatures.push_back(requiresFeature);
      }
      courses_.emplace_back(requiredFeatures);
      courses_.back().students = coursesAttendance[i];
    }
  }
};

#endif