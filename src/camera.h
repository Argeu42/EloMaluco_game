#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Camera {
public:
    Camera(const glm::vec3& position = glm::vec3(5.0f, 5.0f, 5.0f),
           const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f),
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f))
        : m_position(position), m_target(target), m_up(up) {
        updateOrientation();
    }

    void rotate(float angle, const glm::vec3& axis) {
        glm::quat rotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
        m_orientation = rotation * m_orientation;
        updateVectors();
    }

    void move(const glm::vec3& offset) {
        m_position += offset;
        m_target += offset;
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(m_position, m_target, m_up);
    }

    const glm::vec3& getPosition() const { return m_position; }
    const glm::vec3& getTarget() const { return m_target; }
    const glm::vec3& getUp() const { return m_up; }

private:
    glm::vec3 m_position;
    glm::vec3 m_target;
    glm::vec3 m_up;
    glm::quat m_orientation;

    void updateOrientation() {
        glm::vec3 direction = glm::normalize(m_target - m_position);
        glm::vec3 right = glm::normalize(glm::cross(direction, m_up));
        glm::vec3 up = glm::cross(right, direction);

        glm::mat3 rotationMatrix(right, up, -direction);
        m_orientation = glm::quat_cast(rotationMatrix);
    }

    void updateVectors() {
        glm::mat4 rotationMatrix = glm::mat4_cast(m_orientation);
        glm::vec3 forward = -glm::vec3(rotationMatrix[2]);
        m_target = m_position + forward;
        m_up = glm::vec3(rotationMatrix[1]);
    }
};