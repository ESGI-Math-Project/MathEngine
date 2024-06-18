//
// Created by ianpo on 16/06/2024.
//

#pragma once


#include "Panel.hpp"
#include "Voxymore/Voxymore.hpp"

namespace Voxymore::Editor
{

	class RenderParameterPanel : public Panel<RenderParameterPanel>
	{
	public:
		VXM_IMPLEMENT_PANEL("Render Parameter");
	public:
		RenderParameterPanel();
		virtual void OnImGuiRender() override;

	private:
		static inline bool s_DrawWireframe = false;
		static inline bool s_DrawDoublesided = true;
	};

} // namespace Voxymore::Editor

