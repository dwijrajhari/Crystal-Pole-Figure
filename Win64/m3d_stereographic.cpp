#include <M3D/m3d_stereographic.h>

glm::vec2 fProjectPoint(glm::vec3 p_)
{
	return glm::vec2(p_ / (1+glm::abs(p_.z)));
}
