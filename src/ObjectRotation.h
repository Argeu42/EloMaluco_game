#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ObjectRotation {
public:
    ObjectRotation(const glm::vec3& bottomCenter = glm::vec3(0.5f, 0.0f, 0.5f)) 
        : m_rotation(1.0f, 0.0f, 0.0f, 0.0f), m_bottomCenter(bottomCenter) {}

    void rotate(float deltaX, float deltaY) {
        float sensitivity = 0.005f;
        
        glm::quat yawQuat = glm::angleAxis(deltaX * sensitivity, glm::vec3(0.0f, 1.0f, 0.0f));
        
        m_rotation = yawQuat * m_rotation;
    }

    void rotateY(float angle) {
        glm::quat yawQuat = glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        m_rotation = yawQuat * m_rotation;
    }

    glm::mat4 getRotationMatrix() const {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_bottomCenter);
        glm::mat4 rotation = glm::mat4_cast(m_rotation);
        glm::mat4 translationBack = glm::translate(glm::mat4(1.0f), m_bottomCenter);
        
        return translationBack * rotation * translation;
    }

private:
    glm::quat m_rotation;
    glm::vec3 m_bottomCenter;
};

