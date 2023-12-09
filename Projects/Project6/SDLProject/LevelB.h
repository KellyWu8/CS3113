#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 5;
    int LIVES_LIMIT = 5;

    ~LevelB();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
