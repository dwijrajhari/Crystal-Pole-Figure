#ifndef M3D_CRYSTALS
#define M3D_CRYSTALS

#include <glm/glm.hpp>

enum class LatticeSystem
{
	UNDEFINED, TRICLINIC, MONOCLINIC, ORTHORHOMBIC, TETRAGONAL, HEXAGONAL, RHOMBOHEDRAL, CUBIC
};

class Lattice
{
public:
	LatticeSystem mLatticeSystem = LatticeSystem::UNDEFINED;
	float a, b, c, alpha, beta, gamma;
	glm::vec3 basis[3];
	glm::vec3 reciprocal[3];

	Lattice();
	Lattice(float a_, float b_, float c_, float alpha_ = 90.0f, float beta_ = 90.0f, float gamma_ = 90.0f);

	void fGenerateBasis();
	void fGenerateLatticeSystem();
};

struct CrystalFace
{
	glm::vec3 coords[4];
};

class Symmetry;

class Crystal2
{
public:
	Lattice lattice;
	Symmetry symmetry();
	CrystalFace faces[6];
	glm::vec3 centroid;
	glm::vec3 orientation = { 0, 0, 0 };
	float radius;

	Crystal2();
	Crystal2(Lattice);
	void fGeneratePlane(float*, int, int, int, bool*);
	void fGenerateCrystalRenderData(float*, float*);

};

class Crystal
{
public:
	CrystalFace faces[6];


	Lattice lattice;
	glm::vec3 centroid;
	glm::vec3 orientation = {0, 0, 0};
	float radius;

	Crystal();
	Crystal(Lattice);
	void fGeneratePlane(float*, int, int, int, bool*);
	void fGenerateCrystalRenderData(float*, float*);

};

class MillerDirection
{
public:
	glm::vec3 uvw;

	MillerDirection(glm::vec3, Crystal* = nullptr);
	MillerDirection(int u = 0, int v = 0, int w = 0, Crystal* = nullptr);
};

MillerDirection operator*(const glm::quat&, const MillerDirection&);

#endif



