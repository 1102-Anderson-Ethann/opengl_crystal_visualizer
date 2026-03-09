#pragma once
#include <string>
#include <unordered_map>

class AtomData
{
public:
    static float getRadius(const std::string& element) {
        static const std::unordered_map<std::string, float> radii = {
            {"H",  0.53f}, {"C",  0.77f}, {"N",  0.75f},
            {"O",  0.73f}, {"Fe", 1.26f}, {"Cu", 1.28f},
            {"Zn", 1.22f}, {"Al", 1.43f}, {"Si", 1.11f},
            {"Ti", 1.47f}, {"Ni", 1.25f}, {"Cr", 1.29f},
            {"Mg", 1.60f}, {"Ca", 1.97f}, {"Na", 1.86f}
        };

        auto it = radii.find(element);
        if (it != radii.end()) return it->second;
        return 1.0f; // default if element not found
    }

    static glm::vec3 getColor(const std::string& element) {
        static const std::unordered_map<std::string, glm::vec3> colors = {
            {"H",  {1.0f, 1.0f, 1.0f}},
            {"C",  {0.2f, 0.2f, 0.2f}},
            {"N",  {0.2f, 0.2f, 1.0f}},
            {"O",  {1.0f, 0.1f, 0.1f}},
            {"Mg", {0.8f, 0.8f, 0.8f}},
            {"Fe", {0.8f, 0.5f, 0.2f}},
            {"Cu", {0.9f, 0.5f, 0.2f}},
            {"Zn", {0.5f, 0.6f, 0.7f}},
            {"Al", {0.0f, 1.0f, 0.0f}},
            {"Si", {0.5f, 0.4f, 0.3f}},
            {"Ti", {0.6f, 0.6f, 0.7f}},
            {"Ni", {0.4f, 0.5f, 0.5f}},
            {"Ca", {0.4f, 0.7f, 0.4f}},
            {"Na", {0.6f, 0.4f, 0.8f}}
        };

        auto it = colors.find(element);
        if (it != colors.end()) return it->second;
        return glm::vec3(0.8f, 0.8f, 0.8f); // default gray
    }



    static std::string extractElement(const std::string& label) {
        if (label.empty()) return label;

        std::string element;
        element += label[0]; // first char always uppercase

        // add second char only if it's lowercase
        if (label.size() > 1 && std::islower(label[1])) {
            element += label[1];
        }

        return element;
    }

private:

};
