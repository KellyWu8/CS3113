/**
* Author: Kelly Wu
* Assignment: UNDERGROUND HOPPER
* Date due: 2023-12-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_LEFT_EDGE 5.0f
#define LEVEL1_RIGHT_EDGE 13.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "Menu.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.16f,
            BG_BLUE = 0.14f,
            BG_GREEN = 0.13f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char  V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
            F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char FONT_SPRITE_FILEPATH[] = "assets/images/font1.png";
const int FONTBANK_SIZE = 16;

const float MILLISECONDS_IN_SECOND = 1000.0;

// ––––– GLOBAL VARIABLES ––––– //
Scene* g_current_scene;
Menu* g_menu;
LevelA* g_levelA;
LevelB* g_levelB;
LevelC* g_levelC;

Scene* g_levels[4];

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

GLuint g_text_texture_id;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

int g_total_lives = 3; // Lives Limit
int g_curr_lives_left = 3; // Current Lives Counter
int g_main_death_count = 0; // How many deaths so far throughout all levels

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene* scene)
{
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Project 6: UNDERGROUND HOPPER",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.m_program_id);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ————— TEXT ————— //
    glUseProgram(g_program.m_program_id);
    g_text_texture_id = Utility::load_texture(FONT_SPRITE_FILEPATH);

    // ————— Scenes ————— //
    g_menu = new Menu();
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();

    g_levels[0] = g_menu;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;

    // Start at Menu screen
    switch_to_scene(g_levels[0]);
}

void process_input()
{
    // If nothing is pressed, we don't want to go anywhere
    g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_game_is_running = false;
                break;

            case SDLK_SPACE:
                // Jump
                if (g_current_scene->m_state.player->get_is_active() && g_current_scene->m_state.player->m_collided_bottom)
                {
                    g_current_scene->m_state.player->m_is_jumping = true;
                    Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
                    Mix_VolumeChunk(g_current_scene->m_state.jump_sfx, MIX_MAX_VOLUME / 12);
                }
                break;

            case SDLK_RETURN:
                // Menu Screen -> LevelA
                if (g_current_scene == g_menu) {
                    g_current_scene->m_state.next_scene_id = 1;
                    switch_to_scene(g_levelA);
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_current_scene->m_state.player->move_left();
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_current_scene->m_state.player->move_right();
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->RIGHT];
    }

    if (glm::length(g_current_scene->m_state.player->get_movement()) > 1.0f)
    {
        g_current_scene->m_state.player->set_movement(glm::normalize(g_current_scene->m_state.player->get_movement()));
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);

        g_is_colliding_bottom = g_current_scene->m_state.player->m_collided_bottom;

        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    // Prevent the camera from showing anything outside of the "edge" of the level
    g_view_matrix = glm::mat4(1.0f);

    if (g_current_scene->m_state.player->get_position().x > LEVEL1_LEFT_EDGE && g_current_scene->m_state.player->get_position().x < LEVEL1_RIGHT_EDGE) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_position().x, 3.75, 0));
    }
    else if (g_current_scene->m_state.player->get_position().x > LEVEL1_RIGHT_EDGE) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-13, 3.75, 0));
    }
    else {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
    }

    // Maintain the same death count when we switch to another level
    if (g_main_death_count > g_current_scene->m_state.player->m_death_count && g_current_scene->m_state.player->m_death_count == 0) {
        g_current_scene->m_state.player->m_death_count = g_main_death_count;
    }

    if (g_main_death_count != g_current_scene->m_state.player->m_death_count) {
        g_main_death_count = g_current_scene->m_state.player->m_death_count;
    }
    g_curr_lives_left = g_total_lives - g_main_death_count;
    //std::cout << "g_curr_lives_left: " << g_curr_lives_left << std::endl;

    // Switching scenes
    if (g_current_scene == g_levelA && g_current_scene->m_state.player->get_position().x >= 18.0f) {    // LevelA -> LevelB
        switch_to_scene(g_levelB);
    }

    if (g_current_scene == g_levelB && g_current_scene->m_state.player->get_position().x >= 18.0f) {    // LevelB -> LevelC
        switch_to_scene(g_levelC);
    }
}

void render()
{
    g_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_program.m_program_id);
    g_current_scene->render(&g_program);

    // ----- TEXT ----- //
    // Text on Menu Scene
    if (g_current_scene == g_menu) {
        Utility::draw_text(&g_program, g_text_texture_id, std::string("UNDERGROUND HOPPER"), 0.5f, 0.01f, glm::vec3(0.5f, -2.5f, 0.0f));
        Utility::draw_text(&g_program, g_text_texture_id, std::string("Press enter to start"), 0.45f, 0.01f, glm::vec3(0.5f, -3.0f, 0.0f));
    }

    // Player's lives text
    if (g_current_scene != g_menu) {
        if (g_curr_lives_left == 3) {
            Utility::draw_text(&g_program, g_text_texture_id, std::string("3"), 0.45f, 0.01f, 
                               glm::vec3(g_current_scene->m_state.player->get_position().x,
                               g_current_scene->m_state.player->get_position().y + 0.57f, 0.0f));
        }
        else if (g_curr_lives_left == 2) {
            Utility::draw_text(&g_program, g_text_texture_id, std::string("2"), 0.45f, 0.01f,
                glm::vec3(g_current_scene->m_state.player->get_position().x,
                    g_current_scene->m_state.player->get_position().y + 0.57f, 0.0f));
        }
        else if (g_curr_lives_left == 1) {
            Utility::draw_text(&g_program, g_text_texture_id, std::string("1"), 0.45f, 0.01f,
                glm::vec3(g_current_scene->m_state.player->get_position().x,
                    g_current_scene->m_state.player->get_position().y + 0.57f, 0.0f));
        }
    }

    // Player has no more lives left
    if (g_curr_lives_left <= 0) {
        Utility::draw_text(&g_program, g_text_texture_id, std::string("You Lose!"), 0.5f, 0.01f, glm::vec3(3.0f, -2.5f, 0.0f));
        g_current_scene->m_state.player->deactivate();
    }

    // Player wins the game
    if (g_current_scene == g_levelC && g_current_scene->m_state.player->get_position().x >= 18.0f) {
        Utility::draw_text(&g_program, g_text_texture_id, std::string("You Win!"), 0.5f, 0.01f, glm::vec3(11.5f, -2.5f, 0.0f));
        g_current_scene->m_state.player->deactivate();
    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete g_menu;
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;

}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();

        if (g_current_scene->m_state.next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->m_state.next_scene_id]);

        render();
    }

    shutdown();
    return 0;
}