#include <M3D/m3d_crystals.h>
#include <M3D/m3d_symmetry.h>
#include <cstdarg>


constexpr auto eps = 1e-5;
#define _qeq(A, B) ( glm::abs(A.x-B.x)<eps && glm::abs(A.y-B.y)<eps && glm::abs(A.z-B.z)<eps && glm::abs(A.w-B.w)<eps )
#define  qeq(A, B) ( _qeq(A, B) || _qeq(A, (-B)) )

Symmetry::Symmetry()
{

}

void Symmetry::fAddOp(glm::quat op_, bool improper)
{
	operations.push_back({ op_, improper });
}

Symmetry fOuterUnique(Symmetry& a, Symmetry& b)
{
	Symmetry res;
	for (const std::pair<glm::quat, bool>& aq : a.operations)
	{
		for (const std::pair<glm::quat, bool>& bq : b.operations)
		{
			glm::quat newOp = aq.first * bq.first;
			bool unique = true;
			for (const std::pair<glm::quat, bool>& cq : res.operations)
			{
				if (qeq(cq.first, newOp))
				{
					unique = false;
					break;
				}
			}
			if (unique) res.operations.push_back({ newOp , aq.second ^ bq.second });
			
		}
	}
	return res;
}

Symmetry _fCombineSymmetry(Symmetry& a, Symmetry& b)
{
	Symmetry res = fOuterUnique(a, b);
	int size = 1;
	int size_new = res.operations.size();
	while (size != size_new && size_new < 48)
	{
		size = size_new;
		res = fOuterUnique(res, res);
		size_new = res.operations.size();
	}
	return res;
}

Symmetry fCombineSymmetry(int num, ...)
{
	Symmetry result;
	result.fAddOp(glm::quat(1.0, 0.0, 0.0, 0.0));

	va_list valist;
	va_start(valist, num);
	for (int i = 0; i < num; i++)
	{
		result = _fCombineSymmetry(result, va_arg(valist, Symmetry));
	}
	va_end(valist);
	return result;
}

std::vector<MillerDirection> operator*(const Symmetry& s, const MillerDirection& m)
{
	std::vector<MillerDirection> eqv;
	for (const std::pair<glm::quat, bool>& q : s.operations)
	{
		glm::vec3 temp = q.first * m.uvw;
		bool unique = true;
		for (const MillerDirection& m0 : eqv)
		{
			if (glm::all(glm::lessThan(glm::abs(temp - m0.uvw), glm::vec3(1e-5))))
			{
				unique = false;
				break;
			}
		}
		if (unique)
		{
			eqv.push_back(temp);
		}
	}
	return eqv;
}

Symmetry fCreateSymmetry(int num, ...)
{
	va_list valist;
	Symmetry result;
	va_start(valist, num);
	for (int i = 0; i < num; i++)
	{
		result.fAddOp(va_arg(valist, glm::quat));
	}
	va_end(valist);
	return _fCombineSymmetry(result, result);
}