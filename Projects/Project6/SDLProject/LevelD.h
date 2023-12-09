#include "Scene.h"

class LevelD : public Scene {
public:
    int ENEMY_COUNT = 10;
    int LIVES_LIMIT = 5;

    ~LevelD();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
