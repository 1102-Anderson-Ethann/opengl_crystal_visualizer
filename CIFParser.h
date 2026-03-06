#pragma once
#include <string>
#include <vector>
#include<glm/glm.hpp>

struct LatticeParameters
{
	float a, b, c;
	float alpha, beta, gamma;
};

struct AtomSite
{
	std::string label;
	glm::vec3 fractionalPos;
};

struct SymOp {
	glm::mat3 rotation;
	glm::vec3 translation;
};

struct CIFData
{
	LatticeParameters lattice;
	std::vector<AtomSite> atoms;
	std::vector<SymOp> symOps;
};

class CIFParser
{
public:
	static CIFData parse(const std::string& filename);
	static SymOp parseSymOp(const std::string& opString);
	static void applySymmetry(CIFData& data);
private:

};
