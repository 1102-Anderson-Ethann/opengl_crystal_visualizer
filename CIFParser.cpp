#include "CIFParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

CIFData CIFParser::parse(const std::string& filename) {
	CIFData data;
	bool inSymOpBlock = false;
	bool inAtomSiteBlock = false;
	int atomHeaderCount = 0;
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Could not open CIF file: " << filename << std::endl;
		return data;
	 }

	int labelCol = -1;
	int xCol = -1;
	int yCol = -1;
	int zCol = -1;
	int colIndex = 0;

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "loop_") {
			inSymOpBlock = false;
			inAtomSiteBlock = false;
			atomHeaderCount = 0;
			colIndex = 0;
		}

		if (token == "_cell_length_a")  iss >> data.lattice.a;
		if (token == "_cell_length_b")  iss >> data.lattice.b;
		if (token == "_cell_length_c")  iss >> data.lattice.c;
		if (token == "_cell_angle_alpha") iss >> data.lattice.alpha;
		if (token == "_cell_angle_beta")  iss >> data.lattice.beta;
		if (token == "_cell_angle_gamma") iss >> data.lattice.gamma;

		if (token == "_space_group_symop_operation_xyz" ||
			token == "_symmetry_equiv_pos_as_xyz") {
			inSymOpBlock = true;
		}

		if (inSymOpBlock && !token.empty() &&
			token[0] != '_' && token != "loop_" &&
			(token.find(',') != std::string::npos)) {
			data.symOps.push_back(parseSymOp(token));
		}

		if (token == "_atom_site_label") {
			inAtomSiteBlock = true;
			labelCol = colIndex;
			colIndex++;
			std::cout << "Found atom site block, labelCol = " << labelCol << std::endl;
		}
		else if (token == "_atom_site_fract_x") { xCol = colIndex; colIndex++; }
		else if (token == "_atom_site_fract_y") { yCol = colIndex; colIndex++; }
		else if (token == "_atom_site_fract_z") { zCol = colIndex; colIndex++; }
		else if (token == "loop_" && inAtomSiteBlock) { inAtomSiteBlock = false; }
		else if (token.find("_atom_site_") == 0) { colIndex++; }

		if (inAtomSiteBlock && xCol != -1 &&
			!token.empty() && token[0] != '_' && token != "loop_") {

			// Read all tokens on this line into a vector
			std::vector<std::string> tokens;
			tokens.push_back(token); // first token already consumed
			std::string t;
			while (iss >> t) {
				tokens.push_back(t);
			}

			// Make sure we have enough columns
			if ((int)tokens.size() > zCol) {
				AtomSite atom;
				atom.label = tokens[labelCol];
				atom.fractionalPos.x = std::stof(tokens[xCol]);
				atom.fractionalPos.y = std::stof(tokens[yCol]);
				atom.fractionalPos.z = std::stof(tokens[zCol]);
				std::cout << atom.label << " " << atom.fractionalPos.x << " " << atom.fractionalPos.y << " " << atom.fractionalPos.z << std::endl;
				data.atoms.push_back(atom);
			}


		}


	}
	applySymmetry(data);
	return data;
}

SymOp CIFParser::parseSymOp(const std::string& opString) {
	SymOp op;
	op.rotation = glm::mat3(0.0f);
	op.translation = glm::vec3(0.0f);

	std::istringstream iss(opString);
	std::string component;
	int row = 0;

	while (std::getline(iss, component, ',') && row < 3) {
		// parse each component here

		float sign = 1.0f;
		float numerator = 0.0f;
		float denominator = 1.0f;
		bool readingNumerator = false;
		bool readingDenominator = false;

		for (char c : component) {
			//Handle signs
			if (c == '+')
			{
				sign = 1.0f;
			}
			else if (c == '-') {
				sign = -1.0f;
			}
			//handle digits
			else if (c == '/')
			{
				readingDenominator = true;
				readingNumerator = false;
			}
			else if (std::isdigit(c))
			{
				if (!readingDenominator)
				{
					numerator = c - '0';
					readingNumerator = true;
				}
				else {
					denominator = c - '0';
				}
			}
			//handle x,y,z
			else if (c == 'x')
			{
				op.rotation[0][row] = sign;
				sign = 1.0f;
			}
			else if (c == 'y')
			{
				op.rotation[1][row] = sign;
				sign = 1.0f;
			}
			else if (c == 'z')
			{
				op.rotation[2][row] = sign;
				sign = 1.0f;
			}

		}

		if (numerator != 0.0f) {
			op.translation[row] = numerator / denominator;
		}

		row++;

		
	}

	return op;
}

void CIFParser::applySymmetry(CIFData& data) {
	std::vector<AtomSite> expanded;

	for (auto& atom : data.atoms) {
		for (auto& op : data.symOps) {
			AtomSite newAtom;
			newAtom.label = atom.label;

			// apply rotation and translation
			glm::vec3 newPos = op.rotation * atom.fractionalPos + op.translation;

			// wrap into unit cell
			newPos.x = newPos.x - std::floor(newPos.x);
			newPos.y = newPos.y - std::floor(newPos.y);
			newPos.z = newPos.z - std::floor(newPos.z);

			// check if this position already exists
			bool duplicate = false;
			for (auto& existing : expanded) {
				if (glm::length(existing.fractionalPos - newPos) < 0.001f) {
					duplicate = true;
					break;
				}
			}

			if (!duplicate) {
				newAtom.fractionalPos = newPos;
				expanded.push_back(newAtom);
			}
		}

		
	}

	data.atoms = expanded;

	for (auto& atom : data.atoms) {
		atom.fractionalPos.x = atom.fractionalPos.x - std::floor(atom.fractionalPos.x);
		atom.fractionalPos.y = atom.fractionalPos.y - std::floor(atom.fractionalPos.y);
		atom.fractionalPos.z = atom.fractionalPos.z - std::floor(atom.fractionalPos.z);
	}



}