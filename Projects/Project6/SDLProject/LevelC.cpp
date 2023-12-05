#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 19
#define LEVEL_HEIGHT 8

unsigned int LevelC_DATA[] =
{
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 0, 0, 0, 0, 0, 9, 9, 0, 9, 0, 0, 0, 9, 0, 9, 9, 9, 9,
    9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 0, 0, 0, 0, 0,
    9, 0, 9, 0, 0, 9, 9, 9, 9, 0, 9, 9, 0, 0, 0, 9, 9, 9, 9,
    9, 9, 9, 0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 9, 0, 9, 9, 9, 9,
    9, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 9, 9, 9, 0, 9, 0, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 0, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, 0,
    9, 9, 9, 9, 9, 9, 9, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
};

LevelC::~LevelC()
{
    delete[] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelC::initialise()
{
    m_state.next_scene_id = -1;

    // ————— MAP SET-UP ————— //
    GLuint map_texture_id = Utility::load_texture("assets/kenney_pixel-platformer/Tilemap/tilemap_packed.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LevelC_DATA, map_texture_id, 1.0f, 20, 9);

    // ————— PLAYER SET-UP ————— //
    // Existing
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(1.0f, -1.0f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->set_speed(3.0f);
    m_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("assets/images/player.png");

    // Walking
    m_state.player->m_walking[m_state.player->LEFT] = new int[8] { 40, 41, 42, 43, 44, 45, 46, 47 };
    m_state.player->m_walking[m_state.player->RIGHT] = new int[8] { 56, 57, 58, 59, 60, 61, 62, 63 };
    m_state.player->m_walking[m_state.player->UP] = new int[8] { 32, 33, 34, 35, 36, 37, 38, 39};
    m_state.player->m_walking[m_state.player->DOWN] = new int[8] { 48, 49, 50, 51, 52, 53, 54, 55 };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];  // start player looking right
    m_state.player->m_animation_frames = 8;
    m_state.player->m_animation_index = 0;
    m_state.player->m_animation_time = 0.0f;
    m_state.player->m_animation_cols = 8;
    m_state.player->m_animation_rows = 8;
    m_state.player->set_height(0.73f);
    m_state.player->set_width(0.3f);

    // Jumping
    m_state.player->m_jumping_power = 5.0f;

    // ––––– ENEMIES (SLIME) ––––– //
    GLuint enemy_texture_id = Utility::load_texture("assets/images/slime.png");
    m_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; ++i) {
        m_state.enemies[i].set_entity_type(ENEMY);
        m_state.enemies[i].m_texture_id = enemy_texture_id;
        m_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_state.enemies[i].set_speed(1.84f);
        m_state.enemies[i].set_acceleration(glm::vec3(0.0f, -12.0f, 0.0f));
        m_state.enemies[i].set_height(1.0f);
        m_state.enemies[i].set_width(0.3f);
    }

    // ENEMY 1
    m_state.enemies[0].set_ai_type(GUARD);
    m_state.enemies[0].set_ai_state(IDLE);
    m_state.enemies[0].set_position(glm::vec3(1.0f, -5.0f, 0.0f));

    // ENEMY 2
    m_state.enemies[1].set_ai_type(GUARD);
    m_state.enemies[1].set_ai_state(IDLE);
    m_state.enemies[1].set_position(glm::vec3(8.0f, -2.0f, 0.0f));

    // ENEMY 3
    m_state.enemies[2].set_ai_type(WALKER);
    m_state.enemies[2].set_position(glm::vec3(11.0f, -1.0f, 0.0f));
    m_state.enemies[2].m_initial_position = glm::vec3(11.0f, -1.0f, 0.0f);
    m_state.enemies[2].m_left_boundary_offset = 0.0f;
    m_state.enemies[2].m_right_boundary_offset = 1.0f;

    // ENEMY 4
    m_state.enemies[3].set_ai_type(GUARD);
    m_state.enemies[3].set_ai_state(IDLE);
    m_state.enemies[3].set_position(glm::vec3(15.0f, -2.0f, 0.0f));

    // ENEMY 5
    m_state.enemies[4].set_ai_type(WALKER);
    m_state.enemies[4].set_position(glm::vec3(15.0f, -6.0f, 0.0f));
    m_state.enemies[4].m_initial_position = glm::vec3(15.0f, -6.0f, 0.0f);
    m_state.enemies[4].m_left_boundary_offset = 0.0f;
    m_state.enemies[4].m_right_boundary_offset = 3.0f;
    m_state.enemies[4].set_speed(2.5f);

    // ————— MUSIC/SOUND STUFF ————— //
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_state.bgm = Mix_LoadMUS("assets/audio/enter_the_maze.mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 10.0f);

    m_state.jump_sfx = Mix_LoadWAV("assets/audio/jump.wav");
}

void LevelC::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);

    // Player fell in the void
    if (m_state.player->get_position().y < -8.0f && m_state.player->get_position().x < 18.0f) {
        m_state.player->m_death_count += 1;
        m_state.player->set_position(glm::vec3(1.0f, -1.0f, 0.0f));
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_state.enemies[i].update(delta_time, m_state.player, m_state.player, 1, m_state.map);
    }
}

void LevelC::render(ShaderProgram* program)
{
    // ––––– MAP ––––– //
    m_state.map->render(program);

    // ––––– ENEMIES (SLIME) ––––– //
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_state.enemies[i].render(program);
    }

    // ––––– PLAYER ––––– //
    if (m_state.player->m_death_count == 3) {
        m_state.player->deactivate();
    }
    else {
        m_state.player->render(program);
    }
}