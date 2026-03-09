#include "XRDPoints.h"
#include "AtomData.h"
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <array>



float XRDPoints::scatteringFactor(const std::string& element, float Q) {
	static const std::unordered_map<std::string, std::array<float, 9>> cm = {
		{"O",  {3.048f,13.277f, 2.287f,5.701f, 1.546f,0.324f, 0.867f,322.913f, 0.251f}},
		{"Mg", {5.420f,2.827f,  2.174f,79.261f,1.226f,0.381f, 2.307f,7.194f,   0.858f}},
		{"Fe", {11.769f,4.761f, 7.357f,0.307f, 3.522f,15.353f,2.305f,76.881f,  1.036f}},
		{"Cu", {13.338f,3.583f, 7.168f,0.247f, 5.616f,11.397f,1.673f,64.812f,  1.191f}},
		{"Al", {6.420f,3.039f,  1.900f,0.743f, 1.594f,31.547f,1.965f,85.089f,  1.115f}},
		{"Si", {6.292f,2.439f,  3.035f,32.334f,1.989f,0.678f, 1.541f,81.694f,  1.141f}},
		{"Ni", {12.174f,4.041f, 3.422f,28.756f,2.013f,0.217f, 8.983f,66.342f,  1.103f}},
	};

	float s = Q / (4.0f * glm::pi<float>());
	float s2 = s * s;

	auto it = cm.find(element);
	if (it == cm.end()) return 6.0f;

	const auto& c = it->second;
	return c[0] * exp(-c[1] * s2) + c[2] * exp(-c[3] * s2) +
		c[4] * exp(-c[5] * s2) + c[6] * exp(-c[7] * s2) + c[8];
}



std::vector<XRDPoint> XRDPoints::genXRD(const CIFData& cifData) {
	std::vector<XRDPoint> points;

	glm::vec3 A, B, C;
	getLatticeVectors(cifData.lattice, A, B, C);

	glm::vec3 Astar, Bstar, Cstar;
	getReciprocalVectors(A, B, C, Astar, Bstar, Cstar);

	

	for (int h = -4; h <= 4; h++) {
		for (int k = -4; k <= 4; k++) {
			for (int l = -4; l <= 4; l++) {
				if (h == 0 && k == 0 && l == 0) {
					continue;
				}

				XRDPoint p;
				p.h = h; p.k = k; p.l = l;
				p.position = (float)h * Astar + (float)k * Bstar + (float)l * Cstar;
				float Q = glm::length(p.position);
				float real = 0.0f, imag = 0.0f;
				for (auto& atom : cifData.atoms) {
					std::string element = AtomData::extractElement(atom.label);
					float f = scatteringFactor(element, Q);

					float phase = 2.0f * glm::pi<float>() * (
						h * atom.fractionalPos.x +
						k * atom.fractionalPos.y +
						l * atom.fractionalPos.z
						);
					real += f * cos(phase);
					imag += f * sin(phase);
				}
				float Fsq = real * real + imag * imag;
				p.intensity = sqrt(Fsq);


				points.push_back(p);

			}
		}
	}

	// find max intensity
	float maxI = 0.0f;
	for (auto& p : points) {
		if (p.intensity > maxI) maxI = p.intensity;
	}

	// normalize
	if (maxI > 0.0f) {
		for (auto& p : points) {
			p.intensity = p.intensity / maxI;
		}
	}

	points.erase(
		std::remove_if(points.begin(), points.end(),
			[](const XRDPoint& p) { return p.intensity < 0.01f; }),
		points.end()
	);
	for (int i = 0; i < std::min((int)points.size(), 10); i++) {
		std::cout << "point " << i << " intensity=" << points[i].intensity << std::endl;
	}

	return points;
}

int XRDPoints::getXRDPointCount() {
	return xrdPointCount;
}

void XRDPoints::getLatticeVectors(const LatticeParameters& lat,
	glm::vec3& A, glm::vec3& B, glm::vec3& C)
{
	float a = lat.a, b = lat.b, c = lat.c;
	float al = glm::radians(lat.alpha);
	float be = glm::radians(lat.beta);
	float ga = glm::radians(lat.gamma);

	// a along x-axis
	A = glm::vec3(a, 0.0f, 0.0f);

	// b in xy-plane
	B = glm::vec3(b * cos(ga), b * sin(ga), 0.0f);

	// c general
	float cx = c * cos(be);
	float cy = c * (cos(al) - cos(be) * cos(ga)) / sin(ga);
	float cz = sqrt(c * c - cx * cx - cy * cy);
	C = glm::vec3(cx, cy, cz);
}

void XRDPoints::getReciprocalVectors(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C,
	glm::vec3& Astar, glm::vec3& Bstar, glm::vec3& Cstar)
{
	float V = glm::dot(A, glm::cross(B, C));

	Astar = (2.0f * glm::pi<float>() / V) * glm::cross(B, C);
	Bstar = (2.0f * glm::pi<float>() / V) * glm::cross(C, A);
	Cstar = (2.0f * glm::pi<float>() / V) * glm::cross(A, B);
}