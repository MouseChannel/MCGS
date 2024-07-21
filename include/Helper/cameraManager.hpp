#include "glm/glm.hpp"
namespace MCGS {

class CameraManager {

public:
    enum MOVE_TYPE {
        FRONT,
        BACK,
        LEFT,
        RIGHT
    };
    void Init();
    void Tick();
    void Move(MOVE_TYPE dir);
    void Rotate(float xoffset,float yoffset);
    void update();

    auto Get_p_matrix()
    {
        return m_pMatrx;
    }
    auto Get_v_matrix()
    {
        return m_vMatrix;
    }
    auto get_pos()
    {
        return  m_position;;
    }
    

private:
    glm::vec3 get_right();
    glm::vec3 get_top();
    
    glm::vec3 m_position{0,0.7f,2.4f};
    glm::vec3 m_right{1,0,0};
    glm::vec3 m_top{0,1,0};
    
    glm::vec3 m_forward{0,0,1};
    glm::mat4 m_vMatrix;
    glm::mat4 m_pMatrx;
    glm::vec2 m_mouse_pos;
    float m_pitch,m_yaw;
};
}