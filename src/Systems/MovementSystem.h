#define pragma once

class MovementSystem : public System
{
public:
    MovementSystem()
    {
        // RequireComponent<TransformComponent>();
        // RequireComponent<MovementComponent>();
    }

    void Update()
    {
        // for (auto entity : GetSystemEntities())
        // {
        //     auto &transform = entity.GetComponent<TransformComponent>();
        //     auto &movement = entity.GetComponent<MovementComponent>();

        //     transform.position += movement.velocity;
        // }
    }
};