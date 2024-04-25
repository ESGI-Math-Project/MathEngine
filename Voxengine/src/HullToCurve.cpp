//
// Created by ianpo on 24/04/2024.
//

#include "TestSystems/HullToCurve.hpp"
#include "Voxymore/Components/HullComponent.hpp"
#include "Voxymore/ImGui/ImGuiLib.hpp"

using namespace Voxymore::Core;
using namespace Voxymore::Editor;

bool HullToCurve::OnImGuiRender()
{
	static UUID bezierOne = 0;
	static UUID bezierTwo = 0;

	static MaterialField bSplineMat = AssetHandle(0);
	static MaterialField bezierMat = AssetHandle(0);
	static MaterialField defaultMat = AssetHandle(0);

	auto* ptr = Application::Get().FindLayer<EditorLayer>();
	Ref<Scene> scene = ptr->GetActiveScene();

	ImGuiLib::InputEntityID("Bezier One", &bezierOne);
	ImGuiLib::InputEntityID("Bezier Two", &bezierTwo);

	ImGuiLib::DrawAssetField("BSpline Material", &bSplineMat);
	ImGuiLib::DrawAssetField("Bezier Material", &bezierMat);
	ImGuiLib::DrawAssetField("Default Material", &defaultMat);

	if(ImGui::Button("Create Bezier From Hull")) {
		std::vector<Vec3> points;
		auto view = scene->view<TransformComponent, HullComponent>(exclude<DisableComponent>);
		points.reserve(view.size_hint());
		for (auto e : view) {
			const auto& t = view.get<TransformComponent>(e);
			points.push_back(t.GetPosition());
		}
		Entity e = scene->CreateEntity("Bezier from Hull");
		auto& bez = e.AddComponent<GenericBezierCurve>(Math::ConvexHull<Real>(points));
		bez.m_Material = bezierMat;
	}

	if(ImGui::Button("Create BSpline From Hull")) {
		std::vector<Vec3> points;
		auto view = scene->view<TransformComponent, HullComponent>(exclude<DisableComponent>);
		points.reserve(view.size_hint());
		for (auto e : view) {
			const auto& t = view.get<TransformComponent>(e);
			points.push_back(t.GetPosition());
		}
		Entity e = scene->CreateEntity("BSpline from Hull");
		auto& bs = e.AddComponent<BSplinesComponents>(Math::ConvexHull<Real>(points));
		bs.m_Material = bSplineMat;
	}

	Entity eOne = scene->GetEntity(bezierOne);
	Entity eTwo = scene->GetEntity(bezierTwo);
	bool valid = eOne && eTwo;
	if (valid) {
		valid = eOne.HasComponent<GenericBezierCurve>() && eTwo.HasComponent<GenericBezierCurve>();
	}

	static float precision = 0.0001;
	ImGui::InputFloat("Precision", &precision, 0.0001f, 0.01f, "%.4f");
	precision = Math::Clamp(precision, 0.0001f, 1.f);
	ImGui::BeginDisabled(!valid);
	if(ImGui::Button("Create Cube at intersection")) {
		auto &bezOne = eOne.GetComponent<GenericBezierCurve>();
		auto &bezTwo = eTwo.GetComponent<GenericBezierCurve>();
		auto intersection = GetIntersectionPoint(bezOne.GetWorldPoints(eOne.GetComponent<TransformComponent>().GetTransform()), bezTwo.GetWorldPoints(eTwo.GetComponent<TransformComponent>().GetTransform()), precision);
		if(intersection) {
			Entity cube = scene->CreateEntity("Cube at Intersection");
			auto& prim = cube.AddComponent<PrimitiveComponent>(PrimitiveMesh::Type::Cube, defaultMat);
			cube.GetComponent<TransformComponent>().SetPosition(*intersection);
		} else {
			VXM_CORE_WARN("No intersection found.");
		}
	}
	ImGui::EndDisabled();

	return false;
}
