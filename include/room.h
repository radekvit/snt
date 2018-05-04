#ifndef SNT_ROOM_H
#define SNT_ROOM_H
#include <vector>

class Room {
 public:
  using Feature = size_t;

  Room(const std::vector<Feature>& f) : features_(f) {}

  const std::vector<Feature>& features() { return features_; }

 private:
  std::vector<Feature> features_;
};

#endif