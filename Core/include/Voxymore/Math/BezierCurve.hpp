//
// Created by ianpo on 21/04/2024.
//

#pragma once

#include "Voxymore/Math/Math.hpp"
#include <vector>


namespace Voxymore::Core
{

	template<typename T>
	class BezierCurve
	{
	public:
		using BezierIterator = std::vector<T>::iterator;
		using ConstBezierIterator = std::vector<T>::const_iterator;
	public:
		BezierCurve() = default;
		~BezierCurve() = default;
		inline BezierCurve(const std::vector<T> & p) : m_ControlPoints(p) {}
		inline BezierCurve(const std::initializer_list<T>& p) : m_ControlPoints(p) {}

		inline void SetControlPoints(const std::vector<T>& p) {m_ControlPoints = p;}
		inline const std::vector<T>& GetControlPoints() const {return m_ControlPoints;}

		T Evaluate(Real t);
		T operator()(Real t) {return Evaluate(t);}

		T& operator[](uint64_t i) {return m_ControlPoints[i];}
		const T& operator[](uint64_t i) const {return m_ControlPoints[i];}
		uint64_t size() const {return m_ControlPoints.size();}

		BezierIterator begin() {return m_ControlPoints.begin();}
		BezierIterator end() {return m_ControlPoints.end();}

		ConstBezierIterator cbegin() const {return m_ControlPoints.cbegin();}
		ConstBezierIterator cend() const {return m_ControlPoints.cend();}
	private:
		float BinomialCoeff(int n, int k);
		std::vector<T> m_ControlPoints;
	};

	template<typename T>
	float BezierCurve<T>::BinomialCoeff(int n, int k) {
		VXM_PROFILE_FUNCTION();
		int res = 1;

		for (int i = 0; i < k; ++i)
		{
			res *= (n - i);
			res /= (i + 1);
		}

		return res;
	}

	template<typename T>
	inline T BezierCurve<T>::Evaluate(Real t)
	{
		VXM_PROFILE_FUNCTION();
		VXM_CORE_CHECK(0 <= t && t <= 1, "t({}) is not within 0 & 1. Behaviour undefined.", t);
		T tmp[m_ControlPoints.size()];
		const int64_t count = m_ControlPoints.size();
		std::memcpy(tmp, m_ControlPoints.data(), count * sizeof(T));

		T finalPos = T(0);
		for (int64_t i = 0; i < count; ++i) {
			Real binom = BinomialCoeff(count - 1, i);
			Real bernstein = binom * Math::Pow<Real>(Real(1) - t, count - 1 - i) * Math::Pow<Real>(t, i);
			finalPos += tmp[i] * bernstein;
		}
	}

//	template<typename T>
//	T BezierCurve<T>::Evaluate(Real t)
//	{
//		VXM_PROFILE_FUNCTION();
//		VXM_CORE_CHECK(0 <= t && t <= 1, "t({}) is not within 0 & 1. Behaviour undefined.", t);
//		T tmp[m_ControlPoints.size()];
//		const uint64_t count = m_ControlPoints.size();
//		std::memcpy(tmp, m_ControlPoints.data(), count * sizeof(T));
//
//
//		// Application de l'algorithme de De Casteljau
//		for (uint64_t j = 1; j < count; j++) {
//			for (uint64_t i = 0; i < count - j; i++) {
//				tmp[i] = (1.0 - t) * tmp[i] + t * tmp[i+1];
//			}
//		}
//
//		return tmp[0];
//	}

} // namespace Voxymore::Core

