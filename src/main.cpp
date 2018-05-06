#include <hbmo-etp.h>
int main() {
  TimetablingProblem tp;
  HbmoEtp solver(tp);
  solver.run().print();
  return 0;
}
