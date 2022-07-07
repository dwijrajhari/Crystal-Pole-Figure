#include <M3D\m3d_crystals.h>

constexpr auto eps = 1e-7;
# define feq2(A, B) (glm::abs((A)-(B)) < eps)
# define fneq2(A, B) (!feq2(A, B))
# define feq3(A, B, C) (feq2(A, B) && feq2(B, C))

Lattice::Lattice()
	:a(1.0f), b(1.0f), c(1.0f), alpha(90.0f), beta(90.0f), gamma(90.0f) {
	fGenerateBasis();
	fGenerateLatticeSystem();
}

Lattice::Lattice(float a_, float b_, float c_, float alpha_, float beta_, float gamma_)
	: a(a_), b(b_), c(c_), alpha(alpha_), beta(beta_), gamma(gamma_) {
	fGenerateBasis();
	fGenerateLatticeSystem();
}

void Lattice::fGenerateBasis()
{
	float alphaR = glm::radians(alpha);
	float betaR = glm::radians(beta);
	float gammaR = glm::radians(gamma);

	basis[0] = glm::vec3(a, 0.0f, 0.0f);
	basis[1] = glm::vec3(b * glm::cos(gammaR), b * glm::sin(gammaR), 0.0f);

	float cx, cy, cz;
	cx = c * glm::cos(betaR);
	cy = c * ((glm::cos(alphaR) - glm::cos(betaR) * glm::cos(gammaR)) / glm::sin(gammaR));
	cz = glm::sqrt(c * c - cx * cx - cy * cy);

	basis[2] = glm::vec3(cx, cy, cz);

	for (int i = 0; i < 3; i++)
	{
		glm::vec3 temp = glm::cross(basis[(i + 1) % 3], basis[(i + 2) % 3]);
		reciprocal[i] = temp / glm::dot(basis[i], temp);
	}
}

void Lattice::fGenerateLatticeSystem()
{
	if (feq3(a, b, c) && feq3(alpha, beta, gamma))
	{
		if (feq2(alpha, 90.0f))
		{
			mLatticeSystem = LatticeSystem::CUBIC;
		}
		else
		{
			mLatticeSystem = LatticeSystem::RHOMBOHEDRAL;
		}
	}
	
	else if ((feq2(a, b) && fneq2(a, c) && feq2(alpha, 120.0f) && feq3(beta, gamma, 90.0f)) ||
		(feq2(b, c) && fneq2(a, b) && feq2(beta, 120.0f) && feq3(alpha, gamma, 90.0f)) ||
		(feq2(c, a) && fneq2(a, b) && feq2(gamma, 120.0f) && feq3(beta, alpha, 90.0f)))
	{
		mLatticeSystem = LatticeSystem::HEXAGONAL;
	}

	else if (feq3(alpha, beta, gamma))
	{
		if ((feq2(a, b) && fneq2(a, c)) || (feq2(b, c) && fneq2(a, b)) || (feq2(c, a) && fneq2(a, b)) && feq2(alpha, 90.0f))
		{
			mLatticeSystem = LatticeSystem::TETRAGONAL;
		}
		else if ((fneq2(a, b) && fneq2(a, c) && fneq2(b, c)))
		{
			if (feq2(alpha, 90.0f))
			{
				mLatticeSystem = LatticeSystem::ORTHORHOMBIC;
			}
			else
			{
				mLatticeSystem = LatticeSystem::MONOCLINIC;
			}
		}
	}
	else
	{
		mLatticeSystem = LatticeSystem::TRICLINIC;
	}
}

Crystal::Crystal()
	:lattice(Lattice()) { }

Crystal::Crystal(Lattice lattice_ = Lattice())
	:lattice(lattice_) { }

void Crystal::fGenerateCrystalRenderData(float* data = nullptr, float* tt = nullptr)
{
	glm::vec3 A, B, C, o, oa, oab, ob, oc, oca, ocab, ocb;
	lattice.fGenerateBasis();
	lattice.fGenerateLatticeSystem();
	A = lattice.basis[0];
	B = lattice.basis[1];
	C = lattice.basis[2];

	o = glm::vec3(0.0f);
	oa = A;
	ob = B;
	oc = C;
	oab = A + B;
	oca = C + A;
	ocb = C + B;
	ocab = oca + B;
	centroid = (float)0.125 * (o + oa + ob + oc + oab + oca + ocb + ocab);
	radius = glm::length(centroid - ocab);

	faces[0] = CrystalFace({ o, ob, oab, oa });
	faces[1] = CrystalFace({ o, oa, oca, oc });
	faces[2] = CrystalFace({ o, oc, ocb, ob });
	faces[3] = CrystalFace({ ocab, ocb, oc, oca });
	faces[4] = CrystalFace({ ocab, oca, oa, oab });
	faces[5] = CrystalFace({ ocab, oab, ob, ocb });

	if (data != nullptr)
	{
		int p = 0;
		for (int face = 0; face < 6; face++)
		{
			for (int vertex = 0; vertex < 3; vertex++)
			{
				memcpy(data + p, &faces[face].coords[vertex], 3 * sizeof(float));
				p += 3;

			}
			for (int vertex = 2; vertex < 5; vertex++)
			{
				memcpy(data + p, &faces[face].coords[vertex % 4], 3 * sizeof(float));
				p += 3;
			}
		}
	}

	if (tt != nullptr)
	{
		float ttVertices[] = {
		  o.x,   o.y,   o.z,
		oca.x, oca.y, oca.z,	
		ocb.x, ocb.y, ocb.z,

		  o.x,   o.y,   o.z,
		ocb.x, ocb.y, ocb.z,
		oab.x, oab.y, oab.z,

		  o.x,   o.y,   o.z,	
		oab.x, oab.y, oab.z,	
		oca.x, oca.y, oca.z,

		oab.x, oab.y, oab.z,
		ocb.x, ocb.y, ocb.z,
		oca.x, oca.y, oca.z

		};
		memcpy(tt, ttVertices, 36 * sizeof(float));
	}
}

void Crystal::fGeneratePlane(float *data, int h, int k, int l, bool *isTri)
{
	*isTri = 0;
	float hkl[3] = { (1 / (float)h), (1 / (float)k), (1 / (float)l) };
	glm::vec3 vertData[4];

	if (!(h == 0 || k == 0 || l == 0))
	{
		*isTri = 1;
		
		vertData[0] = hkl[0] * lattice.basis[0];
		vertData[1] = hkl[1] * lattice.basis[1];
		vertData[2] = hkl[2] * lattice.basis[2];
		vertData[3] = vertData[0];

	}
	else
	{
		int idx = 0;
		bool twoZero = 1;

		if      (k == 0 && l == 0)	{	idx = 0;	}
		else if (h == 0 && l == 0)	{	idx = 1;	}
		else if (h == 0 && k == 0)	{	idx = 2;	}
		else { twoZero = 0; }

		if (twoZero)
		{
			vertData[0] = hkl[idx] * lattice.basis[idx];
			vertData[1] = vertData[0] + lattice.basis[(idx + 1) % 3];
			vertData[2] = vertData[1] + lattice.basis[(idx + 2) % 3];
			vertData[3] = vertData[0] + lattice.basis[(idx + 2) % 3];
		}
		else
		{
			if		(h == 0) { idx = 0; }
			else if (k == 0) { idx = 1; }
			else if (l == 0) { idx = 2; }

			vertData[0] = hkl[(idx + 1) % 3] * lattice.basis[(idx + 1) % 3];
			vertData[1] = hkl[(idx + 2) % 3] * lattice.basis[(idx + 2) % 3];
			vertData[2] = vertData[1] + lattice.basis[idx];
			vertData[3] = vertData[0] + lattice.basis[idx];
		}
	}

	// If plane is out of the unit cell, shift the origin
	for (int i = 0; i < 3; i++)
	{
		if (hkl[i] < 0)
		{
			for (int j = 0; j < 4; j++)
			{
				vertData[j] += lattice.basis[i];
			}
		}
	}
	memcpy(data, vertData, sizeof(vertData));
	
}

MillerDirection::MillerDirection(glm::vec3 uvw_, Crystal*)
{
	uvw = glm::round(uvw_);
}

MillerDirection::MillerDirection(int u, int v, int w, Crystal*)
{
	uvw.x = u;
	uvw.y = v;
	uvw.z = w;
}

MillerDirection operator*(const glm::quat& q, const MillerDirection& d)
{
	return MillerDirection(q * glm::vec3(d.uvw));
}

