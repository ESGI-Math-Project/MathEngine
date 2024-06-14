//
// Created by ianpo on 13/06/2024.
//

#pragma once

#include <vector>
#include <string>
#include <utility>
#include <glm/vec3.hpp>
#include "Voxymore/Core/Macros.hpp"
#include "Voxymore/Core/Logger.hpp"

namespace Voxymore::Core
{
	enum class CurveType : int {
		Polygon,
		Bezier,
		Nurbs,
	};

	static inline std::string CurveToString(CurveType curve)
	{
		switch (curve) {
			case CurveType::Polygon: return "Polygon";
			case CurveType::Bezier: return "Bezier";
			case CurveType::Nurbs: return "Nurbs";
		}
		VXM_CORE_ASSERT(false, "Curve '{}' doesnt exist.", (uint8_t) curve);
		return "Unknown";
	}

	static inline CurveType CurveFromString(const std::string& curve)
	{
		if (curve == "Polygon") return CurveType::Polygon;
		else if (curve == "Bezier") return CurveType::Bezier;
		else if (curve == "Nurbs") return CurveType::Nurbs;

		VXM_CORE_ASSERT(false, "Curve '{}' doesnt exist.", curve);
		return CurveType::Polygon;
	}

	struct CurveParams {
		CurveParams() = default;
		~CurveParams() = default;
		inline CurveParams(std::vector<glm::vec3> points, std::vector<float> nodes, std::vector<float> weights, int curveType, int degree) : Points(std::move(points)), Nodes(std::move(nodes)), Weights(std::move(weights)), Type((CurveType)curveType), Degree(degree) {}

		std::vector<glm::vec3> Points;
		std::vector<float> Nodes;
		std::vector<float> Weights;
		CurveType Type;
		int Degree;
	};

}