#include <course_timetabling.h>
#include <map>

CourseSolution::CourseSolution(const TimetablingProblem& ttp) {
  slots_.fill(vector<int>(ttp.rooms().size(), -1));
}

void CourseSolution::print() {
	using std::cout;
	std::map<size_t, size_t> mappings;
	for (size_t i = 0; i < slots_.size(); ++i) {
		for (size_t j = 0; j < slots_[i].size(); ++j) {
			mappings[i] = j;
		}
	}
	for(auto&& resPair: mappings) {
		cout << resPair.first << ' ' << resPair.second << "\n";
	}
}
