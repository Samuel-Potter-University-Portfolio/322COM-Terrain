#include "Material.h"

#include "Window.h"
#include "Scene.h"
#include "Mesh.h"


Material::Material(Shader* shader) :
	m_shader(shader)
{
	m_uniformObjectToWorld = m_shader->GetUniform("ObjectToWorld");
	m_uniformWorldToView = m_shader->GetUniform("WorldToView");
	m_uniformViewToClip = m_shader->GetUniform("ViewToClip");
}

Material::~Material()
{
}


void Material::Bind(Window& window, Scene& scene)
{
	m_shader->Bind();
	m_shader->SetUniformMat4(m_uniformWorldToView, scene.GetCamera().GetViewMatrix());
	m_shader->SetUniformMat4(m_uniformViewToClip, scene.GetCamera().GetPerspectiveMatrix(&window));
}

void Material::PrepareMesh(Mesh& mesh)
{
	glBindVertexArray(mesh.GetID());
	m_boundMeshTris = mesh.GetTriangleCount();
}

void Material::RenderInstance(class Transform& transform) 
{
	m_shader->SetUniformMat4(m_uniformObjectToWorld, transform.GetTransformMatrix());

	glDrawElements(GL_TRIANGLES, m_boundMeshTris, GL_UNSIGNED_INT, nullptr);
}