//
// Created by ianpo on 20/07/2023.
//
#include "Voxymore/Renderer/Renderer.hpp"
#include "Voxymore/Renderer/Shader.hpp"
#include "Voxymore/OpenGL/OpenGLShader.hpp"
#include "Voxymore/Core/Logger.hpp"
#include <utility>

namespace Voxymore::Core{

	Ref<Shader> Shader::Create(const std::string& name, const std::vector<ShaderSourceField>& sources)
	{
		VXM_PROFILE_FUNCTION();
		std::unordered_map<ShaderType, ShaderSourceField> map;
		for (auto& src : sources) {
			if(src) {
				map.insert_or_assign(src.GetAsset()->GetShaderType(), src);
			}
		}
		return Create(name, map);
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::unordered_map<ShaderType, ShaderSourceField>& sources)
	{
		VXM_PROFILE_FUNCTION();
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:
				VXM_CORE_ASSERT(false, "RendererAPI::API::None is not supported to create a shader.")
				return nullptr;
				break;
			case RendererAPI::API::OpenGL: {
				return CreateRef<OpenGLShader>(name, sources);
			} break;
		}
		VXM_CORE_ASSERT(false, "Render API '{0}' not supported.", RendererAPIToString(Renderer::GetAPI()))
		return nullptr;
	}

	RuntimeShaderSource::RuntimeShaderSource(ShaderType type) : Type(type)
	{
	}

	RuntimeShaderSource::RuntimeShaderSource(std::string source) : Source(std::move(source))
	{
	}

	RuntimeShaderSource::RuntimeShaderSource(ShaderType type, std::string source) : Type(type), Source(std::move(source))
	{
	}

	EditorShaderSource::EditorShaderSource(ShaderType type) : Type(type)
	{
	}

	const char* EditorShaderSource::GetRawString()
	{
		UpdateSourceIfNeeded();
		return m_Source.c_str();
	}

	std::string EditorShaderSource::GetString()
	{
		UpdateSourceIfNeeded();
		return m_Source;
	}

	void EditorShaderSource::UpdateSourceIfNeeded()
	{
		VXM_PROFILE_FUNCTION();
		auto metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(Handle);
		auto lastWrite = std::filesystem::last_write_time(metadata.FilePath);
		bool dirty = m_Ftt != lastWrite;
		if(dirty) {
			m_Ftt = lastWrite;
			m_Source = FileSystem::ReadFileAsString(metadata.FilePath);
		}
	}
	bool EditorShaderSource::NeedUpdate()
	{
		auto metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(Handle);
		auto lastWrite = std::filesystem::last_write_time(metadata.FilePath);
		bool dirty = m_Ftt != lastWrite;
		return dirty;
	}

	void EditorShaderSource::Reload()
	{
		auto metadata = Project::GetActive()->GetEditorAssetManager()->GetMetadata(Handle);
		m_Ftt = std::filesystem::last_write_time(metadata.FilePath);
		m_Source = FileSystem::ReadFileAsString(metadata.FilePath);
	}
}
