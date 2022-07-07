#include <M3D/m3d_utils.h>
#include <M3D/m3d_crystals.h>
#include <M3D/m3d_symmetry.h>

std::string LS_enum2str(LatticeSystem& ls)
{
    if (ls == LatticeSystem::CUBIC) return "Cubic";
    else if (ls == LatticeSystem::TETRAGONAL) return "Tetragonal";
    else if (ls == LatticeSystem::HEXAGONAL) return "Hexagonal";
    else if (ls == LatticeSystem::RHOMBOHEDRAL) return "Rhombohedral";
    else if (ls == LatticeSystem::ORTHORHOMBIC) return "Orthorhombic";
    else if (ls == LatticeSystem::MONOCLINIC) return "Monoclinic";
    else if (ls == LatticeSystem::TRICLINIC) return "Triclinic";
    else return "None";
}

void print_symmetry(Symmetry& s)
{
    std::printf("Symmetry(%d)\n", (int)s.operations.size());
    for (const std::pair<glm::quat, bool>& q : s.operations)
    {
        std::printf("[%.3f, %.3f, %.3f, %.3f]\n", q.first.w, q.first.x, q.first.y, q.first.z);
    }
    std::printf("\n");
}