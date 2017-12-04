#pragma once
#include "Common.h"


/**
* A container to hold any perlin noise information
*/
class PerlinNoise
{
private:
	///
	/// Noise vars
	///
	uint32 m_seed = 0;

public:
	/**
	* Calculate a normalized perlin noise value for this coordinate
	* @param x,y,z				Coordinates to perform lookup for
	* @returns Value between 0-1
	*/
	float Get01(const float& x, const float& y, const float& z);

	/**
	* Get noise by merging multiple samples together to with given settings
	* @param x,y,z				Coordinates to perform lookup for
	* @param octaves			How many samples to blend between
	* @param persistence		How much amplitude decays by with each sample
	* @returns Value between 0-1
	*/
	float GetOctave(const float& x, const float& y, const float& z, const uint32& octaves, const float& persistence);


	///
	/// Getters & Setters
	///
public:
	inline void SetSeed(const uint32& value) { m_seed = value; }
	inline uint32 GetSeed() const { return m_seed; }
};

