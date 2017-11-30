#pragma once
#include "Common.h"


/**
* Loads several GLSL shaders and links them
*/
class Shader
{
private:
	///
	/// Shader ids
	///
	uint32 m_programId;

	uint32 m_vertexId;
	uint32 m_fragmentId;

	///
	/// Shader settings
	///
	bool bEnableCullFace = true;
	bool bCullBackFace = true;
	bool bEnableDepthTest = true;

public:
	Shader();
	virtual ~Shader();

	/**
	* Bind all of the settings required to use this shader
	*/
	virtual void Bind();

	/**
	* Adds a vertex shader unit to this shader from source code
	* @param code		The glsl source code for the shader
	* @returns If succesful or not
	*/
	bool LoadVertexShaderFromMemory(const string& code);
	/**
	* Adds a fragment shader unit to this shader from source code
	* @param code		The glsl source code for the shader
	* @returns If succesful or not
	*/
	bool LoadFragmentShaderFromMemory(const string& code);

	/**
	* Attempt to link all subshaders and store them in a program
	* @returns If succesful or not
	*/
	bool LinkShader();


	/**
	* Getters & Setters
	*/
public:
	inline uint32 GetID() const { return m_programId; }

	inline void SetCullFace(const bool& value) { bEnableCullFace = value; }
	inline bool GetCullFace() const { return bEnableCullFace; }

	inline void SetCullBackFace(const bool& value) { bCullBackFace = value; }
	inline bool GetCullBackFace() const { return bCullBackFace; }

	inline void SetDepthTest(const bool& value) { bEnableDepthTest = value; }
	inline bool GetDepthTest() const { return bEnableDepthTest; }
};

