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
		std::vector<T> tmp(m_ControlPoints.size());
		const int64_t count = m_ControlPoints.size();
		std::memcpy(tmp.data(), m_ControlPoints.data(), count * sizeof(T));

		T finalPos = T(0);
		for (int64_t i = 0; i < count; ++i) {
			Real binom = BinomialCoeff(count - 1, i);
			Real bernstein = binom * Math::Pow<Real>(Real(1) - t, count - 1 - i) * Math::Pow<Real>(t, i);
			finalPos += tmp[i] * bernstein;
		}
		return finalPos;
	}

	using BezierCurve3 = BezierCurve<glm::vec3>;

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

	// Returns true if the lines intersect, otherwise false. In addition, if the lines
	// intersect the intersection point may be stored in the floats i_x and i_y.
	inline bool get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y, float p3_x, float p3_y, float *i_x = nullptr, float *i_y = nullptr)
	{
		//calculate s1 & s2. Vectors from p0 to p1 & p2 to p3.
		float s1_x, s1_y, s2_x, s2_y;
		// Calculate S1
		s1_x = p1_x - p0_x;
		s1_y = p1_y - p0_y;
		// Calculate S2
		s2_x = p3_x - p2_x;
		s2_y = p3_y - p2_y;

		// Calculate s & t such as p0 + t * s1 == p3 + s * s2 is true
		float s, t;
		s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
		t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

		// check both s & t are in their respective segment
		if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
		{
			// Collision detected
			if (i_x) *i_x = p0_x + (t * s1_x);
			if (i_y) *i_y = p0_y + (t * s1_y);
			return true;
		}

		return false; // No collision
	}

	inline std::optional<glm::vec3> GetIntersectionPoint(const std::vector<glm::vec3>& envelop1, const std::vector<glm::vec3>& envelop2, float pas = 0.001)
	{
		auto b1 = BezierCurve3(envelop1);
		auto b2 = BezierCurve3(envelop2);

		for (float t1 = 0; t1 <= 1-pas; t1+=pas) {

			glm::vec3 p = b1.Evaluate(t1);
			glm::vec3 p1 = b1.Evaluate(t1 + pas);
			glm::vec3 r = p1 - p;

			for (float t2 = 0; t2 <= 1-pas; t2 += pas) {

				glm::vec3 q = b2.Evaluate(t2);
				glm::vec3 q1 = b2.Evaluate(t2 + pas);
				glm::vec3 s = q1 - q;

				float x, y;
				if(get_line_intersection(p.x, p.y, p1.x, p1.y, q.x, q.y, q1.x, q1.y, &x, &y)) {
					return glm::vec3{x,y,p.z};
				}
			}
		}
		return {};
	}

} // namespace Voxymore::Core

