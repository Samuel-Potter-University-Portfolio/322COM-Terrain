#pragma once
#include "Mesh.h"


/**
* Stores several meshes which can all be swapped out at different LoDs
*/
class LeveledMesh
{
private:
	///
	/// LoD settings
	///
	std::vector<Mesh*> m_meshes;
	float m_lodRange = 100.0f;

public:
	~LeveledMesh() 
	{
		for (Mesh* mesh : m_meshes)
			delete mesh;
	}
	
	/**
	* Add a new level of detail 
	* @returns The new mesh for this LoD
	*/
	inline Mesh* AddLoD() 
	{
		Mesh* mesh = new Mesh;
		m_meshes.emplace_back(mesh);
		return mesh;
	}

	/**
	* Add several levels of details
	* @param count		How many levels to add
	*/
	inline void AddLoDs(const uint32& count)
	{
		for (uint32 i = 0; i < count; ++i)
			AddLoD();
	}


	/**
	* Get the mesh to use for this particular level
	* @param level			The level to get the mesh for
	* @returns The mesh that will be used for this level
	*/
	inline Mesh* GetLoD(const uint32& level) const
	{
		return m_meshes[level > m_meshes.size() - 1 ? m_meshes.size() - 1 : level];
	}

	/**
	* Get the mesh that should be displayed at this distance
	* @param distanceSqrd			The squared distance to look for
	*/
	inline Mesh* GetMesh(const float& distanceSqrd) const 
	{
		uint32 level = distanceSqrd / (m_lodRange*m_lodRange);
		return GetLoD(level);
	}

	///
	/// Getters & Setters
	///
public:
	/** How may levels of detail currently exist */
	inline uint32 GetLevelCount() const { return m_meshes.size(); }

	inline void SetLoDRange(const float& range) { m_lodRange = range; }
	inline float GetLoDRange() const { return m_lodRange; }


};

