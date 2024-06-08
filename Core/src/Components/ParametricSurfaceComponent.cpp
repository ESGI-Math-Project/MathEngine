//
// Created by ianpo on 03/06/2024.
//

#include "Voxymore/Components/ParametricSurfaceComponent.hpp"
#include "Voxymore/ImGui/ImGuiLib.hpp"

namespace Voxymore::Core
{
	ParametricSurfaceComponent::CurveType ParametricSurfaceComponent::CurveFromString(const std::string &curve)
	{
		if (curve == "Polygon") return CurveType::Polygon;
		else if (curve == "Bezier") return CurveType::Bezier;
		else if (curve == "Nurbs") return CurveType::Nurbs;

		VXM_CORE_ASSERT(false, "Curve '{}' doesnt exist.", curve);
		return CurveType::Polygon;
	}

	std::string ParametricSurfaceComponent::CurveToString(ParametricSurfaceComponent::CurveType curve)
	{
		switch (curve) {
			case CurveType::Polygon: return "Polygon";
			case CurveType::Bezier: return "Bezier";
			case CurveType::Nurbs: return "Nurbs";
		}
		VXM_CORE_ASSERT(false, "Curve '{}' doesnt exist.", (uint8_t) curve);
		return "Unknown";
	}

	void ParametricSurfaceComponent::DeserializeComponent(YAML::Node& node, Entity e)
	{
		Material = node["Material"].as<MaterialField>(NullAssetHandle);
		Definition = node["Definition"].as<int>(100);

		auto mainCurveNode = node["Main Curve"];
		{
			MainCurveDegree = mainCurveNode["Degree"].as<int>(2);
			MainCurveType = CurveFromString(mainCurveNode["Curve Type"].as<std::string>());
			DeserializeVector(mainCurveNode["Curve"], MainCurve, glm::vec3(0));
			DeserializeVector(mainCurveNode["Weights"], MainCurveWeights, float(1));
		}

		auto profileNode = node["Profile"];
		{
			ProfileDegree = profileNode["Degree"].as<int>(2);
			ProfileType = CurveFromString(profileNode["Curve Type"].as<std::string>());
			DeserializeVector(profileNode["Curve"], Profile, glm::vec3(0));
			DeserializeVector(profileNode["Weights"], ProfileWeights, float(1));
		}
	}

	void ParametricSurfaceComponent::SerializeComponent(YAML::Emitter &out, Entity e)
	{
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
		bool changed = false;

		changed |= ImGuiLib::DrawAssetField("Material", &Material);
		changed |= ImGui::DragInt("Definition", &Definition);
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
		return false;
	}

	std::vector<glm::vec3> ParametricSurfaceComponent::GetMainCurveWorldPoints(const glm::mat4& localToWorld)
	{
		std::vector<glm::vec3> points(MainCurve.size());
		std::transform(MainCurve.begin(), MainCurve.end(), points.begin(), [&localToWorld](const glm::vec3& p) {return Math::TransformPoint(localToWorld, p);});
		return points;
	}

	std::vector<glm::vec3> ParametricSurfaceComponent::GetProfileWorldPoints(const glm::mat4& localToWorld)
	{
		std::vector<glm::vec3> points(Profile.size());
		std::transform(Profile.begin(), Profile.end(), points.begin(), [&localToWorld](const glm::vec3& p) {return Math::TransformPoint(localToWorld, p);});
		return points;
	}

	bool ParametricSurfaceComponent::ImGuiPolygonCurve(const std::string &name, std::vector<glm::vec3> &curve)
	{
		bool changed = false;
		ImGui::PushID(name.c_str());
		changed |= ImGuiLib::DrawVector<glm::vec3>((name + " Polygon").c_str(), &curve, [](const char* name, glm::vec3* p){return ImGuiLib::DragReal3(name, glm::value_ptr(*p));}, glm::vec3(0));
		ImGui::PopID();
		return changed;
	}
	bool ParametricSurfaceComponent::ImGuiBezierCurve(const std::string &name, std::vector<glm::vec3> &curve, int &degree)
	{
		bool changed = false;
		ImGui::PushID(name.c_str());
		changed |= ImGui::DragInt((name + " Degree").c_str(), &degree, 1, 1, INT_MAX);
		changed |= ImGuiLib::DrawVector<glm::vec3>((name + " Bezier Curve").c_str(), &curve, [](const char* name, glm::vec3* p){return ImGuiLib::DragReal3(name, glm::value_ptr(*p));}, glm::vec3(0));
		ImGui::PopID();
		return changed;
	}
	bool ParametricSurfaceComponent::ImGuiNurbsCurve(const std::string &name, std::vector<glm::vec3> &curve, std::vector<float> &weights)
	{
		bool changed = false;
		ImGui::PushID(name.c_str());
		changed |= ImGuiLib::DrawVector<glm::vec3>((name + " Nurbs Curve").c_str(), &curve, [](const char* name, glm::vec3* p){return ImGuiLib::DragReal3(name, glm::value_ptr(*p));}, glm::vec3(0));
		changed |= ImGuiLib::DrawVector<float>((name + " Weight").c_str(), &weights, VXM_BIND_FN(ImGui::DragFloat), 0, curve.size(), curve.size());
		ImGui::PopID();
		return changed;
	}

	bool ParametricSurfaceComponent::ImGuiCurveTypeCombo(const std::string &name, CurveType &curve)
	{
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
}// namespace Voxymore::Core