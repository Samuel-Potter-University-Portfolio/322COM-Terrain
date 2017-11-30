#include "Shader.h"
#include "Logger.h"

#include <GL\glew.h>


Shader::Shader()
{
}

Shader::~Shader()
{
	if (m_programId != 0)
		glDeleteProgram(m_programId);
}


void Shader::Bind() 
{
	glUseProgram(m_programId);


	if (bEnableCullFace)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(bCullBackFace ? GL_BACK : GL_FRONT);
	}
	else
		glDisable(GL_CULL_FACE);


	if (bEnableDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}


bool Shader::LoadVertexShaderFromMemory(const string& code)
{
	m_vertexId = glCreateShader(GL_VERTEX_SHADER);

	const char* str = code.c_str();
	const int32 length = code.size();

	glShaderSource(m_vertexId, 1, &str, &length);
	glCompileShader(m_vertexId);


	// Make sure compile correctly
	int32 result;
	glGetShaderiv(m_vertexId, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int32 logLength;
		glGetShaderiv(m_vertexId, GL_INFO_LOG_LENGTH, &logLength);

		char* logMessage = new char[logLength];
		glGetShaderInfoLog(m_vertexId, logLength, nullptr, logMessage);

		LOG_ERROR("Shader(vert) compile error: '%s'", logMessage);
		delete[] logMessage;
		m_vertexId = 0;
		return false;
	}

	return true;
}

bool Shader::LoadFragmentShaderFromMemory(const string& code)
{
	m_fragmentId = glCreateShader(GL_FRAGMENT_SHADER);

	const char* str = code.c_str();
	const int32 length = code.size();

	glShaderSource(m_fragmentId, 1, &str, &length);
	glCompileShader(m_fragmentId);


	// Make sure compile correctly
	int32 result;
	glGetShaderiv(m_fragmentId, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int32 logLength;
		glGetShaderiv(m_fragmentId, GL_INFO_LOG_LENGTH, &logLength);

		char* logMessage = new char[logLength];
		glGetShaderInfoLog(m_fragmentId, logLength, nullptr, logMessage);

		LOG_ERROR("Shader(frag) compile error: '%s'", logMessage);
		delete[] logMessage;
		m_fragmentId = 0;
		return false;
	}

	return true;
}

bool Shader::LinkShader() 
{
	m_programId = glCreateProgram();

	glAttachShader(m_programId, m_vertexId);
	glAttachShader(m_programId, m_fragmentId);
	glLinkProgram(m_programId);

	// Make sure link correctly
	int32 result;
	glGetShaderiv(m_programId, GL_LINK_STATUS, &result);

	if (result == GL_FALSE)
	{
		int32 logLength;
		glGetShaderiv(m_programId, GL_INFO_LOG_LENGTH, &logLength);

		char* logMessage = new char[logLength];
		glGetShaderInfoLog(m_programId, logLength, nullptr, logMessage);

		LOG_ERROR("Shader link error: '%s'", logMessage);
		delete[] logMessage;
		m_programId = 0;
		return false;
	}
	
	return true;
}