#define pragma once

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
class MovementSystem : public System
{
public:
    MovementSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<RigidBodyComponent>();
    }

    void Update(double deltaTime)
    {
        for (auto entity : GetEntities())
        {
            auto &transform = entity.GetComponent<TransformComponent>();
            const auto movement = entity.GetComponent<RigidBodyComponent>();
            transform.position.x += movement.velocity.x * deltaTime;
            transform.position.y += movement.velocity.y * deltaTime;

            // Logger::Log("Entity " + std::to_string(entity.GetId()) + " moved to position: " + std::to_string(transform.position.x) + ", " + std::to_string(transform.position.y));
        }
    }
};