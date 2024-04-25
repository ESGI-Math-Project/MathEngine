//
// Created by ianpo on 24/04/2024.
//

#pragma once

#include <Voxymore/Voxymore.hpp>
#include "Voxymore/Editor/EditorLayer.hpp"

class HullToCurve : public Voxymore::Core::System
{
public:
	VXM_IMPLEMENT_SYSTEM(HullToCurve)
public:
	virtual bool OnImGuiRender() override;

};