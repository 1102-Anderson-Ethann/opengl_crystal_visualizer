#pragma once
#include "CIFParser.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

struct XRDPoint {
	glm::vec3 position; //from 2 theta and phi
	float intensity;
	int h, k, l;
};

//XRD interface
class XRDPoints
{
	public:
		int getXRDPointCount();

		//PLACEHolder
		static std::vector<XRDPoint> genXRD(const CIFData& cifData);
		static void getLatticeVectors(const LatticeParameters& lat,
			glm::vec3& A, glm::vec3& B, glm::vec3& C);
		static void getReciprocalVectors(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C,
			glm::vec3& Astar, glm::vec3& Bstar, glm::vec3& Cstar);
		static float scatteringFactor(const std::string& element, float Q);

		//Later XRD data loader goes here
	private:
		int xrdPointCount;


};

