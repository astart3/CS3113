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

#define LOG(argument) std::cout << argument << '\n'

SDL_Window* g_display_window;
bool g_game_is_running = true; //tracks whether game is running

//DEFINE GLOBAL CONSTANTS
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
//const float MINIMUM_X_COLLISION_DISTANCE = 0.375f;
//const float MINIMUM_Y_COLLISION_DISTANCE = 0.8f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

//filepaths for assets
const char IDLE_SPRITE_FILEPATH[] = "sprites/ship_idle.png";     //192 x 192
const char MOVING_SPRITE_FILEPATH[] = "sprites/ship_move.png";     //1152 x 192 = 6 * 192 * 192

//DEFINE GLOBAL VARIABLES
//texture files
GLuint g_idle_texture_id;
GLuint g_moving_texture_id;

ShaderProgram g_shader_program; //shader program
glm::mat4 view_matrix, g_projection_matrix;
//model matrices of assets use
glm::mat4 g_player_model_matrix;

float g_previous_ticks = 0.0f; //used for delta time calculation


glm::vec3 g_player_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_player_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_player_acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
bool g_accelerating = false;

const glm::vec3 GRAVITY_ACCELERATION = glm::vec3(0.0f, -0.1f, 0.0f);
const float SHIP_ACCELERATION = 0.2f;


float g_player_speed = 5.0f;

//const float ROT_SPEED = 22.5f;
const float ROT_SPEED = 1.0f;
float g_ship_angle = 0.0f;


//START OF CODE -----------------------------------------------------------------------------------------

//FUNCTION PROFESSOR WROTE IN CLASS
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

    g_display_window = SDL_CreateWindow("Pong Game",
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

    g_player_model_matrix = glm::mat4(1.0f);

    view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f); 

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(255.0f, 255.0f, 255.0f, 1.0f); //sets background to white by default

    //loads textures based on filepath
    g_idle_texture_id = load_texture(IDLE_SPRITE_FILEPATH);
    g_moving_texture_id = load_texture(MOVING_SPRITE_FILEPATH);


    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_player_velocity = glm::vec3(0.0f);
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;
            //keystrokes check
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q:
                g_game_is_running = false;
                break;
            default:
                break;
            }
        default:
            break;
        }
    }

    //key hold checks                                                                       
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_ship_angle += ROT_SPEED;
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_ship_angle += -ROT_SPEED;
    }
    if (key_state[SDL_SCANCODE_SPACE])
    {
        g_accelerating = true;
    }
    else {
        g_accelerating = false;
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    glClearColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f);
    
    if (g_accelerating) {
        g_player_velocity += glm::vec3(SHIP_ACCELERATION * sin(g_ship_angle),SHIP_ACCELERATION * cos(g_ship_angle), 0.0f);
    }
    g_player_velocity += GRAVITY_ACCELERATION;
    g_player_position += g_player_velocity * g_player_speed * delta_time;

    g_player_model_matrix = glm::mat4(1.0f);
    g_player_model_matrix = glm::translate(g_player_model_matrix, g_player_position);
    g_player_model_matrix = glm::rotate(g_player_model_matrix, glm::radians(g_ship_angle), glm::vec3(0.0f, 0.0f, 1.0f));
}

//FUNCTION PROFESSOR USED IN EXAMPLE
void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    //declare vertices based on dimension of image
    int SCALE = 240;
    float model_width = 192.0f / SCALE; // width of the image
    float model_height = 192.0f / SCALE; // height of the image
    float vertices[] = {
        model_width, -model_width,
        model_width, model_width,
        -model_width, model_width,  // triangle 1
        model_width, -model_width,
        -model_width, model_width,
        -model_width, -model_width   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };


    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_player_model_matrix, g_idle_texture_id);
   
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
* Assignment: Lunar Lander
* Date due: 2024-09-03, 11:59pm
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
