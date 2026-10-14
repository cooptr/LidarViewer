#include <iostream>
#include <vector>
#include "ply_loader.hpp"
#include "renderer.hpp"

int main() {
  std::cout << "sizeof(Point) = " << sizeof(Point) << '\n';
  render_test("bun_zipper.ply");
  return 0;
}