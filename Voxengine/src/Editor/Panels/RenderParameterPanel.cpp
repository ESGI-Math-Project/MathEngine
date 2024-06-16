//
// Created by ianpo on 16/06/2024.
//

#include "Voxymore/Editor/Panels/RenderParameterPanel.hpp"

using namespace Voxymore::Core;

namespace Voxymore::Editor
{
	RenderParameterPanel::RenderParameterPanel()
	{

	}

	void RenderParameterPanel::OnImGuiRender()
	{
		std::string str = RendererAPIToString(RendererAPI::GetAPI());
		ImGui::Text("Current API : %s", str.c_str());

		ImGui::Spacing();

		if(!s_DrawWireframe && ImGui::Button("Draw Wireframe")) {
			s_DrawWireframe = true;
			RenderCommand::EnableWireframe(true);
		}

		if(s_DrawWireframe && ImGui::Button("Draw Polygon")) {
			s_DrawWireframe = false;
			RenderCommand::EnableWireframe(false);
		}
	}
} // namespace Voxymore::Editor
