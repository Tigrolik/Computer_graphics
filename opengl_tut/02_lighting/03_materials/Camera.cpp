#include "Camera.h"

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
    return glm::transpose(glm::mat4 {glm::vec4{cam_right_, 0},
            glm::vec4{glm::cross(-cam_front_, cam_right_), 0},
            glm::vec4{-cam_front_, 0}, glm::vec4{0, 0, 0, 1} }) *
        glm::mat4 {glm::vec4{1, 0, 0, 0}, glm::vec4{0, 1, 0, 0},
            glm::vec4{0, 0, 1, 0}, glm::vec4{-cam_pos_, 1} };

    // using glm lookat matrix function
    //return glm::lookAt(cam_pos_, cam_pos_ + cam_front_, cam_up_);
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
            cam_pos_.x -= cam_right_.x * vel;
            break;
        case right_dir:
            cam_pos_.x += cam_right_.x * vel;
            break;
        case up_dir:
            cam_pos_.y += cam_right_.x * vel;
            break;
        case down_dir:
            cam_pos_.y -= cam_right_.x * vel;
            break;
    }
}

// process movement of a mouse
void Camera::process_mouse_move(const GLfloat x_offset, const GLfloat y_offset,
            const GLboolean cut_pitch) {
    // update Euler angles
    yaw_ang_   += x_offset * mouse_sensitivity_;
    pitch_ang_ += y_offset * mouse_sensitivity_;
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

// using Euler angles to calculate camera front, right and up vectors
void Camera::update_camera_vectors() {
    const GLfloat p_cos = cos(glm::radians(pitch_ang_));
    cam_front_ = glm::normalize(glm::vec3{cos(glm::radians(yaw_ang_)) * p_cos,
            sin(glm::radians(pitch_ang_)), sin(glm::radians(yaw_ang_))*p_cos});
    cam_right_ = glm::normalize(glm::cross(cam_front_, world_up_));
    cam_up_ = glm::normalize(glm::cross(cam_right_, cam_front_));
}

