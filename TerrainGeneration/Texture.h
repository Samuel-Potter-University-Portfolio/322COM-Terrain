#pragma once
#include "Common.h"

#include <vector>


/**
* Lets user load and access texture data
*/
class Texture
{
private:
	///
	/// IDs
	///
	uint32 m_id = 0;

	///
	/// Texture Settings
	///
	uint32 m_width;
	uint32 m_height;
	bool bIsLoaded = false;
	bool bIsRepeated = true;
	bool bIsSmooth = true;


public:
	Texture();
	~Texture();


	///
	/// Loading texture
	///
public:
	/**
	* Attempt to load a texture from an image
	* @param file		The path to the file to load from
	* @returns If succesful or not
	*/
	bool LoadFromFile(const string& file);
	


private:


	///
	/// Getters & Setters
	///
public:
	inline uint32 GetID() const { return m_id; }

	void SetRepeated(const bool& value);
	inline bool IsRepeated() const { return bIsRepeated; }

	void SetSmooth(const bool& value);
	inline bool IsSmooth() const { return bIsSmooth; }
};

