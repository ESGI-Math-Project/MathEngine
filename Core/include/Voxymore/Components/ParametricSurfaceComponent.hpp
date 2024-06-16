//
// Created by ianpo on 03/06/2024.
//

#pragma once


#include "CustomComponent.hpp"
#include "Voxymore/Core/Core.hpp"
#include "Voxymore/Math/BezierCurve.hpp"
#include "Voxymore/Math/CurveParams.hpp"
#include "Voxymore/Math/Math.hpp"
#include "Voxymore/Math/Nurbs.hpp"
#include "Voxymore/Renderer/Material.hpp"
#include "Voxymore/Renderer/Renderer.hpp"
#include <variant>

namespace Voxymore::Core
{

	class ParametricSurfaceComponent : public SelfAwareComponent<ParametricSurfaceComponent>
	{
		VXM_IMPLEMENT_NAME(ParametricSurfaceComponent);
	public:

		static constexpr const inline unsigned int CurveTypeCount = 3;
	public:
		void DeserializeComponent(YAML::Node& node, Entity e);
		void SerializeComponent(YAML::Emitter& out, Entity e);
		bool OnImGuiRender(Entity e);
		bool OnImGuizmo(Entity e, const float* viewMatrix, const float* projectionMatrix);
	public:
		std::vector<glm::vec3> GetMainCurveWorldPoints(const glm::mat4& localToWorld) const;
		std::vector<glm::vec3> GetProfileWorldPoints(const glm::mat4& localToWorld) const;

		CurveParams GetMainCurveParams(const glm::mat4& localToWorld) const;
		CurveParams GetProfileCurveParams(const glm::mat4& localToWorld) const;
	public:
		MaterialField Material;
		int Definition = 100;

		CurveType MainCurveType;
		int MainCurveDegree = 2;
		std::vector<glm::vec3> MainCurve;
		std::vector<float> MainCurveWeights;

		CurveType ProfileType;
		int ProfileDegree = 2;
		std::vector<glm::vec3> Profile;
		std::vector<float> ProfileWeights;

		RendererData::TesselationControlParams TesscoParams{};
		float simpleTesscoParams = 16;
		bool useSimpleTessco = true;
	private:
		static bool ImGuiPolygonCurve(const std::string& name, std::vector<glm::vec3>& curve);
		static bool ImGuiBezierCurve(const std::string& name, std::vector<glm::vec3>& curve, int& degree);
		static bool ImGuiNurbsCurve(const std::string& name, std::vector<glm::vec3>& curve, std::vector<float>& weights);
		static bool ImGuiCurveTypeCombo(const std::string& name, CurveType& curve);

		bool DrawMainCurveGizmos(Entity e, const float* viewMatrix, const float* projectionMatrix);
		bool DrawProfileCurveGizmos(Entity e, const float* viewMatrix, const float* projectionMatrix);

		template<typename T>
		static void DeserializeVector(YAML::Node sequenceNode, std::vector<T>& vector, const T& defaultValue)
		{
			if(!sequenceNode.IsSequence()) {
				VXM_CORE_ERROR("The Node '{}' is not a Sequence node.", sequenceNode.Tag());
				return;
			}
			vector.clear();
			vector.reserve(sequenceNode.size());
			for (auto nodeValue : sequenceNode)
			{
				vector.push_back(nodeValue.as<T>(defaultValue));
			}
		}

		template<typename T>
		static void SerializeVector(YAML::Emitter& out, const std::string& name, std::vector<T>& vector)
		{
			out << KEYVAL(name, YAML::BeginSeq);
			{
				for(const auto& val : vector)
				{
					out << val;
				}
			}
			out << YAML::EndSeq;
		}
	private:
		enum class ControlledGizmos : uint8_t
		{
			None,
			MainCurve,
			ProfileCurve,
			Both,
		};
		ControlledGizmos m_ControlledGizmos = ControlledGizmos::None;
	};

	VXM_CREATE_COMPONENT(ParametricSurfaceComponent);
} // namespace Voxymore::Core

