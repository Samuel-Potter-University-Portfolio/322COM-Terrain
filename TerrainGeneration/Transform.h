#pragma once
#include "Common.h"

/**
* Holds all transform information e.g. translation, rotation, scale
*/
class Transform
{
private:
	vec3 m_location;
	vec3 m_rotation;
	vec3 m_scale;

	///
	/// Only rebuild matrix, if there has been a change
	///
	mat4 m_matrix;
	bool bRequiresRebuild = true;

public:
	Transform();

	/**
	* Get the transformation matrix to use for this transform data
	*/
	mat4 GetTransformMatrix();

	/**
	* Getters & Setters
	*/
public:
	inline void SetLocation(const vec3& value) { m_location = value; bRequiresRebuild = true; }
	inline void Translate(const vec3& value) { m_location += value; bRequiresRebuild = true; }
	inline vec3 GetLocation() const { return m_location; }

	inline void SetEularRotation(const vec3& value) { m_rotation = value; bRequiresRebuild = true; }
	inline vec3 GetEularRotation() const { return m_rotation; }

	inline void SetScale(const vec3& value) { m_scale = value; bRequiresRebuild = true; }
	inline vec3 GetScale() const { return m_scale; }
};

