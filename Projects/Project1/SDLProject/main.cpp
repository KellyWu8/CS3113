/**
* Author: Kelly Wu
* Assignment: Simple 2D Scene
* Date due: 2023-09-30, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

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
const float DEGREES_PER_SECOND = 90.0f;

const float MIN_Y_DIRECTION = -2;
const float MAX_Y_DIRECTION = 2;

const float SCALE_FACTOR = 0.5f;
const int MAX_FRAME = 400;

const float RADIUS = 1.5f;      // radius of your circle
const float ROT_SPEED = 1.5f;  // rotational speed

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char BUNNY_SPRITE_FILEPATH[] = "bunny.png";
const char CARROT_SPRITE_FILEPATH[] = "carrot.png";

SDL_Window* g_display_window;
bool g_game_is_running = true;
bool g_is_growing = true;
bool g_move_up = true;
int  g_frame_counter = 0;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_model_matrix, g_projection_matrix, g_model_matrix_2;

float g_triangle_x = 0.0f;
float g_triangle_y = 0.0f;
float g_y_direction_counter = 0.0f;
float g_triangle_rotate = 0.0f;
float g_angle = 0.0f;     // current angle
float g_x_coord = RADIUS, // current x and y coordinates
      g_y_coord = 0.0f;
float g_previous_ticks = 0.0f;
float g_scale_size = 1.0f;

GLuint g_bunny_texture_id, g_carrot_texture_id;

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
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Project 1: Simple 2D Scene",
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

    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix_2 = glm::mat4(1.0f);
    g_model_matrix_2 = glm::translate(g_model_matrix_2, glm::vec3(2.0f, 0.0f, 0.0f));
    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    // Notice we haven't set our model matrix yet!

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_bunny_texture_id = load_texture(BUNNY_SPRITE_FILEPATH);
    g_carrot_texture_id = load_texture(CARROT_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_game_is_running = false;
        }
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    // ----------- Moving bunny diagonally ----------- //
    if (g_move_up == true) {
        if (g_triangle_y >= MAX_Y_DIRECTION) {
            g_move_up = false;
        }
        else {
            g_triangle_x += 0.5f * delta_time;
            g_triangle_y += 0.5f * delta_time;
        }
    }
    else {
        if (g_triangle_y <= MIN_Y_DIRECTION) {
            g_move_up = true;
        }
        else {
            g_triangle_x -= 0.5f * delta_time;
            g_triangle_y -= 0.5f * delta_time;
        }
    }

    // ----------- Scaling rabbit - heartbeat ----------- //
    glm::vec3 scale_vector;
    g_frame_counter += 1;

    if (g_frame_counter >= MAX_FRAME)
    {
        g_is_growing = !g_is_growing;
        g_frame_counter = 0;
    }

    if (g_is_growing) {
        g_scale_size += SCALE_FACTOR * delta_time;
    }
    else {
        g_scale_size -= SCALE_FACTOR * delta_time;
    }

    scale_vector = glm::vec3(g_scale_size, g_scale_size, 1.0f);

    // ----------- Orbiting carrot ----------- //
    g_angle += ROT_SPEED * delta_time;     // increment g_angle by ROT_SPEED

    g_x_coord = RADIUS * glm::cos(g_angle);
    g_y_coord = RADIUS * glm::sin(g_angle);

    // ----------- Rotating carrot ----------- //
    g_triangle_rotate += DEGREES_PER_SECOND * delta_time; // 90-degrees per second

    // ----------- Reset Both Matrix ----------- //
    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix_2 = glm::mat4(1.0f);

    // ----------- Transformations ----------- //
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(g_triangle_x, g_triangle_y, 0.0f));
    g_model_matrix_2 = glm::translate(g_model_matrix_2, glm::vec3(2.0f, 0.0f, 0.0f));
    g_model_matrix_2 = glm::translate(g_model_matrix, glm::vec3(g_x_coord, g_y_coord, 0.0f));
    g_model_matrix_2 = glm::rotate(g_model_matrix_2, glm::radians(g_triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    g_model_matrix = glm::scale(g_model_matrix, scale_vector);
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
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
    draw_object(g_model_matrix, g_bunny_texture_id);

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
    draw_object(g_model_matrix_2, g_carrot_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
}

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
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