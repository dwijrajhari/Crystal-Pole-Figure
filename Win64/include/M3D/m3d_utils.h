#ifndef M3D_UTILS
#define M3D_UTILS

#include <string>

class Symmetry;
enum class LatticeSystem;

std::string LS_enum2str(LatticeSystem& ls);
void print_symmetry(Symmetry& s);

#endif