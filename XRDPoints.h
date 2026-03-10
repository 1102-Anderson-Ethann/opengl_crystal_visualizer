#pragma once
#include "CIFParser.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp> //pi

//lattice point struct
struct XRDPoint {
	glm::vec3 position; //3D loction in angstrom^-1
	float intensity; 
	int h, k, l; //miller indices for planes
};

//XRD interface
class XRDPoints
{
	public:

		//functions

		//generates recipricol space lattice points from cifData
		static std::vector<XRDPoint> genXRD(const CIFData& cifData);

		//converts a,b,c al,be,ga to vectors ABC to describe unit cell
		static void getLatticeVectors(const LatticeParameters& lat,
			glm::vec3& A, glm::vec3& B, glm::vec3& C);

		//uses cross prroduct to produce reciprocal space vectors from the real space vectors A,B,C
		static void getReciprocalVectors(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C,
			glm::vec3& Astar, glm::vec3& Bstar, glm::vec3& Cstar);

		//returns the X-ray scattering strength of a given element at a given reciprocal space distance, accounting for both atomic number and angular falloff
		static float scatteringFactor(const std::string& element, float Q);

	
};

