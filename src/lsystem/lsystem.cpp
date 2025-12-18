#include "lsystem/lsystem.h"

#include <cmath>
#include <random>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace {
constexpr float PI = 3.14159265358979323846f;

std::mt19937& getRNG() {
    static thread_local std::mt19937 rng(std::random_device{}());
    return rng;
}

float randomFloat() {
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(getRNG());
}
} // namespace

void LSystem::setAxiom(const std::string& axiom) {
    m_axiom = axiom;
}

void LSystem::addRule(char predecessor, const std::string& successor, float probability) {
    LSystemRule rule;
    rule.successor = successor;
    rule.probability = probability;
    m_rules[predecessor].push_back(rule);
}

void LSystem::clearRules() {
    m_rules.clear();
}

std::string LSystem::generate(int iterations) {
    std::string current = m_axiom;

    for (int i = 0; i < iterations; ++i) {
        std::string next;
        next.reserve(current.size() * 2);

        for (char c : current) {
            auto it = m_rules.find(c);
            if (it != m_rules.end() && !it->second.empty()) {
                const auto& rules = it->second;
                if (rules.size() == 1) {
                    next += rules[0].successor;
                } else {
                    float r = randomFloat();
                    float cumulative = 0.0f;
                    for (const auto& rule : rules) {
                        cumulative += rule.probability;
                        if (r <= cumulative) {
                            next += rule.successor;
                            break;
                        }
                    }
                }
            } else {
                next += c;
            }
        }

        current = std::move(next);
    }

    return current;
}

void LSystem::rotateYaw(TurtleState& state, float angleDeg) {
    float rad = glm::radians(angleDeg);
    glm::mat3 rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), rad, state.up));
    state.heading = rotation * state.heading;
    state.left = rotation * state.left;
}

void LSystem::rotatePitch(TurtleState& state, float angleDeg) {
    float rad = glm::radians(angleDeg);
    glm::mat3 rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), rad, state.left));
    state.heading = rotation * state.heading;
    state.up = rotation * state.up;
}

void LSystem::rotateRoll(TurtleState& state, float angleDeg) {
    float rad = glm::radians(angleDeg);
    glm::mat3 rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), rad, state.heading));
    state.left = rotation * state.left;
    state.up = rotation * state.up;
}

void LSystem::interpret(const std::string& lstring) {
    m_branches.clear();
    m_leaves.clear();

    TurtleState state;
    state.width = m_initialWidth;
    state.depth = 0;

    std::stack<TurtleState> stateStack;
    float currentLength = m_stepLength;

    for (char c : lstring) {
        switch (c) {
            case 'F': {
                glm::vec3 newPos = state.position + state.heading * currentLength;
                float newWidth = state.width * m_widthDecay;

                Branch branch;
                branch.start = state.position;
                branch.end = newPos;
                branch.startRadius = state.width;
                branch.endRadius = newWidth;
                branch.depth = state.depth;
                m_branches.push_back(branch);

                state.position = newPos;
                state.width = newWidth;
                break;
            }
            case 'f': {
                state.position += state.heading * currentLength;
                break;
            }
            case '+': {
                rotateYaw(state, m_angle);
                break;
            }
            case '-': {
                rotateYaw(state, -m_angle);
                break;
            }
            case '^': {
                rotatePitch(state, m_angle);
                break;
            }
            case '&': {
                rotatePitch(state, -m_angle);
                break;
            }
            case '\\': {
                rotateRoll(state, m_angle);
                break;
            }
            case '/': {
                rotateRoll(state, -m_angle);
                break;
            }
            case '|': {
                rotateYaw(state, 180.0f);
                break;
            }
            case '[': {
                stateStack.push(state);
                state.depth++;
                break;
            }
            case ']': {
                Leaf leaf;
                leaf.position = state.position;
                leaf.direction = state.heading;
                leaf.size = state.width * 5.0f;
                m_leaves.push_back(leaf);

                if (!stateStack.empty()) {
                    state = stateStack.top();
                    stateStack.pop();
                }
                break;
            }
            case '!': {
                state.width *= m_widthDecay;
                break;
            }
            case 'L': {
                Leaf leaf;
                leaf.position = state.position;
                leaf.direction = state.heading;
                leaf.size = state.width * 5.0f;
                m_leaves.push_back(leaf);
                break;
            }
            default:
                break;
        }
    }
}

void LSystem::generateCylinder(const Branch& branch, int segments, std::vector<float>& vertices) const {
    glm::vec3 direction = branch.end - branch.start;
    float length = glm::length(direction);
    if (length < 1e-6f) return;

    direction = glm::normalize(direction);

    glm::vec3 up = std::abs(direction.y) < 0.999f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 right = glm::normalize(glm::cross(up, direction));
    up = glm::cross(direction, right);

    for (int i = 0; i < segments; ++i) {
        float angle0 = (2.0f * PI * i) / segments;
        float angle1 = (2.0f * PI * (i + 1)) / segments;

        float cos0 = std::cos(angle0);
        float sin0 = std::sin(angle0);
        float cos1 = std::cos(angle1);
        float sin1 = std::sin(angle1);

        glm::vec3 b0 = branch.start + (right * cos0 + up * sin0) * branch.startRadius;
        glm::vec3 b1 = branch.start + (right * cos1 + up * sin1) * branch.startRadius;

        glm::vec3 t0 = branch.end + (right * cos0 + up * sin0) * branch.endRadius;
        glm::vec3 t1 = branch.end + (right * cos1 + up * sin1) * branch.endRadius;

        glm::vec3 n0 = glm::normalize(right * cos0 + up * sin0);
        glm::vec3 n1 = glm::normalize(right * cos1 + up * sin1);

        vertices.push_back(b0.x); vertices.push_back(b0.y); vertices.push_back(b0.z);
        vertices.push_back(n0.x); vertices.push_back(n0.y); vertices.push_back(n0.z);

        vertices.push_back(t0.x); vertices.push_back(t0.y); vertices.push_back(t0.z);
        vertices.push_back(n0.x); vertices.push_back(n0.y); vertices.push_back(n0.z);

        vertices.push_back(t1.x); vertices.push_back(t1.y); vertices.push_back(t1.z);
        vertices.push_back(n1.x); vertices.push_back(n1.y); vertices.push_back(n1.z);

        vertices.push_back(b0.x); vertices.push_back(b0.y); vertices.push_back(b0.z);
        vertices.push_back(n0.x); vertices.push_back(n0.y); vertices.push_back(n0.z);

        vertices.push_back(t1.x); vertices.push_back(t1.y); vertices.push_back(t1.z);
        vertices.push_back(n1.x); vertices.push_back(n1.y); vertices.push_back(n1.z);

        vertices.push_back(b1.x); vertices.push_back(b1.y); vertices.push_back(b1.z);
        vertices.push_back(n1.x); vertices.push_back(n1.y); vertices.push_back(n1.z);
    }
}

std::vector<float> LSystem::generateBranchMesh(int radialSegments) const {
    std::vector<float> vertices;
    vertices.reserve(m_branches.size() * radialSegments * 6 * 6);

    for (const Branch& branch : m_branches) {
        generateCylinder(branch, radialSegments, vertices);
    }

    return vertices;
}

std::vector<float> LSystem::generateLeafMesh() const {
    std::vector<float> vertices;
    vertices.reserve(m_leaves.size() * 6 * 6);

    for (const Leaf& leaf : m_leaves) {
        glm::vec3 up = std::abs(leaf.direction.y) < 0.999f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
        glm::vec3 right = glm::normalize(glm::cross(up, leaf.direction));
        up = glm::normalize(glm::cross(leaf.direction, right));

        glm::vec3 normal = leaf.direction;
        float halfSize = leaf.size * 0.5f;

        glm::vec3 v0 = leaf.position - right * halfSize - up * halfSize;
        glm::vec3 v1 = leaf.position + right * halfSize - up * halfSize;
        glm::vec3 v2 = leaf.position + right * halfSize + up * halfSize;
        glm::vec3 v3 = leaf.position - right * halfSize + up * halfSize;

        vertices.push_back(v0.x); vertices.push_back(v0.y); vertices.push_back(v0.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);

        vertices.push_back(v1.x); vertices.push_back(v1.y); vertices.push_back(v1.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);

        vertices.push_back(v2.x); vertices.push_back(v2.y); vertices.push_back(v2.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);

        vertices.push_back(v0.x); vertices.push_back(v0.y); vertices.push_back(v0.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);

        vertices.push_back(v2.x); vertices.push_back(v2.y); vertices.push_back(v2.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);

        vertices.push_back(v3.x); vertices.push_back(v3.y); vertices.push_back(v3.z);
        vertices.push_back(normal.x); vertices.push_back(normal.y); vertices.push_back(normal.z);
    }

    return vertices;
}

LSystem LSystem::createSimplePlant() {
    LSystem lsystem;
    lsystem.setAxiom("F");
    lsystem.addRule('F', "F[+F]F[-F]F");
    lsystem.setAngle(25.7f);
    lsystem.setStepLength(0.5f);
    lsystem.setInitialWidth(0.05f);
    lsystem.setWidthDecay(0.7f);
    return lsystem;
}

LSystem LSystem::createTree() {
    LSystem lsystem;
    lsystem.setAxiom("X");
    lsystem.addRule('X', "F[&+X][&-X][&X]");
    lsystem.addRule('F', "FF");
    lsystem.setAngle(25.0f);
    lsystem.setStepLength(0.3f);
    lsystem.setInitialWidth(0.08f);
    lsystem.setWidthDecay(0.65f);
    return lsystem;
}

LSystem LSystem::createBush() {
    LSystem lsystem;
    lsystem.setAxiom("F");
    lsystem.addRule('F', "FF+[+F-F-F]-[-F+F+F]");
    lsystem.setAngle(22.5f);
    lsystem.setStepLength(0.4f);
    lsystem.setInitialWidth(0.04f);
    lsystem.setWidthDecay(0.75f);
    return lsystem;
}

LSystem LSystem::createFern() {
    LSystem lsystem;
    lsystem.setAxiom("X");
    lsystem.addRule('X', "F+[[X]-X]-F[-FX]+X");
    lsystem.addRule('F', "FF");
    lsystem.setAngle(25.0f);
    lsystem.setStepLength(0.35f);
    lsystem.setInitialWidth(0.02f);
    lsystem.setWidthDecay(0.8f);
    return lsystem;
}
