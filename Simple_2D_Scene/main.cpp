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
#include "stb_image.h"

enum Coordinate
{
    x_coordinate,
    y_coordinate
};

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char OMORI_SPRITE_FILEPATH[] = "assets/sunny.png";     //1200 x 1200
const char BOX_SPRITE_FILEPATH[] = "assets/white_space.png"; //191 x 128
const char CAT_SPRITE_FILEPATH[] = "assets/meow.png";        //623 x 400
const char HAND_SPRITE_FILEPATH[] = "assets/hand.png";       //136 x 369

GLuint g_omori_texture_id;
GLuint g_box_texture_id;
GLuint g_cat_texture_id;
GLuint g_hand_texture_id;

SDL_Window* g_display_window;
bool g_game_is_running = true;
bool g_is_growing = true;

ShaderProgram g_shader_program;
glm::mat4 view_matrix, g_projection_matrix;
glm::mat4 g_omori_model_matrix, g_box_model_matrix, g_cat_model_matrix, g_hand_model_matrix, g_hand2_model_matrix;

float g_previous_ticks = 0.0f;

float angle = 0.0f;
float blackout_timer = 0.0f;
bool blackout = false;

glm::vec3 g_cat_pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_hand_pos = glm::vec3(0.0f, 0.0f, 0.0f);

bool black_out_before = false;

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO);

    g_display_window = SDL_CreateWindow("Omori 2D Scene",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    #ifdef _WINDOWS
        glewInit();
    #endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_omori_model_matrix = glm::mat4(1.0f);
    g_box_model_matrix = glm::mat4(1.0f);
    g_cat_model_matrix = glm::mat4(1.0f);
    g_hand_model_matrix = glm::mat4(1.0f);
    g_hand2_model_matrix = glm::mat4(1.0f);
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);
    // Notice we haven't set our model matrix yet!

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(255.0f, 255.0f, 255.0f, 1.0f);

    g_omori_texture_id = load_texture(OMORI_SPRITE_FILEPATH);
    g_box_texture_id = load_texture(BOX_SPRITE_FILEPATH);
    g_cat_texture_id = load_texture(CAT_SPRITE_FILEPATH);
    g_hand_texture_id = load_texture(HAND_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_WINDOWEVENT_CLOSE:
        case SDL_QUIT:
            g_game_is_running = false;
            break;
        default:
            break;
        }
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    angle += delta_time;
    //if (angle >= 6.3f) angle = angle - 6.2830f;

    blackout_timer += delta_time;
    float BLACKOUT_TIME = 3.0f;
    float NON_BLACKOUT_TIME = 5.0f;

    if (blackout) {
        if (blackout_timer >= BLACKOUT_TIME) {
            blackout = false;
            glClearColor(255.0f, 255.0f, 255.0f, 1.0f);
            blackout_timer = 0.0f;
        }
    }
    else {
        if (blackout_timer >= NON_BLACKOUT_TIME) {
            blackout = true;
            glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
            blackout_timer = 0.0f;
            black_out_before = true;
        }
    }

    float radius = 3.0f;
    g_cat_pos.x = sin(angle) * radius;
    g_cat_pos.y = cos(angle) * radius;

    g_cat_model_matrix = glm::mat4(1.0f);
    g_cat_model_matrix = glm::translate(g_cat_model_matrix, g_cat_pos);
    

    //g_omori_model_matrix = glm::mat4(1.0f);
    //glm::vec3 omori_position = glm::vec3(0.0f, 0.0f, 0.0f);
    /*omori_position = glm::vec3(0.0f, 0.0f, 0.0f);;
    omori_position.x = -1.0f;
    omori_position.y = -1.0f;
    g_omori_model_matrix = glm::translate(g_omori_model_matrix, omori_position);*/

    if (black_out_before) {
        g_omori_model_matrix = glm::rotate(g_omori_model_matrix, 2.0f * delta_time, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    g_box_model_matrix = glm::mat4(1.0f);
    glm::vec3 box_position = glm::vec3(0.0f, 0.0f, 0.0f);
    //box_position.x = -4.0f;
    box_position.y = -0.5f;
    g_box_model_matrix = glm::translate(g_box_model_matrix, box_position);
    glm::vec3 scale_vector = glm::vec3(0.90f, 0.90f, 1.0f);
    g_box_model_matrix = glm::scale(g_box_model_matrix, scale_vector);


    //VIEWPORT_WIDTH = WINDOW_WIDTH,
    //VIEWPORT_HEIGHT = WINDOW_HEIGHT;
    float LEFT_MAX = -4.0f;
    float RIGHT_MAX = 4.0f;
    float VERT_DISP = 2.0f;
    g_hand_pos.x = RIGHT_MAX - (RIGHT_MAX - LEFT_MAX) * blackout_timer / 3.0f;
    g_hand_pos.y = sin(g_hand_pos.x) * VERT_DISP;

    g_hand_model_matrix = glm::mat4(1.0f);
    g_hand_model_matrix = glm::translate(g_hand_model_matrix, g_hand_pos);

    glm::vec3 hand2_relative_pos = glm::vec3(1.0f, -1.0f, 0.0f);

    g_hand2_model_matrix = glm::translate(g_hand_model_matrix, hand2_relative_pos);

    //item_model_matrix = glm::translate(character_model, glm::vec3(TRANS_VALUE, TRANS_VALUE, 0.0f));
    //item_model_matrix = glm::rotate(character_model_matrix, ANGLE, glm::vec3(0.0f, 0.0f, 1.0f));
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}



void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    if (!blackout){
    int SCALE = 30;
    float model_width = 191.0f / SCALE; // Width of the box
    float model_height = 128.0f / SCALE; // Height of the box

    float box_vertices[] = {
        -model_width / 2.0f, -model_height / 2.0f,
        model_width / 2.0f, -model_height / 2.0f,  
        model_width / 2.0f, model_height / 2.0f,  
        -model_width / 2.0f, -model_height / 2.0f, 
        model_width / 2.0f, model_height / 2.0f,   
        -model_width / 2.0f, model_height / 2.0f   
    };


    float box_texture_coordinates[] = {
    0.0f, 0.0f,   
    1.0f, 0.0f,  
    1.0f, 1.0f,    
    0.0f, 0.0f,    
    1.0f, 1.0f,   
    0.0f, 1.0f      
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, box_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, box_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_box_model_matrix, g_box_texture_id);

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_omori_model_matrix, g_omori_texture_id);

    SCALE = 600;
    model_width = 623.0f / SCALE; 
    model_height = 400.0f / SCALE; 

    float cat_vertices[] = {
        -model_width / 2.0f, -model_height / 2.0f,  
        model_width / 2.0f, -model_height / 2.0f,  
        model_width / 2.0f, model_height / 2.0f,   
        -model_width / 2.0f, -model_height / 2.0f, 
        model_width / 2.0f, model_height / 2.0f,    
        -model_width / 2.0f, model_height / 2.0f   
    };


    float cat_texture_coordinates[] = {
    0.0f, 1.0f,
    1.0f, 1.0f, 
    1.0f, 0.0f,    
    0.0f, 1.0f,   
    1.0f, 0.0f,     
    0.0f, 0.0f     
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, cat_vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, cat_texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_cat_model_matrix, g_cat_texture_id);
    
    } else {
    int SCALE = 300;
    float model_width = 136.0f / SCALE; // Width of the box
    float model_height = 369.0f / SCALE; // Height of the box

    float vertices[] = {
    -model_height / 2.0f, -model_width / 2.0f, 
    -model_height / 2.0f, model_width / 2.0f,
    model_height / 2.0f, model_width / 2.0f,
    -model_height / 2.0f, -model_width / 2.0f,
    model_height / 2.0f, model_width / 2.0f,
    model_height / 2.0f, -model_width / 2.0f 
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };


    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_hand_model_matrix, g_hand_texture_id);

    draw_object(g_hand2_model_matrix, g_hand_texture_id);
    }

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
}

int main(int argc, char* argv[])
/**
* Author: Cato Wen
* Assignment: Simple 2D Scene
* Date due: 2024-02-17, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
