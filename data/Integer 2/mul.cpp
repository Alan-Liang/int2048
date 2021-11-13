#include "int2048.hpp"

using sjtu::int2048;

int main () {
  int2048 a("34567898765456");
  int2048 b("765467898765467890");
  std::cout << a * b << std::endl;
  return 0;
}
