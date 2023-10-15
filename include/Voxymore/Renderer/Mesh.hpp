//
// Created by Sayama on 27/09/2023.
//

#pragma once

#include "Voxymore/Core/Macros.hpp"
#include "Voxymore/Core/Logger.hpp"
#include "Voxymore/Core/SmartPointers.hpp"
#include "Voxymore/Core/FileSystem.hpp"
#include "Voxymore/Renderer/Buffer.hpp"
#include "Voxymore/Renderer/VertexArray.hpp"
#include "Voxymore/Renderer/UniformBuffer.hpp"
#include "Voxymore/Renderer/Shader.hpp"
#include <optional>
#include <vector>

namespace Voxymore::Core
{
	class Model;

	//TODO: create an API to be able to create Mesh from the client side.
	class Mesh
	{
	private:
		friend class Model;
	public:
		struct SubMesh
		{
		public:
			SubMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texcoords, const std::vector<glm::vec4>& colors, const std::vector<uint32_t >& indexes);
			~SubMesh();
			inline const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }
			void Bind() const;
			void Unbind() const;
		private:
			Ref<VertexArray> m_VertexArray;
			Ref<VertexBuffer> m_VertexBuffer;
			Ref<IndexBuffer> m_IndexBuffer;
			BufferLayout m_BufferLayout;

			const size_t VerticeCount;
			const size_t VerticeFloatCount;
			const size_t BufferSize;
			float* VertexData;
		};
	private:
		std::vector<SubMesh> m_SubMeshes;
		std::optional<Ref<Shader>> m_Shader;
	public:
		Mesh() = default;
		inline Mesh(const Ref<Shader>& shader) : m_Shader(shader) {}
		void Bind() const;
		void Unbind() const;
		//		inline const std::vector<Ref<VertexArray>>& GetVertexArrays() const { return m_VertexArrays; }
		inline const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }
		void AddSubMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texcoords, const std::vector<glm::vec4> &colors, const std::vector<uint32_t >& indexes);
	};

} // namespace Voxymore::Core

