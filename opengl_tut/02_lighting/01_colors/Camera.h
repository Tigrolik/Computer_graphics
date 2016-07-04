#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Camera {
public:
    // default values
    static constexpr GLfloat yaw_default         = -90;
    static constexpr GLfloat pitch_default       = 0;
    static constexpr GLfloat speed_default       = 3;
    static constexpr GLfloat sensitivity_default = 0.25;
    static constexpr GLfloat zoom_default        = 45;

    enum Movement {forward_dir, backward_dir, left_dir, right_dir};

    explicit Camera(const glm::vec3 = {0, 0, 0});
    explicit Camera(const glm::vec3, const glm::vec3, const GLfloat,
            const GLfloat);

    explicit Camera(const Camera&) = default;
    Camera& operator=(const Camera&) = default;

    explicit Camera(Camera&&) = default;
    Camera& operator=(Camera&&) = default;

    ~Camera() = default;

    const glm::vec3 pos()      const { return cam_pos_; }
    const glm::vec3 world_up() const { return world_up_; }
    const glm::vec3 front()    const { return cam_front_; }
    const glm::vec3 right()    const { return cam_right_; }
    const glm::vec3 up()       const { return cam_up_; }

    GLfloat pitch()       const { return pitch_ang_; }
    GLfloat yaw()         const { return yaw_ang_; }
    GLfloat move_speed()  const { return move_speed_; }
    GLfloat sensitivity() const { return mouse_sensitivity_; }
    GLfloat zoom()        const { return zoom_; }

    glm::mat4 view_matrix() const;

    void process_keyboard(const Movement, const GLfloat);
    void process_mouse_move(const GLfloat, const GLfloat,
            const GLboolean = true);
    void process_scroll(const GLfloat);

private:
    // attributes
    glm::vec3 cam_pos_;
    glm::vec3 world_up_;
    glm::vec3 cam_front_;
    glm::vec3 cam_right_;
    glm::vec3 cam_up_;
    // Euler angles
    GLfloat pitch_ang_;
    GLfloat yaw_ang_;
    // options
    GLfloat move_speed_;
    GLfloat mouse_sensitivity_;
    GLfloat zoom_;
    // calculate camera vectors
    void update_camera_vectors();
};

#endif /* CAMERA_H */

