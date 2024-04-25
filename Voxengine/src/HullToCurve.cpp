//
// Created by ianpo on 24/04/2024.
//

#include "TestSystems/HullToCurve.hpp"
#include "Voxymore/Components/HullComponent.hpp"

using namespace Voxymore::Core;
using namespace Voxymore::Editor;

bool HullToCurve::OnImGuiRender()
{
	if(ImGui::Button("Create Bezier From Hull")) {
		auto* ptr = Application::Get().FindLayer<EditorLayer>();
		Ref<Scene> scene = ptr->GetActiveScene();
		std::vector<Vec3> points;
		auto view = scene->view<TransformComponent, HullComponent>(exclude<DisableComponent>);
		points.reserve(view.size_hint());
		for (auto e : view) {
			const auto& t = view.get<TransformComponent>(e);
			points.push_back(t.GetPosition());
		}
		Entity e = scene->CreateEntity("Bezier from Hull");
		e.AddComponent<GenericBezierCurve>(Math::ConvexHull<Real>(points));
	}

	if(ImGui::Button("Create BSpline From Hull")) {
		auto* ptr = Application::Get().FindLayer<EditorLayer>();
		Ref<Scene> scene = ptr->GetActiveScene();
		std::vector<Vec3> points;
		auto view = scene->view<TransformComponent, HullComponent>(exclude<DisableComponent>);
		points.reserve(view.size_hint());
		for (auto e : view) {
			const auto& t = view.get<TransformComponent>(e);
			points.push_back(t.GetPosition());
		}
		Entity e = scene->CreateEntity("BSpline from Hull");
		e.AddComponent<BSplinesComponents>(Math::ConvexHull<Real>(points));
	}

	return false;
}
