class Entity
{
private:
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 m_model_matrix;
    bool      m_accelerating = false;
    float     m_ship_angle = 0.0f;

    // ————— STATIC VARIABLES ————— //
    const glm::vec3 GRAVITY_ACCELERATION = glm::vec3(0.0f, -0.02f, 0.0f);
    const float SHIP_ACCELERATION = 0.05f;
    const float MAX_GRAVITY_VELOCITY = -1.5f;
    const float MAX_HORIZONTAL_VELOCITY = 1.0f;
    static const int SECONDS_PER_FRAME = 4;
    const float SHIP_SPEED = 2.0f;
    const float ROT_SPEED = 1.0f;
    const float COLLISION_DIST = 0.5f;
    
    // ————— TEXTURES ————— //
    GLuint    m_idle_texture_id;
    GLuint    m_moving_texture_id;
    
public:
    // ————— ANIMATION ————— //   
    float m_animation_time    = 0.0f;
    int m_animation_index = 0;

    // ————— METHODS ————— //
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index);
    void update(float delta_time);
    void render(ShaderProgram *program);
    
    void rotate_left() { m_ship_angle += 1.0f; };
    void rotate_right() { m_ship_angle += -1.0f; };
    bool const check_collision(const glm::vec3& boxPosition) const;
    
    // ————— GETTERS ————— //
    glm::vec3 const get_position()   const { return m_position;   };
    glm::vec3 const get_velocity()   const { return m_velocity;   };
    GLuint    const get_idle_texture_id() const { return m_idle_texture_id; };
    GLuint    const get_moving_texture_id() const { return m_moving_texture_id; };

    
    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position)  { m_position   = new_position;     };
    void const set_velocity(glm::vec3 new_velocity)  { m_velocity   = new_velocity;     };
    void const set_idle_texture_id(GLuint new_texture_id) { m_idle_texture_id = new_texture_id;   };
    void const set_moving_texture_id(GLuint new_texture_id) { m_moving_texture_id = new_texture_id; };
    void const set_acceleration(bool accel)          { m_accelerating = accel;          };
};
