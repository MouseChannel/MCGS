#include "Helper/cameraManager.hpp"
#include "Rendering/AppWindow.hpp"
#include "Rendering/Context.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
namespace MCGS {

void CameraManager::Init()
{
    glfwSetWindowUserPointer(MCRT::Context::Get_Singleton()->get_window()->get_handle(), this);
    glfwSetCursorPosCallback(MCRT::Context::Get_Singleton()->get_window()->get_handle(),
                             [](GLFWwindow* window, double xpos, double ypos) {
                                 auto camera = (CameraManager*)glfwGetWindowUserPointer(window);
                                 camera->Rotate(xpos, ypos);
                             });
    m_pMatrx = glm::perspective(glm::radians(60.f), 1.f, 0.01f, 100.f);
    update();
}
void CameraManager::Tick()
{
    auto dir = CameraManager::MOVE_TYPE::FRONT;
    if (glfwGetKey(MCRT::Context::Get_Singleton()->get_window()->get_handle(), GLFW_KEY_W) == GLFW_PRESS) {
        dir = CameraManager::MOVE_TYPE::FRONT;
    } else if (glfwGetKey(MCRT::Context::Get_Singleton()->get_window()->get_handle(), GLFW_KEY_S) == GLFW_PRESS) {
        dir = CameraManager::MOVE_TYPE::BACK;
    } else if (glfwGetKey(MCRT::Context::Get_Singleton()->get_window()->get_handle(), GLFW_KEY_A) == GLFW_PRESS) {
        dir = CameraManager::MOVE_TYPE::LEFT;

    } else if (glfwGetKey(MCRT::Context::Get_Singleton()->get_window()->get_handle(), GLFW_KEY_D) == GLFW_PRESS) {

        dir = CameraManager::MOVE_TYPE::RIGHT;
    }
    Move(dir);
}

glm::vec3 CameraManager::get_right()
{
    m_right =
        glm::cross(m_forward, glm::vec3 { 0, 1, 0 });
    return m_right;
}
glm::vec3 CameraManager::get_top()
{
    m_top = glm::cross(m_right, m_forward);
    return m_top;
}

void CameraManager::Move(MOVE_TYPE dir)
{

    switch (dir) {

    case FRONT:
        m_position += m_forward * 0.1f;
        break;
    case BACK:
        m_position += m_forward * -0.1f;
        break;
    case LEFT:
        m_position += m_right * -0.1f;
        break;
    case RIGHT:
        m_position += m_right * 0.1f;
        break;
    }
    update();
}
void CameraManager::update()
{

    m_right =
        glm::cross(m_forward, glm::vec3 { 0, 1, 0 });
    m_top = glm::cross(m_right, m_forward);
    m_vMatrix = glm::lookAt(m_position, m_position + m_forward, glm::vec3 { 0, 1, 0 });
}
void CameraManager::Rotate(float xoffset, float yoffset)
{

    if (glfwGetMouseButton(MCRT::Context::Get_Singleton()->get_window()->get_handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        std::cout << xoffset << ' ' << yoffset << std::endl;
        m_yaw += xoffset * 0.1f;
        m_pitch += yoffset * 0.1f;
        m_pitch = std::clamp(m_pitch, -89.f, 89.f);
        m_forward.y = sin(glm::radians(m_pitch));
        m_forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

        m_forward = glm::normalize(m_forward);
    }
    Tick();
}

}