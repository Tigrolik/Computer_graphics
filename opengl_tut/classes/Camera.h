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

    enum Movement {forward_dir, backward_dir, left_dir, right_dir, up_dir,
        down_dir};

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
    void reverse_yaw() { yaw_ang_ = -yaw_ang_; }
    void reverse_pitch() { pitch_ang_ = -pitch_ang_; }
    void rear_view();

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

// default constructor
Camera::Camera(const glm::vec3 pos):
    cam_pos_ {pos}, world_up_ {glm::vec3{0, 1, 0}},
    cam_front_ {}, cam_right_ {}, cam_up_ {},
    pitch_ang_ {pitch_default}, yaw_ang_ {yaw_default},
    move_speed_ {speed_default}, mouse_sensitivity_ {sensitivity_default},
    zoom_ {zoom_default} {
        update_camera_vectors();
    }

// constructor with position and angles parameters
Camera::Camera(const glm::vec3 pos, const glm::vec3 up,
        const GLfloat pitch_ang, const GLfloat yaw_ang):
    cam_pos_ {pos}, world_up_ {up}, cam_front_ {}, cam_right_ {}, cam_up_ {},
    pitch_ang_ {pitch_ang}, yaw_ang_ {yaw_ang},
    move_speed_ {speed_default}, mouse_sensitivity_ {sensitivity_default},
    zoom_ {zoom_default} {
        update_camera_vectors();
    }

// get lookat matrix
glm::mat4 Camera::view_matrix() const {
    // using own version of lookat matrix impementation
    //return glm::transpose(glm::mat4 {glm::vec4{cam_right_, 0},
    //        glm::vec4{glm::cross(-cam_front_, cam_right_), 0},
    //        glm::vec4{-cam_front_, 0}, glm::vec4{0, 0, 0, 1} }) *
    //    glm::mat4 {glm::vec4{1, 0, 0, 0}, glm::vec4{0, 1, 0, 0},
    //        glm::vec4{0, 0, 1, 0}, glm::vec4{-cam_pos_, 1} };

    // using glm lookat matrix function
    return glm::lookAt(cam_pos_, cam_pos_ + cam_front_, cam_up_);
}

// process input from keyboard
void Camera::process_keyboard(const Camera::Movement dir,const GLfloat dt) {
    const GLfloat vel = move_speed_ * dt;
    switch (dir) {
        case forward_dir:
            cam_pos_ += cam_front_ * vel;
            break;
        case backward_dir:
            cam_pos_ -= cam_front_ * vel;
            break;
        case left_dir:
            cam_pos_ -= glm::normalize(glm::cross(cam_front_, cam_up_)) * vel;
            break;
        case right_dir:
            cam_pos_ += glm::normalize(glm::cross(cam_front_, cam_up_)) * vel;
            break;
        case up_dir:
            cam_pos_ += glm::normalize(glm::cross(cam_right_, cam_front_)) *
                vel;
            break;
        case down_dir:
            cam_pos_ -= glm::normalize(glm::cross(cam_right_, cam_front_)) *
                vel;
            break;
    }
}

// process movement of a mouse
void Camera::process_mouse_move(const GLfloat x_offset, const GLfloat y_offset,
            const GLboolean cut_pitch) {
    // update Euler angles
    yaw_ang_   += (x_offset * mouse_sensitivity_);
    pitch_ang_ += (y_offset * mouse_sensitivity_);
    // avoid flipping over
    if (cut_pitch) {
        if (pitch_ang_ > 89)
            pitch_ang_ = 89;
        else if (pitch_ang_ < -89)
            pitch_ang_ = -89;
    }
    update_camera_vectors();
}

// process mouse wheel scrolling
void Camera::process_scroll(const GLfloat y_offset) {
    if (zoom_ >= 1 && zoom_ <= 45)
        zoom_ -= y_offset * 0.2;
    else if (zoom_ < 1)
        zoom_ = 1;
    else
        zoom_ = 45;
}

// set the angles for the rear view
void Camera::rear_view() {
    // "turn back"
    yaw_ang_ = std::fmod(yaw_ang_ + 180, 360);
    // if the camera looks upwards, the "mirror" should show the back "bottom"
    pitch_ang_ = -pitch_ang_;
    // the other parameters should be updated
    update_camera_vectors();
}

// using Euler angles to calculate camera front, right and up vectors
void Camera::update_camera_vectors() {
    const GLfloat p_cos = cos(glm::radians(pitch_ang_));
    cam_front_ = glm::normalize(glm::vec3{cos(glm::radians(yaw_ang_)) * p_cos,
            sin(glm::radians(pitch_ang_)), sin(glm::radians(yaw_ang_)) *
            p_cos});
    cam_right_ = glm::normalize(glm::cross(cam_front_, world_up_));
    cam_up_ = glm::normalize(glm::cross(cam_right_, cam_front_));
}

#endif /* CAMERA_H */

