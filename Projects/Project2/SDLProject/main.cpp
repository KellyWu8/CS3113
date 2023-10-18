/**
* Author: Kelly Wu
* Assignment: Pong Clone
* Date due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const int NUMBER_OF_TEXTURES = 1;   // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;   // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char PLAYER_1_SPRITE_FILEPATH[] = "dolphin1.png";
const char PLAYER_2_SPRITE_FILEPATH[] = "dolphin2.png";
const char BALL_SPRITE_FILEPATH[] = "ball.png";
const char P1_WIN_SPRITE_FILEPATH[] = "player_1_win.png";
const char P2_WIN_SPRITE_FILEPATH[] = "player_2_win.png";

// Define screen boundaries
const float TOP_BOUNDARY = 3.75f;
const float BOTTOM_BOUNDARY = -3.75f;
const float LEFT_BOUNDARY = -5.1f;
const float RIGHT_BOUNDARY = 5.1f;

const glm::vec3 PADDLE_SCA = glm::vec3(1.0f, 1.0f, 0.0f);
const glm::vec3 BALL_SCA = glm::vec3(1.0f, 1.0f, 0.0f);


SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
GLuint        g_player_1_texture_id, g_player_2_texture_id, g_ball_texture_id, g_p1_win_texture_id, g_p2_win_texture_id;
glm::mat4     g_view_matrix,
              g_model_matrix,
              g_projection_matrix,
              g_model_matrix_2,
              g_model_matrix_3,
              g_model_matrix_4,
              g_model_matrix_5;

float g_previous_ticks = 0.0f;
                                 
glm::vec3 g_player_1_position = glm::vec3(-4.0f, 0.0f, 0.0f);     
glm::vec3 g_player_1_movement = glm::vec3(0.0f, 0.0f, 0.0f);    

glm::vec3 g_player_2_position = glm::vec3(4.0f, 0.0f, 0.0f);
glm::vec3 g_player_2_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);

float g_player_speed = 3.0f;
float g_ball_speed = 0.73f;

bool player_2_manual_control = true;
bool player_2_go_up = true;


GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
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
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Project 2: Pong Clone!",
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

    g_player_1_texture_id = load_texture(PLAYER_1_SPRITE_FILEPATH);
    g_player_2_texture_id = load_texture(PLAYER_2_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_p1_win_texture_id = load_texture(P1_WIN_SPRITE_FILEPATH);
    g_p2_win_texture_id = load_texture(P2_WIN_SPRITE_FILEPATH);

    g_model_matrix = glm::mat4(1.0f);   // Player 1
    g_model_matrix_2 = glm::mat4(1.0f); // Player 2
    g_model_matrix_3 = glm::mat4(1.0f); // Ball

    // Ball starts moving diagonally
    g_ball_movement.x = 2.0f;
    g_ball_movement.y = 1.0f;

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere   
    g_player_1_movement = glm::vec3(0.0f);                    
    g_player_2_movement = glm::vec3(0.0f);
  
// –––––––––––––––––––––––––––––––– KEYSTROKES ––––––––––––––––––––––––– //
    SDL_Event event;                                                         
    while (SDL_PollEvent(&event))                                            
    {                                                                        
        switch (event.type)                                                  
        {                                                                    
        // End game                                                      
        case SDL_QUIT:                                                   
        case SDL_WINDOWEVENT_CLOSE:                                      
            g_game_is_running = false;                                   
            break;                                                       
  
        case SDL_KEYDOWN:                                                
            switch (event.key.keysym.sym)                                
            {                                                            
            case SDLK_w:                                          
                // Move the player_1 up
                g_player_1_movement.y = 1.0f;
                break;                                               
                
            case SDLK_s:                                         
                // Move the player_1 down  
                g_player_1_movement.y = -1.0f;                          

                break;    

            case SDLK_UP:
                // Move the player_2 up       
                if (player_2_manual_control) {
                    g_player_2_movement.y = 1.0f;
                }
                break;

            case SDLK_DOWN:
                // Move the player_2 down    
                if (player_2_manual_control) {
                    g_player_2_movement.y = -1.0f;
                }
                break;

            case SDLK_t:
                // Toggle control mode for player 2
                player_2_manual_control = !player_2_manual_control;
                break;

            case SDLK_q:                                             
                // Quit the game with a keystroke                    
                g_game_is_running = false;                           
                break;                                               
                
            default:                                                 
                break;                                               
            }                                                            
                                                                         
        default:                                                         
            break;                                                       
        }                                                                    
    }                                                                        
                                                                             
    // ––––––––––––––––––––––––––––––– KEY HOLD –––––––––––––––––––––––––––– //
                                                                             
    const Uint8* key_state = SDL_GetKeyboardState(NULL);                     
                                                            
                                                                             
    if (key_state[SDL_SCANCODE_W])                                          
    {                                                                        
        g_player_1_movement.y = 1.0f;                                          
    }                                                                        
    else if (key_state[SDL_SCANCODE_S])                                   
    {                                                                        
        g_player_1_movement.y = -1.0f;                                         
    }    

    if (key_state[SDL_SCANCODE_UP] && player_2_manual_control)
    {
        g_player_2_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN] && player_2_manual_control)
    {
        g_player_2_movement.y = -1.0f;
    }
                                                                             
    // This makes sure that the player can't "cheat" their way into moving faster                                                                
    if (glm::length(g_player_1_movement) > 1.0f)                               
    {                                                                        
        g_player_1_movement = glm::normalize(g_player_1_movement);               
    }      

    if (glm::length(g_player_2_movement) > 1.0f)
    {
        g_player_2_movement = glm::normalize(g_player_2_movement);
    }
    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks; 

    // ----------- Calculate the new position for players ----------- //
    glm::vec3 p1_new_position = g_player_1_position + g_player_1_movement * g_player_speed * delta_time;
    glm::vec3 p2_new_position;

    if (player_2_manual_control == true) {
        p2_new_position = g_player_2_position + g_player_2_movement * g_player_speed * delta_time;
    }
    else {  // Player 2 in auto mode
        if (player_2_go_up == true) {
            g_player_2_movement.y = 1.0f;
        }
        else {
            g_player_2_movement.y = -1.0f;
        }
        p2_new_position = g_player_2_position + g_player_2_movement * g_player_speed * delta_time;
    }

    // ----------- Check boundaries for paddles ----------- //
    if (p1_new_position.y + 0.5f <= TOP_BOUNDARY && p1_new_position.y - 0.5f >= BOTTOM_BOUNDARY)
    {
        g_player_1_position = p1_new_position;
    }

    if (p2_new_position.y + 0.5f <= TOP_BOUNDARY && p2_new_position.y - 0.5f >= BOTTOM_BOUNDARY)
    {
        g_player_2_position = p2_new_position;
    }
    else {
        player_2_go_up = !player_2_go_up;
    }

    // ----------- Check for any collisions ----------- //
    // Calculate the new position for ball
    glm::vec3 ball_new_position = g_ball_position + (g_ball_movement * g_ball_speed * delta_time);

    float collision_factor = 0.95f;

    float x_distance_p1 = fabs(ball_new_position.x - p1_new_position.x) - ((PADDLE_SCA.x * collision_factor + 
                                                                            BALL_SCA.x   * collision_factor) / 2.0f);
    float y_distance_p1 = fabs(ball_new_position.y - p1_new_position.y) - ((PADDLE_SCA.y * collision_factor + 
                                                                            BALL_SCA.y   * collision_factor) / 2.0f);

    // Collision with player 1 paddle
    if (x_distance_p1 < 0.0f && y_distance_p1 < 0.0f)
    {
        g_ball_movement.x = -g_ball_movement.x;
    }

    float x_distance_p2 = fabs(ball_new_position.x - p2_new_position.x) - ((PADDLE_SCA.x * collision_factor +
                                                                            BALL_SCA.x   * collision_factor) / 2.0f);
    float y_distance_p2 = fabs(ball_new_position.y - p2_new_position.y) - ((PADDLE_SCA.y * collision_factor + 
                                                                            BALL_SCA.y   * collision_factor) / 2.0f);

    // Collision with player 2 paddle
    if (x_distance_p2 < 0.0f && y_distance_p2 < 0.0f)
    {
        g_ball_movement.x = -g_ball_movement.x;
    }

    // ----------- Check boundaries for ball ----------- //
    //  Ball touch TOP of screen
    if (ball_new_position.y + 0.5f >= TOP_BOUNDARY)
    {
        g_ball_movement.y = -g_ball_movement.y;
        ball_new_position.y = TOP_BOUNDARY - 0.5f;
    }
    //  Ball touch BOTTOM of screen
    if (ball_new_position.y - 0.5f <= BOTTOM_BOUNDARY)
    {
        g_ball_movement.y = -g_ball_movement.y;
        ball_new_position.y = BOTTOM_BOUNDARY + 0.5f;
    }
    //  Ball touch LEFT of screen
    if (ball_new_position.x - 0.5f <= LEFT_BOUNDARY)
    {
        g_model_matrix_5 = glm::mat4(1.0f); // Player 2 Win
        g_game_is_running = false;
    }
    //  Ball touch RIGHT of screen
    if (ball_new_position.x + 0.5f >= RIGHT_BOUNDARY)
    {
        g_model_matrix_4 = glm::mat4(1.0f); // Player 1 Win
        g_game_is_running = false;
    }

    ball_new_position = g_ball_position + (g_ball_movement * g_ball_speed * delta_time);    // Reupdate ball position
    g_ball_position = ball_new_position;
    
    // ----------- Transformations ----------- //
    g_model_matrix = glm::mat4(1.0f);    
    g_model_matrix_2 = glm::mat4(1.0f);
    g_model_matrix_3 = glm::mat4(1.0f);

    g_model_matrix = glm::translate(g_model_matrix, g_player_1_position);
    g_model_matrix_2 = glm::translate(g_model_matrix_2, g_player_2_position);
    g_model_matrix_3 = glm::translate(g_model_matrix_3, g_ball_position);

}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // OBJECT 1
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
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

    // Bind texture
    g_shader_program.set_model_matrix(g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_player_1_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // OBJECT 2
    // Vertices
    float vertices_2[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates_2[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices_2);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates_2);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    g_shader_program.set_model_matrix(g_model_matrix_2);
    glBindTexture(GL_TEXTURE_2D, g_player_2_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // OBJECT 3
    // Vertices
    float vertices_3[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates_3[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices_3);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates_3);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    g_shader_program.set_model_matrix(g_model_matrix_3);
    glBindTexture(GL_TEXTURE_2D, g_ball_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // OBJECT 4
    // Vertices
    float vertices_4[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,  // triangle 1
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f   // triangle 2
    };

    // Textures
    float texture_coordinates_4[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices_4);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates_4);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    g_shader_program.set_model_matrix(g_model_matrix_4);
    glBindTexture(GL_TEXTURE_2D, g_p1_win_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // OBJECT 5
    // Vertices
    float vertices_5[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,  // triangle 1
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f   // triangle 2
    };

    // Textures
    float texture_coordinates_5[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices_5);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates_5);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    g_shader_program.set_model_matrix(g_model_matrix_5);
    glBindTexture(GL_TEXTURE_2D, g_p2_win_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
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
