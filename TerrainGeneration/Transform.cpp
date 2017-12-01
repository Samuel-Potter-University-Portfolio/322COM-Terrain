#include "Transform.h"

#include "glm.hpp"
#include "gtc\matrix_transform.hpp"


Transform::Transform() :
	m_location(0, 0, 0),
	m_rotation(0, 0, 0),
	m_scale(1, 1, 1)
{
}

mat4 Transform::GetTransformMatrix() 
{
	if (!bRequiresRebuild)
		return m_matrix;

	m_matrix = mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_location);
	m_matrix = glm::rotate(m_matrix, m_rotation.x, vec3(1, 0, 0));
	m_matrix = glm::rotate(m_matrix, m_rotation.y, vec3(0, 1, 0));
	m_matrix = glm::rotate(m_matrix, m_rotation.z, vec3(0, 0, 1));
	m_matrix = glm::scale(m_matrix, m_scale);
	bRequiresRebuild = false;
	return m_matrix;
}
