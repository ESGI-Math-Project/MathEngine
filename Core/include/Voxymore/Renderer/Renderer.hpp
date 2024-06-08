//
// Created by ianpo on 20/07/2023.
//

#pragma once

#include "Mesh.hpp"
#include "Voxymore/Core/Core.hpp"
#include "Voxymore/Renderer/Camera.hpp"
#include "Voxymore/Renderer/EditorCamera.hpp"
#include "Voxymore/Renderer/Material.hpp"
#include "Voxymore/Renderer/PerspectiveCamera.hpp"
#include "Voxymore/Renderer/RenderCommand.hpp"
#include "Voxymore/Renderer/Shader.hpp"
#include "Voxymore/Renderer/UniformBuffer.hpp"
#include "Voxymore/Renderer/VertexArray.hpp"
#include "Voxymore/Renderer/Model.hpp"
#include "Voxymore/Renderer/Light.hpp"
#include <map>

namespace Voxymore::Core {

	struct RendererData
	{
		static constexpr const inline int NUM_CONTROL_POINTS_MAX = 48;
		static constexpr const inline int MAX_LIGHT_COUNT = 20;

		// TODO: Add Normal Matrix
		// TODO: Add MVP Matrix
		// Both Previous todo are to reduce the load on the GPU side.
		//  But it might need a bit of checking and more data to know if it's worth.

		struct CameraData
		{
			glm::mat4 ViewProjectionMatrix;
			glm::vec4 CameraPosition;
			glm::vec4 CameraDirection;
		};

		struct ModelData
		{
			inline ModelData() = default;
			inline ~ModelData() = default;
			ModelData(glm::mat4 transformMatrix, glm::mat4 normalMatrix, int entityId);
			glm::mat4 TransformMatrix;
			glm::mat4 NormalMatrix;
			int EntityId;
		};

		struct LightData
		{
			Light lights[MAX_LIGHT_COUNT];
			int lightCount;
		};

		struct CurveParameters
		{
			// Using 96 as it's divisible by 4 so (31*3) round up to be a multiple of 4.
			std::array<glm::vec4, NUM_CONTROL_POINTS_MAX> CurveControlPoints;
			std::array<float, NUM_CONTROL_POINTS_MAX> CurveWeights;
			std::array<glm::vec4, NUM_CONTROL_POINTS_MAX> ProfileControlPoints;
			std::array<float, NUM_CONTROL_POINTS_MAX> ProfileWeights;
			int NumberOfSegment = 0;
			int NumberOfControlPoint = 0;
			int Degree = 0;
			int NumberOfKnot = 0;
		};

		CameraData CameraBuffer;
		ModelData ModelBuffer;
		LightData LightBuffer;
		CurveParameters CurveBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
		Ref<UniformBuffer> ModelUniformBuffer;
		Ref<UniformBuffer> LightUniformBuffer;
		Ref<UniformBuffer> MaterialUniformBuffer;
		Ref<UniformBuffer> CurveParametersBuffer;
		std::multimap<Real, std::tuple<const Ref<Mesh>, glm::mat4, int>> AlphaMeshes;
		std::vector<std::tuple<const Ref<Mesh>, glm::mat4, int>> OpaqueMeshes;
	};

	class Renderer {
	private:
		static void Submit(const Ref<Model>& model, const Node& node, const glm::mat4& transform = glm::mat4(1.0f), int entityId = -1);
		static void DrawMesh(Ref<Mesh> mesh, const glm::mat4& modelMatrix, int entityId = -1);
	public:
		static void Init();
		static void Shutdown();
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(const Camera& camera, const glm::mat4& transform, std::vector<Light> lights = {});
		static void BeginScene(const EditorCamera& camera, std::vector<Light> lights = {});
		static void EndScene();

		[[deprecated("The submission of raw vertex array is not supported anymore. use the class Mesh")]]
		static void Submit(Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f), int entityId = -1);
		[[deprecated("The submission of raw vertex array is not supported anymore. use the class Mesh")]]
		static void Submit(Ref<Material>& material, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f), int entityId = -1);

		static void Submit(const MeshGroup& mesh, const glm::mat4& transform = glm::mat4(1.0f), int entityId = -1);
		static void Submit(const Ref<Model>& model, const glm::mat4& transform = glm::mat4(1.0f), int entityId = -1);
		static void Submit(Ref<Mesh> model, const glm::mat4& transform = glm::mat4(1.0f), int entityId = -1);
//			static void Submit(const Mesh& model, const glm::mat4& transform = glm::mat4(1.0f), int entityId = -1);

		static void Submit(Ref<Material> material, const std::vector<glm::vec3>& bezierControlPoints, int lineDefintion = 1000, int entityId = -1);
		static void Submit(Ref<Material> material, const glm::vec3& controlPoint0, const glm::vec3& controlPoint1, const glm::vec3& controlPoint2, const glm::vec3& controlPoint3, int lineDefintion = 1000, int entityId = -1);
		static void Submit(Ref<Material> material, int degree, const std::vector<glm::vec3>& points, const std::vector<float>& nodes, const std::vector<float>& weights, int lineDefinition = 1000, int entityId = -1);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		//            static Scope<RendererData> s_Data;
	};

} // Voxymore
// Core
