#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include <iostream>

Entity::Entity()
{
    m_position     = glm::vec3(0);
    m_model_matrix = glm::mat4(1.0f);
}

Entity::~Entity()
{
    ;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float) (index % 6) / (float) 6;
    float v_coord = (float) (index / 6) / (float) 1;
    
    // Step 2: Calculate its UV size
    float width = 1.0f / (float) 6;
    float height = 1.0f / (float) 1;
    
    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };
    
    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };
    
    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::update(float delta_time)
{
    if (m_accelerating) {
        m_velocity += glm::vec3(-SHIP_ACCELERATION * sin(glm::radians(m_ship_angle)), SHIP_ACCELERATION * cos(glm::radians(m_ship_angle)), 0.0f);
    }
    m_velocity += GRAVITY_ACCELERATION;
    if (m_velocity.y < MAX_GRAVITY_VELOCITY) {
        m_velocity.y = MAX_GRAVITY_VELOCITY;
    }
    if (m_velocity.y > -MAX_GRAVITY_VELOCITY) {
        m_velocity.y = -MAX_GRAVITY_VELOCITY;
    }
    if (m_velocity.x > MAX_HORIZONTAL_VELOCITY) {
        m_velocity.x = MAX_HORIZONTAL_VELOCITY;
    }
    else if (m_velocity.x < -MAX_HORIZONTAL_VELOCITY) {
        m_velocity.x = -MAX_HORIZONTAL_VELOCITY;
    }
    m_position += m_velocity * SHIP_SPEED * delta_time;

    //std::cout << m_velocity.y << std::endl;

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::rotate(m_model_matrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_model_matrix = glm::rotate(m_model_matrix, glm::radians(m_ship_angle), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Entity::render(ShaderProgram *program)
{
    program->set_model_matrix(m_model_matrix);
    
    if (m_accelerating)
    {
        draw_sprite_from_texture_atlas(program, m_moving_texture_id, m_animation_index);
        m_animation_index += 1;
        return;
    }

    
    float vertices[]   = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = {  0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, m_idle_texture_id);
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}
