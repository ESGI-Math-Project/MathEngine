//
// Created by ianpo on 03/06/2024.
//

#include "Voxymore/Components/ParametricSurfaceComponent.hpp"
#include "Voxymore/Components/Components.hpp"
#include "Voxymore/ImGui/ImGuiLib.hpp"
#include <ImGuizmo.h>

namespace Voxymore::Core
{

	void ParametricSurfaceComponent::DeserializeComponent(YAML::Node& node, Entity e)
	{
		VXM_PROFILE_FUNCTION();
		Material = node["Material"].as<MaterialField>(NullAssetHandle);
		Definition = node["Definition"].as<int>(100);

		auto mainCurveNode = node["Main Curve"];
		{
			MainCurveDegree = mainCurveNode["Degree"].as<int>(2);
			MainCurveType = CurveFromString(mainCurveNode["Curve Type"].as<std::string>(std::string {}));
			DeserializeVector(mainCurveNode["Curve"], MainCurve, glm::vec3(0));
			DeserializeVector(mainCurveNode["Weights"], MainCurveWeights, float(1));
		}

		auto profileNode = node["Profile"];
		{
			ProfileDegree = profileNode["Degree"].as<int>(2);
			ProfileType = CurveFromString(profileNode["Curve Type"].as<std::string>(std::string {}));
			DeserializeVector(profileNode["Curve"], Profile, glm::vec3(0));
			DeserializeVector(profileNode["Weights"], ProfileWeights, float(1));
		}
	}

	void ParametricSurfaceComponent::SerializeComponent(YAML::Emitter &out, Entity e)
	{
		VXM_PROFILE_FUNCTION();
		out << KEYVAL("Material", Material);
		out << KEYVAL("Definition", Definition);

		out << KEYVAL("Main Curve", YAML::BeginMap);
		{
			out << KEYVAL("Curve Type", CurveToString(MainCurveType));
			SerializeVector(out, "Weights", MainCurveWeights);
			out << KEYVAL("Degree", MainCurveDegree);
			SerializeVector(out, "Curve", MainCurve);
		}
		out << YAML::EndMap;

		out << KEYVAL("Profile", YAML::BeginMap);
		{
			out << KEYVAL("Curve Type", CurveToString(ProfileType));
			SerializeVector(out, "Weights", ProfileWeights);
			out << KEYVAL("Degree", ProfileDegree);
			SerializeVector(out, "Curve", Profile);
		}
		out << YAML::EndMap;
	}

	bool ParametricSurfaceComponent::OnImGuiRender(Entity e)
	{
		VXM_PROFILE_FUNCTION();
		bool changed = false;

		changed |= ImGuiLib::DrawAssetField("Material", &Material);
		changed |= ImGui::DragInt("Definition", &Definition);

		{
			uint8_t controlledGizmoIndex = static_cast<uint8_t>(m_ControlledGizmos);
			static std::array<std::string, 4> cgnames = {"None", "Main Curve", "Profile Curve", "Both"};

			if (ImGui::BeginCombo("Controlled Gizmo", cgnames[controlledGizmoIndex].c_str()))
			{
				for (uint8_t n = 0; n < (uint8_t)cgnames.size(); n++)
				{
					bool isSelected = (controlledGizmoIndex == n);
					if (ImGui::Selectable(cgnames[n].c_str(), isSelected)) {
						controlledGizmoIndex = n;
						m_ControlledGizmos = static_cast<ControlledGizmos>(n);
						changed |= true;
					}

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}

		ImGui::PushID("Main Curve");
		if(ImGui::CollapsingHeader("Main Curve")) {
			changed |= ImGuiCurveTypeCombo("Type", MainCurveType);
			switch(MainCurveType) {
				case CurveType::Polygon: changed |= ImGuiPolygonCurve("Polygon Curve", MainCurve); break;
				case CurveType::Bezier: changed |= ImGuiBezierCurve("Bezier Curve", MainCurve, MainCurveDegree); break;
				case CurveType::Nurbs: changed |= ImGuiNurbsCurve("Nurbs Curve", MainCurve, MainCurveWeights); break;
			}
		}
		ImGui::PopID();

		ImGui::Separator();

		ImGui::PushID("Profile");
		if(ImGui::CollapsingHeader("Profile")) {
			changed |= ImGuiCurveTypeCombo("Type", ProfileType);
			switch(ProfileType) {
				case CurveType::Polygon: changed |= ImGuiPolygonCurve("Polygon Curve", Profile); break;
				case CurveType::Bezier: changed |= ImGuiBezierCurve("Bezier Curve", Profile, ProfileDegree); break;
				case CurveType::Nurbs: changed |= ImGuiNurbsCurve("Nurbs Curve", Profile, ProfileWeights); break;
			}
		}
		ImGui::PopID();

		return changed;
	}

	bool ParametricSurfaceComponent::OnImGuizmo(Entity e, const float* viewMatrix, const float* projectionMatrix)
	{
		VXM_PROFILE_FUNCTION();
		bool changed = false;
		switch (m_ControlledGizmos)
		{
			case ControlledGizmos::MainCurve:
				changed |= DrawMainCurveGizmos(e, viewMatrix, projectionMatrix);
				break;
			case ControlledGizmos::ProfileCurve:
				changed |= DrawProfileCurveGizmos(e, viewMatrix, projectionMatrix);
				break;
			case ControlledGizmos::Both:
				changed |= DrawMainCurveGizmos(e, viewMatrix, projectionMatrix);
				changed |= DrawProfileCurveGizmos(e, viewMatrix, projectionMatrix);
				break;
		}
		return changed;
	}

	std::vector<glm::vec3> ParametricSurfaceComponent::GetMainCurveWorldPoints(const glm::mat4& localToWorld) const
	{
		VXM_PROFILE_FUNCTION();
		std::vector<glm::vec3> points(MainCurve.size());
		std::transform(MainCurve.begin(), MainCurve.end(), points.begin(), [&localToWorld](const glm::vec3& p) {return Math::TransformPoint(localToWorld, p);});
		return points;
	}

	std::vector<glm::vec3> ParametricSurfaceComponent::GetProfileWorldPoints(const glm::mat4& localToWorld) const
	{
		VXM_PROFILE_FUNCTION();
		std::vector<glm::vec3> points(Profile.size());
		std::transform(Profile.begin(), Profile.end(), points.begin(), [&localToWorld](const glm::vec3& p) {return Math::TransformPoint(localToWorld, p);});
		auto begin = Profile[0];
		//Profile should be an offset in world coordinate
		std::transform(Profile.begin(), Profile.end(), points.begin(), [begin](const glm::vec3& p) {return p - begin;});
		return points;
	}

	CurveParams ParametricSurfaceComponent::GetMainCurveParams(const glm::mat4& localToWorld) const
	{
		VXM_PROFILE_FUNCTION();
		CurveParams params;

		params.Type = MainCurveType;
		params.Points = std::move(GetMainCurveWorldPoints(localToWorld));
		params.Weights = MainCurveWeights;
		params.Nodes = std::vector<float>(params.Points.size());
		params.Degree = MainCurveDegree;

		return params;
	}

	CurveParams ParametricSurfaceComponent::GetProfileCurveParams(const glm::mat4& localToWorld) const
	{
		VXM_PROFILE_FUNCTION();
		CurveParams params;

		params.Type = ProfileType;
		params.Points = std::move(GetProfileWorldPoints(localToWorld));
		params.Weights = ProfileWeights;
		params.Nodes = std::vector<float>(params.Points.size());
		params.Degree = ProfileDegree;

		return params;
	}

	bool ParametricSurfaceComponent::ImGuiPolygonCurve(const std::string &name, std::vector<glm::vec3> &curve)
	{
		VXM_PROFILE_FUNCTION();
		bool changed = false;
		ImGui::PushID(name.c_str());
		changed |= ImGuiLib::DrawVector<glm::vec3>((name + " Polygon").c_str(), &curve, [](const char* name, glm::vec3* p){return ImGuiLib::DragReal3(name, glm::value_ptr(*p));}, glm::vec3(0));
		ImGui::PopID();
		return changed;
	}
	bool ParametricSurfaceComponent::ImGuiBezierCurve(const std::string &name, std::vector<glm::vec3> &curve, int &degree)
	{
		VXM_PROFILE_FUNCTION();
		bool changed = false;
		ImGui::PushID(name.c_str());
		changed |= ImGui::DragInt((name + " Degree").c_str(), &degree, 1, 1, INT_MAX);
		changed |= ImGuiLib::DrawVector<glm::vec3>((name + " Bezier Curve").c_str(), &curve, [](const char* name, glm::vec3* p){return ImGuiLib::DragReal3(name, glm::value_ptr(*p));}, glm::vec3(0));
		ImGui::PopID();
		return changed;
	}
	bool ParametricSurfaceComponent::ImGuiNurbsCurve(const std::string &name, std::vector<glm::vec3> &curve, std::vector<float> &weights)
	{
		VXM_PROFILE_FUNCTION();
		bool changed = false;
		ImGui::PushID(name.c_str());
		changed |= ImGuiLib::DrawVector<glm::vec3>((name + " Nurbs Curve").c_str(), &curve, [](const char* name, glm::vec3* p){return ImGuiLib::DragReal3(name, glm::value_ptr(*p));}, glm::vec3(0));
		changed |= ImGuiLib::DrawVector<float>((name + " Weight").c_str(), &weights, VXM_BIND_FN(ImGui::DragFloat), 0, curve.size(), curve.size());
		ImGui::PopID();
		return changed;
	}

	bool ParametricSurfaceComponent::ImGuiCurveTypeCombo(const std::string &name, CurveType &curve)
	{
		VXM_PROFILE_FUNCTION();
		bool changed = false;
		int currCurveIndex = static_cast<int>(curve);

		if (ImGui::BeginCombo(name.c_str(), CurveToString(curve).c_str()))
		{
			for (uint8_t n = 0; n < CurveTypeCount; n++)
			{
				bool isSelected = (currCurveIndex == n);
				if (ImGui::Selectable(CurveToString((CurveType)n).c_str(), isSelected)) {
					currCurveIndex = n;
					curve = static_cast<CurveType>(n);
					changed = true;
				}

				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		return changed;
	}

	bool ParametricSurfaceComponent::DrawMainCurveGizmos(Entity e, const float *viewMatrix, const float *projectionMatrix)
	{
		VXM_PROFILE_FUNCTION();
		bool changed = false;
		glm::mat4 selfMat = e.GetComponent<TransformComponent>().GetTransform();
		glm::mat4 matrix;
		ImGuizmo::PushID("MainCurve");
		for (int i = 0; i < MainCurve.size(); ++i) {
			matrix = glm::translate(Math::TransformPoint(selfMat,MainCurve[i]));
			ImGuizmo::PushID(i);
			bool result = ImGuizmo::Manipulate(viewMatrix, projectionMatrix, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, glm::value_ptr(matrix));
			changed |= result;

			if(ImGuizmo::IsUsing())
			{
				Vec3 worldPos = matrix[3];
				MainCurve[i] = Math::TransformPoint(glm::inverse(selfMat), worldPos);
			}
			ImGuizmo::PopID();
		}
		ImGuizmo::PopID();
		return changed;
	}

	bool ParametricSurfaceComponent::DrawProfileCurveGizmos(Entity e, const float *viewMatrix, const float *projectionMatrix)
	{
		VXM_PROFILE_FUNCTION();
		bool changed = false;
		glm::mat4 selfMat = e.GetComponent<TransformComponent>().GetTransform();
		glm::mat4 matrix;
		ImGuizmo::PushID("Profile");
		for (int i = 0; i < Profile.size(); ++i) {
			matrix = glm::translate(Math::TransformPoint(selfMat,Profile[i]));
			ImGuizmo::PushID(i);
			bool result = ImGuizmo::Manipulate(viewMatrix, projectionMatrix, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, glm::value_ptr(matrix));
			changed |= result;

			if(ImGuizmo::IsUsing())
			{
				Vec3 worldPos = matrix[3];
				Profile[i] = Math::TransformPoint(glm::inverse(selfMat), worldPos);
			}
			ImGuizmo::PopID();
		}
		ImGuizmo::PopID();
		return changed;
	}
}// namespace Voxymore::Core