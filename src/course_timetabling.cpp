#include <course_timetabling.h>
#include <map>

CourseSolution::CourseSolution(const TimetablingProblem& ttp) {
  slots_.fill(vector<int>(ttp.rooms().size(), -1));
}

void CourseSolution::print() {
  using std::cout;
  std::map<size_t, std::pair<size_t, size_t>> mappings;
  for (size_t i = 0; i < slots_.size(); ++i) {
    for (size_t j = 0; j < slots_[i].size(); ++j) {
      if (slots_[i][j] != -1) {
        mappings[slots_[i][j]] = {i, j};
      }
    }
  }
  for (auto&& resPair : mappings) {
    cout << resPair.second.first << ' ' << resPair.second.second << "\n";
  }
}
