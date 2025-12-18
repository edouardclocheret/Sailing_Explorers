#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include <functional>

#include <glm/glm.hpp>

struct Branch {
    glm::vec3 start;
    glm::vec3 end;
    float startRadius;
    float endRadius;
    int depth;
};

struct Leaf {
    glm::vec3 position;
    glm::vec3 direction;
    float size;
};

struct LSystemRule {
    std::string successor;
    float probability = 1.0f;
};

struct TurtleState {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 heading = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 left = glm::vec3(-1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
    float width = 1.0f;
    int depth = 0;
};

class LSystem {
public:
    LSystem() = default;

    void setAxiom(const std::string& axiom);
    void addRule(char predecessor, const std::string& successor, float probability = 1.0f);
    void clearRules();
    std::string generate(int iterations);

    void setAngle(float angleDegrees) { m_angle = angleDegrees; }
    void setStepLength(float length) { m_stepLength = length; }
    void setInitialWidth(float width) { m_initialWidth = width; }
    void setWidthDecay(float decay) { m_widthDecay = decay; }
    void setLengthDecay(float decay) { m_lengthDecay = decay; }

    void interpret(const std::string& lstring);

    const std::vector<Branch>& getBranches() const { return m_branches; }
    const std::vector<Leaf>& getLeaves() const { return m_leaves; }

    std::vector<float> generateBranchMesh(int radialSegments = 6) const;
    std::vector<float> generateLeafMesh() const;

    static LSystem createSimplePlant();
    static LSystem createTree();
    static LSystem createBush();
    static LSystem createFern();

private:
    std::string m_axiom;
    std::unordered_map<char, std::vector<LSystemRule>> m_rules;

    float m_angle = 25.0f;
    float m_stepLength = 1.0f;
    float m_initialWidth = 0.1f;
    float m_widthDecay = 0.7f;
    float m_lengthDecay = 0.9f;

    std::vector<Branch> m_branches;
    std::vector<Leaf> m_leaves;

    void rotateYaw(TurtleState& state, float angleDeg);
    void rotatePitch(TurtleState& state, float angleDeg);
    void rotateRoll(TurtleState& state, float angleDeg);
    void generateCylinder(const Branch& branch, int segments, std::vector<float>& vertices) const;
};
