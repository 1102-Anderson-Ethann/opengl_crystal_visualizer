#include "XRDPoints.h"
#include <cmath>


std::vector<XRDPoint> XRDPoints::genFakeXRD(float alpha) {
	std::vector<XRDPoint> points;


	for (int h = -4; h <= 4; h++) {
		for (int k = -4; k <= 4; k++) {
			for (int l = -4; l <= 4; l++) {
				if (h == 0 && k == 0 && l == 0) {
					continue;
				}
				if ((h + k + l) % 2 != 0) {
					continue;
				}

				XRDPoint p;
				float Qx = (2.0f * glm::pi<float>() / alpha) * h;
				float Qy = (2.0f * glm::pi<float>() / alpha) * k;
				float Qz = (2.0f * glm::pi<float>() / alpha) * l;

				p.h = h;
				p.k = k;
				p.l = l;
				p.position = glm::vec3(Qx, Qy, Qz);
				float Q = glm::length(p.position);
				p.intensity = exp(-0.02f * Q * Q);

				points.push_back(p);

			}
		}
	}

	return points;
}

int XRDPoints::getXRDPointCount() {
	return xrdPointCount;
}