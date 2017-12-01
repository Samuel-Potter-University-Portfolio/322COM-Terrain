#pragma once
#include "Common.h"

#include "Mesh.h"


#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 32


class Terrain;


namespace Voxel 
{
	/** Supported voxel types */
	enum class Type : uint8 
	{
		Air = 0,
		Grass,
		Dirt,
		Sand,
		Stone,

		Water,
	};
}


/**
* 
*/
class Chunk
{
private:
	///
	/// Terrain vars
	///
	Terrain* m_terrain;
	Voxel::Type m_voxels[CHUNK_HEIGHT * CHUNK_SIZE * CHUNK_SIZE]{ Voxel::Type::Air };
	ivec2 m_chunkCoords;

	///
	/// Rendering vars
	///
	Mesh* m_mesh = nullptr;

public:
	Chunk(Terrain* terrain);
	~Chunk();


	/**
	* Callback for when this chunk comes into usage
	* (Called when chunk is used from object pool)
	* @param coord			The new chunk coord that this will use
	*/
	void Alloc(const ivec2& coord);
	/**
	* Callback for when this chunk goes out of usage (i.e. goes back into the pool)
	* (Called when chunk goes back into object pool)
	*/
	void Dealloc();


private:
	/// Convect XYZ to a valid array index
	inline static int32 GetIndex(const int32& x, const int32& y, const int32& z) { return x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE; }

	/**
	* Sets a specific voxel from local coordinates
	* @param x,y,z			The local coordinates of the voxel to set
	* @param type			The type this voxel should be
	*/
	inline void Set(const int32& x, const int32& y, const int32& z, const Voxel::Type& type) { m_voxels[GetIndex(x, y, z)] = type; }

	/**
	* Retreive a specific voxel from local coordinates
	* @param x,y,z			The local coordinates of the voxel to set
	*/
	inline Voxel::Type Get(const int32& x, const int32& y, const int32& z) const { return m_voxels[GetIndex(x, y, z)]; }


	///
	/// Getters & Setters
	///
public:
	inline ivec2 GetCoords() const { return m_chunkCoords; }
	inline Mesh* GetMesh() const { return m_mesh; }
};

