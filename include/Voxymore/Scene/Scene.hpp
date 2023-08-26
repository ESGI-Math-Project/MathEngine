//
// Created by ianpo on 24/08/2023.
//

#pragma once

#include "Voxymore/Core/TimeStep.hpp"
#include <entt/entt.hpp>

// TODO: find a better way?
namespace Voxymore::Editor {
	class SceneHierarchyPanel;
}

//TODO: add a way to create/delete system (that can go onto the whole scene) instead of juste NativeScriptComponent.
namespace Voxymore::Core
{
	class Entity;
	struct TagComponent;
	struct TransformComponent;
	struct MeshComponent;
	struct CameraComponent;
	struct NativeScriptComponent;

    class Scene
    {
	private:
		friend class Voxymore::Editor::SceneHierarchyPanel;
		friend class Entity;
    public:
        Scene();
        ~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnUpdate(TimeStep ts);
		void SetViewportSize(uint32_t width, uint32_t height);
	private:
		template<typename T>
		inline void OnComponentAdded(entt::entity entity, T& component);
private:
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
        entt::registry m_Registry;
    };
	template<>
	void Scene::OnComponentAdded<TagComponent>(entt::entity entity, TagComponent& tagComponent);

	template<>
	void Scene::OnComponentAdded<TransformComponent>(entt::entity entity, TransformComponent& transformComponent);

	template<>
	void Scene::OnComponentAdded<MeshComponent>(entt::entity entity, MeshComponent& meshComponent);

	template<>
	void Scene::OnComponentAdded<CameraComponent>(entt::entity entity, CameraComponent& cameraComponent);

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(entt::entity entity, NativeScriptComponent& nativeScriptComponent);

} // Voxymore
// Core
