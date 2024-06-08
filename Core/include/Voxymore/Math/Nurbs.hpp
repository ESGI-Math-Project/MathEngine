//
// Created by ianpo on 03/06/2024.
//

#pragma once

#include "Voxymore/Math/Math.hpp"
#include <vector>

namespace Voxymore::Core
{

	template<typename T>
	class Nurbs
	{
	public:
		using NurbsIterator = std::vector<T>::iterator;
		using ConstNurbsIterator = std::vector<T>::const_iterator;
	public:
		Nurbs() = default;
		~Nurbs() = default;

		inline Nurbs(const std::vector<T> & p) : m_ControlPoints(p) {}
		inline Nurbs(const std::initializer_list<T>& p) : m_ControlPoints(p) {}

		inline void SetControlPoints(const std::vector<T>& p) {m_ControlPoints = p;}
		inline const std::vector<T>& GetControlPoints() const {return m_ControlPoints;}

		T Evaluate(Real t);
		T operator()(Real t) {return Evaluate(t);}

		T& operator[](uint64_t i) {return m_ControlPoints[i];}
		const T& operator[](uint64_t i) const {return m_ControlPoints[i];}
		uint64_t size() const {return m_ControlPoints.size();}

		NurbsIterator begin() {return m_ControlPoints.begin();}
		NurbsIterator end() {return m_ControlPoints.end();}

		ConstNurbsIterator cbegin() const {return m_ControlPoints.cbegin();}
		ConstNurbsIterator cend() const {return m_ControlPoints.cend();}
	private:
		std::vector<T> m_ControlPoints;
		std::vector<uint64_t> m_Nodes;
	};


	template<typename T>
	inline T Nurbs<T>::Evaluate(Real t)
	{
		VXM_PROFILE_FUNCTION();
		//TODO: Do the thing.
		return T{};
	}

	using Nurbs3 = Nurbs<glm::vec3>;

} // namespace Voxymore::Core

