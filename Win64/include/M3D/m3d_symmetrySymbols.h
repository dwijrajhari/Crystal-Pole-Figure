#include <M3D/m3d_symmetry.h>

static double sqrthalf = glm::sqrt(0.5);

Symmetry s1 = fCreateSymmetry(1, glm::quat(1.0, 0.0, 0.0, 0.0));  // s_1

Symmetry s211 = fCreateSymmetry(2, // m11
    glm::quat(1.0, 0.0, 0.0, 0.0),
    glm::quat(0.0, 1.0, 0.0, 0.0));
Symmetry s121 = fCreateSymmetry(2, // 1m1
    glm::quat(1.0, 0.0, 0.0, 0.0),
    glm::quat(0.0, 0.0, 1.0, 0.0));
Symmetry s112 = fCreateSymmetry(2, // 11m
    glm::quat(1.0, 0.0, 0.0, 0.0),
    glm::quat(0.0, 0.0, 0.0, 1.0));

Symmetry s2 = s112;

Symmetry s2bym = fCombineSymmetry(2, s112, s112); //s211 + sm11

Symmetry s222 = fCombineSymmetry(3, s211, s121, s112);
Symmetry smm2 = fCombineSymmetry(2, s211, s112);  //s211 + s11m
Symmetry smmm = fCombineSymmetry(3, s211, s121, s112); //sm11 + s1m1 + s11m

Symmetry _s411 = fCreateSymmetry(4, 
    glm::quat(1.0, 0.0, 0.0, 0.0),
    glm::quat(sqrthalf, sqrthalf, 0.0, 0.0),
    glm::quat(0.0, 0.0, 1.0, 0.0),
    glm::quat(-sqrthalf, sqrthalf, 0.0, 0.0));

Symmetry _s141 = fCreateSymmetry(4, 
    glm::quat(1.0, 0.0, 0.0, 0.0),
    glm::quat(sqrthalf, 0.0, sqrthalf, 0.0),
    glm::quat(0.0, 0.0, 1.0, 0.0),
    glm::quat(-sqrthalf, 0.0, sqrthalf, 0.0));

Symmetry _s114 = fCreateSymmetry(4, 
    glm::quat(1.0, 0.0, 0.0, 0.0),
    glm::quat(sqrthalf, 0.0, 0.0, sqrthalf),
    glm::quat(0.0, 0.0, 1.0, 0.0),
    glm::quat(-sqrthalf, 0.0, 0.0, sqrthalf));

Symmetry s4 = _s114;


Symmetry s432 = fCreateSymmetry(4,
    glm::quat(1.0, 0.0, 0.0, 0.0),
    glm::quat(0.707106, 0.0, 0.0, 0.707106),
    glm::quat(0.5, 0.5, 0.5, 0.5),
    glm::quat(0.0, 0.7071068, 0.7071068, 0.0));