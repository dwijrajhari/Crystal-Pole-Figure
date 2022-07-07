#ifndef M3D_SYMMETRY
#define M3D_SYMMETRY

#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


class MillerDirection;

class Symmetry
{
public:
	std::vector<std::pair<glm::quat, bool>> operations;

	Symmetry();
	void fAddOp(glm::quat op_, bool improper = false);
};

Symmetry fOuterUnique(Symmetry& a, Symmetry& b);
Symmetry fCombineSymmetry(int num, ...);

std::vector<MillerDirection> operator*(const Symmetry&, const MillerDirection&);
Symmetry fCreateSymmetry(int num, ...);

#endif





