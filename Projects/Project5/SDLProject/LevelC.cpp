#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 30
#define LEVEL_HEIGHT 5

unsigned int LevelC_DATA[] =
{
     0,   0,    0,   0,  0,  0,  0,  0,  0,  0, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 101, 102, 102, 102, 102, 102, 102, 103,
     0,   0,    0,   0, 81, 82, 82, 82, 82, 83, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 101, 122, 122, 122, 122, 122, 101, 102, 103,
   101, 102,  103,   0,  0,  0,  0,  0,  0,  0, 0,   0, 101, 102, 102, 102, 102, 102, 102, 102, 102, 122, 122, 122, 122, 122, 122, 121, 122, 123,
    121,101,  102, 103,  0,  0,  0,  0,  0,  0, 0, 101, 102, 103, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 101, 102, 102, 103, 122, 123,
    121, 121, 122, 123,  0,  0,  0,  0,  0,  0, 0, 121, 122, 123, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 121, 122, 122, 123, 122, 123
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
    // ————— MAP SET-UP ————— //
    m_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/kenney_pixel-platformer/Tilemap/tilemap_packed.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LevelC_DATA, map_texture_id, 1.0f, 20, 9);

    // ————— PLAYER SET-UP ————— //
    // Existing
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
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
    m_state.player->set_width(0.6f);

    // Jumping
    m_state.player->m_jumping_power = 5.5f;

    // ––––– ENEMIES (SLIME) ––––– //
    GLuint enemy_texture_id = Utility::load_texture("assets/images/slime.png");
    m_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; ++i) {
        m_state.enemies[i].set_entity_type(ENEMY);
        m_state.enemies[i].m_texture_id = enemy_texture_id;
        m_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_state.enemies[i].set_speed(1.5f);
        m_state.enemies[i].set_acceleration(glm::vec3(0.0f, -12.0f, 0.0f));
        m_state.enemies[i].set_height(1.0f);
        m_state.enemies[i].set_width(0.6f);
    }

    // ENEMY 1
    m_state.enemies[0].set_ai_type(WALKER);
    m_state.enemies[0].set_position(glm::vec3(9.0f, 0.0f, 0.0f));

    // ENEMY 2
    m_state.enemies[1].set_ai_type(GUARD);
    m_state.enemies[1].set_ai_state(IDLE);
    m_state.enemies[1].set_position(glm::vec3(16.0f, 0.0f, 0.0f));

    // ENEMY 3
    m_state.enemies[2].set_ai_type(JUMPER);
    m_state.enemies[2].set_position(glm::vec3(25.0f, 2.0f, 0.0f));
    m_state.enemies[2].m_jumping_power = 0.25f;

    // ————— MUSIC/SOUND STUFF ————— //
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_state.bgm = Mix_LoadMUS("assets/audio/voxel_revolution.mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 16.0f);

    m_state.jump_sfx = Mix_LoadWAV("assets/audio/jump.wav");
}

void LevelC::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_state.enemies[i].update(delta_time, m_state.player, m_state.player, 1, m_state.map);
    }
}

void LevelC::render(ShaderProgram* program)
{
    // ––––– ENEMIES (SLIME) ––––– //
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_state.enemies[i].render(program);
    }

    // See if player killed all the enemies
    m_state.player->m_num_of_killed_enemies = 0;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (m_state.enemies[i].get_is_active() == false) {
            m_state.player->m_num_of_killed_enemies += 1;
        }
        else if (m_state.enemies[i].get_position().y < -5.0f) {    // Killed slime by luring it into the void
            m_state.player->m_num_of_killed_enemies += 1;
        }
    }

    // ––––– PLAYER ––––– //
    if (m_state.player->m_death_count == 3) {
        m_state.player->deactivate();
    }
    else {
        m_state.player->render(program);
    }

    // Player fell in the void
    if (m_state.player->get_position().y < -5.0f) {
        m_state.player->m_death_count += 1;
        m_state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    }

    // ––––– MAP ––––– //
    m_state.map->render(program);
}
