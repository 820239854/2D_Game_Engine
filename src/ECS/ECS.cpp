#include <algorithm>
#include "ECS.h"

void System::AddEntity(Entity entity)
{
    entities.push_back(entity);
}

void System::RemoveEntity(Entity entity)
{
    entities.erase(
        std::remove_if(
            entities.begin(), entities.end(),
            [&entity](Entity otherEntity)
            { return entity == otherEntity; }),
        entities.end());
}

std::vector<Entity> System::GetEntities() const
{
    return entities;
}

const Signature System::GetComponentsSignature() const
{
    return componentSignature;
}