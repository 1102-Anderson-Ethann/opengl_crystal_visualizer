#pragma once

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
		static std::vector<XRDPoint> genFakeXRD(float a);

		//Later XRD data loader goes here
	private:
		int xrdPointCount;


};

