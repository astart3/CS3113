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
#include "Entity.h"
#include <iostream>
#include <vector>

#define LOG(argument) std::cout << argument << '\n'

struct GameState
{
    Entity* player;
};

struct Box
{
    glm::mat4 m_model_matrix = glm::mat4(1.0f);
    bool is_black = true;
    glm::vec3 m_position = glm::vec3(0.0f);

    Box(glm::mat4 matrix, glm::vec3 pos, bool black) {
        m_model_matrix = matrix;
        m_position = pos;
        is_black = black;
    }
    
    void render(ShaderProgram* program, GLuint* texture_id)
    {
        program->set_model_matrix(m_model_matrix);

        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

        glBindTexture(GL_TEXTURE_2D, *texture_id);

        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    }
};

std::vector<Box> g_boxes;

void initializeBoxes(std::vector<Box>& boxes) {
    int cols = 11;
    float posX = -((cols-1)/2);
    for (int col = 0; col < cols; ++col) {
        posX = -((cols - 1) / 2) + col;

        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(posX, -3.5f, 0.0f));

        glm::vec3 modelPosition = glm::vec3(posX, -3.5f, 0.0f);

        // Alternate between black and white boxes
        bool isBlack = (col % 2 == 0);

        boxes.emplace_back(modelMatrix, modelPosition, isBlack);
    }
}

GameState g_game_state;

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

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char IDLE_SPRITE_FILEPATH[] = "sprites/ship_idle.png";     
const char MOVING_SPRITE_FILEPATH[] = "sprites/ship_move.png"; 
const char BLACK_BOX_SPRITE_FILEPATH[] = "sprites/black_box.png";
const char RED_BOX_SPRITE_FILEPATH[] = "sprites/red_box.png";

GLuint g_black_box_texture_id;
GLuint g_red_box_texture_id;

ShaderProgram g_shader_program; //shader program
glm::mat4 view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f; //used for delta time calculation

bool g_game_end = false;
bool g_game_win = false;


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

    g_display_window = SDL_CreateWindow("Lunar Lander",
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

    view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f); 

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(255.0f, 255.0f, 255.0f, 1.0f); //sets background to white by default

    g_black_box_texture_id = load_texture(BLACK_BOX_SPRITE_FILEPATH);
    g_red_box_texture_id = load_texture(RED_BOX_SPRITE_FILEPATH);

    initializeBoxes(g_boxes);

    // ————— PLAYER ————— //
    g_game_state.player = new Entity();
    g_game_state.player->set_position(glm::vec3(-3.0f, 3.0f, 0.0f));
    g_game_state.player->set_velocity(glm::vec3(0.0f));
    g_game_state.player->set_idle_texture_id(load_texture(IDLE_SPRITE_FILEPATH));
    g_game_state.player->set_moving_texture_id(load_texture(MOVING_SPRITE_FILEPATH));

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
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
        g_game_state.player->rotate_left();
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_game_state.player->rotate_right();
    }
    if (key_state[SDL_SCANCODE_SPACE])
    {
        g_game_state.player->set_acceleration(true);
    }
    else {
        g_game_state.player->set_acceleration(false);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;
    
    g_game_state.player->update(delta_time);

    for (const auto& box : g_boxes) {
        if (g_game_state.player->check_collision(box.m_position)) {
            if (box.is_black) {
                g_game_win = true;
            }
            else {
                g_game_win = false;
            }
            g_game_end = true;
        }
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto& box : g_boxes) {
        if (box.is_black) {
            box.render(&g_shader_program, &g_black_box_texture_id);
        }
        else {
            box.render(&g_shader_program, &g_red_box_texture_id);
        }
    }

    g_game_state.player->render(&g_shader_program);

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
* Date due: 2024-03-09, 11:59pm
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
