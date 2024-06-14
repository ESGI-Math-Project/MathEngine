//
// Created by ianpo on 20/07/2023.
//

#include "Voxymore/Renderer/Renderer.hpp"
#include "Voxymore/Core/Logger.hpp"
#include "Voxymore/OpenGL/OpenGLShader.hpp"

namespace Voxymore::Core {
	static RendererData s_Data;
	static ShaderField s_BindedShader = NullAssetHandle;
	static MaterialField s_BindedMaterial = NullAssetHandle;

	RendererData::ModelData::ModelData(glm::mat4 transformMatrix, glm::mat4 normalMatrix, int entityId) : TransformMatrix(transformMatrix), NormalMatrix(normalMatrix), EntityId(entityId) {}

	void Renderer::Init() {
		VXM_PROFILE_FUNCTION();
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);
		s_Data.ModelUniformBuffer = UniformBuffer::Create(sizeof(RendererData::ModelData), 1);
		s_Data.LightUniformBuffer = UniformBuffer::Create(sizeof(RendererData::LightData), 2);
		s_Data.MaterialUniformBuffer = UniformBuffer::Create(sizeof(MaterialParameters), 3);
		s_Data.CurveParametersBuffer = UniformBuffer::Create(sizeof(RendererData::CurveParameters), 4);

		RenderCommand::Init();
	}

	void Renderer::Shutdown() {
		VXM_PROFILE_FUNCTION();


		RenderCommand::Shutdown();
	}

	void Renderer::BeginScene(const EditorCamera &camera, std::vector<Light> lights)
	{
		VXM_PROFILE_FUNCTION();

		s_BindedShader = NullAssetHandle;
		s_BindedMaterial = NullAssetHandle;
		s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetViewProjection();
		s_Data.CameraBuffer.CameraPosition = glm::vec4(camera.GetPosition(), 1);
		s_Data.CameraBuffer.CameraDirection = glm::vec4(camera.GetForwardDirection(), 0);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
		s_Data.LightBuffer.lightCount = std::min((int)lights.size(), RendererData::MAX_LIGHT_COUNT);
		for (size_t i = 0; i < s_Data.LightBuffer.lightCount; ++i)
		{
			s_Data.LightBuffer.lights[i] = lights[i];
		}
		s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer, sizeof(RendererData::LightData));

		s_Data.AlphaMeshes.clear();
		s_Data.OpaqueMeshes.clear();
	}

	void Renderer::BeginScene(const Camera &camera, const glm::mat4 &transform, std::vector<Light> lights)
	{
		VXM_PROFILE_FUNCTION();
		s_BindedShader = NullAssetHandle;
		s_BindedMaterial = NullAssetHandle;
		s_Data.CameraBuffer.ViewProjectionMatrix = camera.GetProjectionMatrix() * glm::inverse(transform);
		auto p = transform * glm::vec4{0,0,0,1};
		s_Data.CameraBuffer.CameraPosition = glm::vec4(glm::vec3(p) / p.w, 1);
		s_Data.CameraBuffer.CameraDirection = transform * glm::vec4{0,0,1,0};
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
		s_Data.LightBuffer.lightCount = std::min((int)lights.size(), RendererData::MAX_LIGHT_COUNT);
		for (size_t i = 0; i < s_Data.LightBuffer.lightCount; ++i)
		{
			s_Data.LightBuffer.lights[i] = lights[i];
		}
		s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer, sizeof(RendererData::LightData));

		s_Data.AlphaMeshes.clear();
		s_Data.OpaqueMeshes.clear();
	}

	void Renderer::DrawMesh(Ref<Mesh> m, const glm::mat4& modelMatrix, int entityId)
	{
		s_Data.ModelBuffer.TransformMatrix = modelMatrix;
		s_Data.ModelBuffer.NormalMatrix = glm::transpose(glm::inverse(modelMatrix));
		s_Data.ModelBuffer.EntityId = entityId;
		s_Data.ModelUniformBuffer->SetData(&s_Data.ModelBuffer, sizeof(RendererData::ModelData));

		MaterialField mat = m->GetMaterial();
		VXM_CORE_ASSERT(mat, "The material ID({}) is not valid.", mat.GetHandle().string());
		Ref<Material> matPtr = mat.GetAsset();
		if(mat) {
			s_Data.MaterialUniformBuffer->SetData(&matPtr->GetMaterialsParameters(), sizeof(MaterialParameters));
			matPtr->Bind(false);
			s_BindedMaterial = mat;
		}

		ShaderField shader = matPtr->GetShaderHandle();
		VXM_CORE_ASSERT(shader, "The shader ID({}) from the material '{}' is not valid.", matPtr->GetMaterialName(), shader.GetHandle().string());
		if (shader) {
			shader.GetAsset()->Bind();
			s_BindedShader = shader;
		}
		m->Bind();
		RenderCommand::DrawIndexed(m->GetVertexArray());
	}

	void Renderer::EndScene() {
		VXM_PROFILE_FUNCTION();

		for(const auto& mesh : s_Data.OpaqueMeshes)
		{
			DrawMesh(std::get<0>(mesh), std::get<1>(mesh), std::get<2>(mesh));
		}

		for(auto it = s_Data.AlphaMeshes.rbegin(); it != s_Data.AlphaMeshes.rend(); ++it)
		{
			auto& mesh = it->second;
			DrawMesh(std::get<0>(mesh), std::get<1>(mesh), std::get<2>(mesh));
		}

		RenderCommand::ClearBinding();
		s_BindedShader = NullAssetHandle;
		s_BindedMaterial = NullAssetHandle;
	}

	void Renderer::Submit(Ref<Shader>& shader, const Ref<VertexArray> &vertexArray, const glm::mat4& transform, int entityId) {
		VXM_PROFILE_FUNCTION();
		//        VXM_CORE_ASSERT(s_Data.CameraBuffer.ViewProjectionMatrix != glm::zero<glm::mat4>(), "A valid View Projection Matrix is required to submit data to the renderer.");


		s_Data.ModelBuffer.TransformMatrix = transform;
		s_Data.ModelBuffer.NormalMatrix = glm::transpose(glm::inverse(transform));
		s_Data.ModelBuffer.EntityId = entityId;
		s_Data.ModelUniformBuffer->SetData(&s_Data.ModelBuffer, sizeof(RendererData::ModelData));

		ShaderField shaderField = shader->Handle;
		VXM_CORE_ASSERT(shader, "The shader ID({}) is not valid.", shaderField.GetHandle().string());
		if (shaderField) {
			shader->Bind();
			s_BindedShader = shaderField;
		}
		//TODO: Set the view projection matrix once per frame not once per model drawn.
		//        std::dynamic_pointer_cast<OpenGLShader>(shader)->SetUniformMat4("u_ViewProjectionMatrix", s_Data.ViewProjectionMatrix);
		//        std::dynamic_pointer_cast<OpenGLShader>(shader)->SetUniformMat4("u_Transform", transform);
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::Submit(Ref<Material> &material, const Ref<VertexArray> &vertexArray, const glm::mat4 &transform, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		//		VXM_CORE_ASSERT(s_Data.ViewProjectionMatrix != glm::zero<glm::mat4>(), "A valid View Projection Matrix is required to submit data to the renderer.");


		s_Data.ModelBuffer.TransformMatrix = transform;
		s_Data.ModelBuffer.NormalMatrix = glm::transpose(glm::inverse(transform));
		s_Data.ModelBuffer.EntityId = entityId;
		s_Data.ModelUniformBuffer->SetData(&s_Data.ModelBuffer, sizeof(RendererData::ModelData));

		s_Data.MaterialUniformBuffer->SetData(&material->GetMaterialsParameters(), sizeof(MaterialParameters));

		MaterialField mat = material->Handle;
		VXM_CORE_ASSERT(mat, "The material ID({}) is not valid.", mat.GetHandle().string());
		Ref<Material> matPtr = mat.GetAsset();
		if(mat) {
			s_Data.MaterialUniformBuffer->SetData(&matPtr->GetMaterialsParameters(), sizeof(MaterialParameters));
			matPtr->Bind(false);
			s_BindedMaterial = mat;
		}

		ShaderField shader = matPtr->GetShaderHandle();
		VXM_CORE_ASSERT(shader, "The shader ID({}) from the material '{}' is not valid.", matPtr->GetMaterialName(), shader.GetHandle().string());
		if (shader) {
			shader.GetAsset()->Bind();
			s_BindedShader = shader;
		}

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::Submit(const MeshGroup& meshGroup, const glm::mat4& transform, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		// s_Data.ModelBuffer.TransformMatrix = transform;
		// s_Data.ModelBuffer.NormalMatrix = glm::transpose(glm::inverse(transform));
		// s_Data.ModelBuffer.EntityId = entityId;
		// s_Data.ModelUniformBuffer->SetData(&s_Data.ModelBuffer, sizeof(RendererData::ModelData));

		for (const auto& mesh : meshGroup)
		{
			if(mesh) {
				Submit(mesh.GetAsset(), transform, entityId);
			}
		}
	}

	void Renderer::Submit(Ref<Mesh> mesh, const glm::mat4& transform, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		Vec4 center = transform*Vec4(mesh->GetBoundingBox().GetCenter(),1);
		center /= center.w;
		Real distance = Math::SqrMagnitude(Vec3(center) - Vec3(s_Data.CameraBuffer.CameraPosition));

		if(mesh->GetMaterial().GetAsset()->GetMaterialsParameters().AlphaMode == AlphaMode::Blend)
		{
			s_Data.AlphaMeshes.insert(std::make_pair(distance, std::tuple(mesh, transform, entityId)));
		}
		else
		{
			s_Data.OpaqueMeshes.emplace_back(std::tuple(mesh, transform, entityId));
		}
	}
/*
	void Renderer::Submit(const Mesh& mesh, const glm::mat4& transform, int entityId)
	{
		VXM_PROFILE_FUNCTION();

		s_Data.ModelBuffer.TransformMatrix = transform;
		s_Data.ModelBuffer.NormalMatrix = glm::transpose(glm::inverse(transform));
		s_Data.ModelBuffer.EntityId = entityId;
		s_Data.ModelUniformBuffer->SetData(&s_Data.ModelBuffer, sizeof(RendererData::ModelData));
		s_Data.MaterialUniformBuffer->SetData(&mesh.GetMaterial()->GetMaterialsParameters(), sizeof(MaterialParameters));
		mesh.Bind();
		auto& shaderName = mesh.GetMaterial()->GetShaderName();
		if(shaderName != s_BindedShader)
		{
			mesh.GetMaterial()->Bind();
			s_BindedShader = shaderName;
		}
		RenderCommand::DrawIndexed(mesh.GetVertexArray());
	}
*/
	void Renderer::Submit(const Ref<Model>& model, const glm::mat4& transform, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		//model->Bind();
		for (int nodeIndex : model->GetDefaultScene())
		{
			Submit(model, model->GetNode(nodeIndex), transform, entityId);
		}
	}

	void Renderer::Submit(const Ref<Model>& model, const Node& node, const glm::mat4& transform, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		glm::mat4 currentTransform = transform * node.transform;
		if(node.HasMesh())
		{
			Submit(model->GetMeshGroup(node.GetMeshIndex()), currentTransform, entityId);
		}

		if(node.HasChildren())
		{
			for(const int i : node.children)
			{
				Submit(model, model->GetNode(i), currentTransform, entityId);
			}
		}
	}

	void Renderer::Submit(Ref<Material> material, const std::vector<glm::vec3>& bezierControlPoints, int lineDefinition, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		VXM_CORE_ASSERT(bezierControlPoints.size() <= s_Data.CurveBuffer.CurveControlPoints.size(), "The shader might won't support more than a {0} control point...", s_Data.CurveBuffer.CurveControlPoints.size());


		uint32_t count = std::min(s_Data.CurveBuffer.CurveControlPoints.size(), bezierControlPoints.size());

		std::vector<Vertex> vertices((count/3) + (count%3 ? 1 : 0));

		s_Data.CurveBuffer.NumberOfSegment = lineDefinition;
		s_Data.CurveBuffer.MainCurveNumberOfControlPoint = static_cast<int>(bezierControlPoints.size());
		for (int i = 0; i < count; ++i) {
			s_Data.CurveBuffer.CurveControlPoints[i] = glm::vec4(bezierControlPoints[i],1);
			vertices[i/3].Position[i%3] = float(i);
		}
		std::vector<uint32_t> indices;
		indices.reserve((bezierControlPoints.size()-1)*2);
		for (uint32_t i = 0; i < bezierControlPoints.size() - 1; ++i) {
			indices.push_back(i);
			indices.push_back(i+1);
		}
		Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices);

		mesh->SetMaterial(material);
		mesh->SetDrawMode(DrawMode::Lines);

		s_Data.ModelBuffer.TransformMatrix = Math::Identity<Mat4>();
		s_Data.ModelBuffer.NormalMatrix = Math::Identity<Mat4>();
		s_Data.ModelBuffer.EntityId = entityId;

		s_Data.ModelUniformBuffer->SetData(&s_Data.ModelBuffer, sizeof(RendererData::ModelData));
		s_Data.MaterialUniformBuffer->SetData(&material->GetMaterialsParameters(), sizeof(MaterialParameters));
		s_Data.CurveParametersBuffer->SetData(&s_Data.CurveBuffer, sizeof(RendererData::CurveParameters));

		MaterialField mat = material->Handle;
		VXM_CORE_ASSERT(mat, "The material ID({}) is not valid.", mat.GetHandle().string());
		Ref<Material> matPtr = mat.GetAsset();
		if(mat) {
			s_Data.MaterialUniformBuffer->SetData(&matPtr->GetMaterialsParameters(), sizeof(MaterialParameters));
			matPtr->Bind(false);
			s_BindedMaterial = mat;
		}

		ShaderField shader = matPtr->GetShaderHandle();
		VXM_CORE_ASSERT(shader, "The shader ID({}) from the material '{}' is not valid.", matPtr->GetMaterialName(), shader.GetHandle().string());
		if (shader) {
			shader.GetAsset()->Bind();
			s_BindedShader = shader;
		}
		mesh->Bind();
		RenderCommand::DrawPatches(vertices.size());
	}

	void Renderer::Submit(Ref<Material> material, const glm::vec3& controlPoint0, const glm::vec3& controlPoint1, const glm::vec3& controlPoint2, const glm::vec3& controlPoint3, int lineDefinition, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		Submit(material, {controlPoint0, controlPoint1, controlPoint2, controlPoint3}, lineDefinition, entityId);
	}

	void Renderer::Submit(Ref<Material> material, int degree, const std::vector<glm::vec3>& points, const std::vector<float>& nodes, const std::vector<float>& weights, int lineDefinition, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		VXM_CORE_ASSERT(points.size() <= s_Data.CurveBuffer.CurveControlPoints.size(), "The shader might won't support more than a {0} control point...", s_Data.CurveBuffer.CurveControlPoints.size());
		VXM_CORE_ASSERT(nodes.size() <= s_Data.CurveBuffer.CurveControlPoints.size(), "The shader might won't support more than a {0} nodes...", s_Data.CurveBuffer.CurveControlPoints.size());
		VXM_CORE_ASSERT(points.size() == weights.size(), "The number of weight and the number of points should be equal.");
		VXM_CORE_ASSERT(nodes.size() >= degree+1, "The number of nodes must be superior to {}.",degree+1);

		uint32_t controlPointCount = std::min(s_Data.CurveBuffer.CurveControlPoints.size(), points.size());
		uint32_t nodeCount = std::min(s_Data.CurveBuffer.CurveControlPoints.size(), nodes.size());

		std::vector<Vertex> vertices((nodeCount /3) + (nodeCount %3 ? 1 : 0));

		s_Data.CurveBuffer.MainCurveDegree = degree;
		s_Data.CurveBuffer.NumberOfSegment = lineDefinition;
		s_Data.CurveBuffer.MainCurveNumberOfControlPoint = static_cast<int>(points.size());
		s_Data.CurveBuffer.MainCurveNumberOfKnot = static_cast<int>(nodeCount);
		for (int i = 0; i < controlPointCount; ++i) {
			s_Data.CurveBuffer.CurveControlPoints[i] = glm::vec4(points[i],1);
			s_Data.CurveBuffer.CurveWeights[i] = weights[i];
		}

		for (int i = 0; i < nodeCount; ++i) {
			vertices[i/3].Position[i%3] = float(nodes[i]);
		}

		std::vector<uint32_t> indices;
		indices.reserve((nodeCount-1)*2);
		for (uint32_t i = 0; i < nodeCount - 1; ++i) {
			indices.push_back(i);
			indices.push_back(i+1);
		}
		Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices);

		mesh->SetMaterial(material);
		mesh->SetDrawMode(DrawMode::Lines);

		s_Data.ModelBuffer.TransformMatrix = Math::Identity<Mat4>();
		s_Data.ModelBuffer.NormalMatrix = Math::Identity<Mat4>();
		s_Data.ModelBuffer.EntityId = entityId;

		s_Data.ModelUniformBuffer->SetData(&s_Data.ModelBuffer, sizeof(RendererData::ModelData));
		s_Data.MaterialUniformBuffer->SetData(&material->GetMaterialsParameters(), sizeof(MaterialParameters));
		s_Data.CurveParametersBuffer->SetData(&s_Data.CurveBuffer, sizeof(RendererData::CurveParameters));

		MaterialField mat = material->Handle;
		VXM_CORE_ASSERT(mat, "The material ID({}) is not valid.", mat.GetHandle().string());
		Ref<Material> matPtr = mat.GetAsset();
		if(mat) {
			s_Data.MaterialUniformBuffer->SetData(&matPtr->GetMaterialsParameters(), sizeof(MaterialParameters));
			matPtr->Bind(false);
			s_BindedMaterial = mat;
		}

		ShaderField shader = matPtr->GetShaderHandle();
		VXM_CORE_ASSERT(shader, "The shader ID({}) from the material '{}' is not valid.", matPtr->GetMaterialName(), shader.GetHandle().string());
		if (shader) {
			shader.GetAsset()->Bind();
			s_BindedShader = shader;
		}

		mesh->Bind();
		RenderCommand::DrawPatches(vertices.size());
	}

	void Renderer::Submit(Ref<Material> material, const CurveParams &mainCurve, const CurveParams &profileCurve, int lineDefinition, int entityId)
	{
		VXM_PROFILE_FUNCTION();
		// VXM_CORE_ASSERT(mainCurve.Points.size() <= s_Data.CurveBuffer.CurveControlPoints.size(), "The shader might won't support more than a {0} control point...", s_Data.CurveBuffer.CurveControlPoints.size());
		// VXM_CORE_ASSERT(mainCurve.Nodes.size() <= s_Data.CurveBuffer.CurveControlPoints.size(), "The shader might won't support more than a {0} nodes...", s_Data.CurveBuffer.CurveControlPoints.size());
		// VXM_CORE_ASSERT(mainCurve.Points.size() == mainCurve.Weights.size(), "The number of weight and the number of points should be equal.");
		// VXM_CORE_ASSERT(mainCurve.Nodes.size() >= mainCurve.Degree+1, "The number of nodes must be superior to {}.",mainCurve.Degree+1);
		//
		// VXM_CORE_ASSERT(profileCurve.Points.size() <= s_Data.CurveBuffer.CurveControlPoints.size(), "The shader might won't support more than a {0} control point...", s_Data.CurveBuffer.CurveControlPoints.size());
		// VXM_CORE_ASSERT(profileCurve.Nodes.size() <= s_Data.CurveBuffer.CurveControlPoints.size(), "The shader might won't support more than a {0} nodes...", s_Data.CurveBuffer.CurveControlPoints.size());
		// VXM_CORE_ASSERT(profileCurve.Points.size() == profileCurve.Weights.size(), "The number of weight and the number of points should be equal.");
		// VXM_CORE_ASSERT(profileCurve.Nodes.size() >= profileCurve.Degree+1, "The number of nodes must be superior to {}.",profileCurve.Degree+1);

		int mainPointCount = std::min(mainCurve.Points.size(), s_Data.CurveBuffer.CurveControlPoints.size());
		int profilePointCount = std::min(profileCurve.Points.size(), s_Data.CurveBuffer.ProfileControlPoints.size());

//		std::vector<Vertex> vertices((RendererData::NUM_CONTROL_POINTS_MAX / 3) + (profilePointCount / 3) + (profilePointCount % 3 ? 1 : 0));

		s_Data.CurveBuffer.NumberOfSegment = lineDefinition;

		s_Data.CurveBuffer.MainCurveNumberOfControlPoint = mainPointCount;
		s_Data.CurveBuffer.MainCurveDegree = mainCurve.Degree;

		s_Data.CurveBuffer.ProfileNumberOfControlPoint = profilePointCount;
		s_Data.CurveBuffer.ProfileDegree = profileCurve.Degree;

		s_Data.CurveBuffer.MainCurveNumberOfKnot = static_cast<int>(mainPointCount);


		for (int i = 0; i < mainPointCount; ++i) {
			s_Data.CurveBuffer.CurveControlPoints[i] = glm::vec4(mainCurve.Points[i],1);
			// s_Data.CurveBuffer.CurveWeights[i] = mainCurve.Weights[i];
		}

		for (int i = 0; i < profilePointCount; ++i) {
			s_Data.CurveBuffer.ProfileControlPoints[i] = glm::vec4(profileCurve.Points[i],1);
			// s_Data.CurveBuffer.ProfileWeights[i] = profileCurve.Weights[i];
		}

//		for (int i = 0; i < mainPointCount; ++i) {
//			vertices[i/3].Position[i%3] = float(i);
//		}
//
//		for (int i = RendererData::NUM_CONTROL_POINTS_MAX; i < RendererData::NUM_CONTROL_POINTS_MAX + profilePointCount; ++i) {
//			vertices[i/3].Position[i%3] = float(i - RendererData::NUM_CONTROL_POINTS_MAX);
//		}
//
//		std::vector<uint32_t> indices;
//		indices.reserve(((mainPointCount + profilePointCount) - 1) * 2);
//		for (uint32_t i = 0; i < mainPointCount - 1; ++i) {
//			indices.push_back(i);
//			indices.push_back(i+1);
//		}
//		for (uint32_t i = 0; i < profilePointCount - 1; ++i) {
//			indices.push_back(RendererData::NUM_CONTROL_POINTS_MAX + i);
//			indices.push_back(RendererData::NUM_CONTROL_POINTS_MAX + i+1);
//		}

		std::vector<Vertex> vertices {{
				Vertex{{-1, 0, -1}},
				Vertex{{+1, 0, -1}},
				Vertex{{+1, 0, +1}},
				Vertex{{-1, 0, +1}},
		}};

		std::vector<uint32_t> indices {{
				0,1,2,3
		}};
		Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices);

		mesh->SetMaterial(material);
		mesh->SetDrawMode(DrawMode::Triangles);

		s_Data.ModelBuffer.TransformMatrix = Math::Identity<Mat4>();
		s_Data.ModelBuffer.NormalMatrix = Math::Identity<Mat4>();
		s_Data.ModelBuffer.EntityId = entityId;

		s_Data.ModelUniformBuffer->SetData(&s_Data.ModelBuffer, sizeof(RendererData::ModelData));
		s_Data.MaterialUniformBuffer->SetData(&material->GetMaterialsParameters(), sizeof(MaterialParameters));
		s_Data.CurveParametersBuffer->SetData(&s_Data.CurveBuffer, sizeof(RendererData::CurveParameters));

		MaterialField mat = material->Handle;
		VXM_CORE_ASSERT(mat, "The material ID({}) is not valid.", mat.GetHandle().string());
		Ref<Material> matPtr = mat.GetAsset();
		if(mat) {
			s_Data.MaterialUniformBuffer->SetData(&matPtr->GetMaterialsParameters(), sizeof(MaterialParameters));
			matPtr->Bind(false);
			s_BindedMaterial = mat;
		}

		ShaderField shader = matPtr->GetShaderHandle();
		VXM_CORE_ASSERT(shader, "The shader ID({}) from the material '{}' is not valid.", matPtr->GetMaterialName(), shader.GetHandle().string());
		if (shader) {
			shader.GetAsset()->Bind();
			s_BindedShader = shader;
		}

		mesh->Bind();
		RenderCommand::DrawPatches(vertices.size());
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		VXM_PROFILE_FUNCTION();
		RenderCommand::SetViewport(0,0,width,height);
	}
} // Core