//
// Created by ianpo on 24/04/2024.
//

#pragma once

#include "Voxymore/Components/CustomComponent.hpp"

namespace Voxymore::Core
{
	struct HullComponent : public Component<HullComponent>
	{
		VXM_IMPLEMENT_COMPONENT(HullComponent);
	public:
		inline void DeserializeComponent(YAML::Node& node) {}
		inline void SerializeComponent(YAML::Emitter& out) {}
		inline bool OnImGuiRender() {return false;}
	private:
		bool someUselessVariable = false;
	};

	VXM_CREATE_COMPONENT(HullComponent)
}
