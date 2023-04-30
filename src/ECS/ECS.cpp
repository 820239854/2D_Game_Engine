#include <algorithm>
#include "ECS.h"
#include "../Logger/Logger.h"

int IComponent::nextId = 0;

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

Entity Registry::CreateEntity()
{
    int entityId = numEntities++;
    if (entityId >= entityComponentsSignatures.size())
    {
        entityComponentsSignatures.resize(entityId + 1);
    }
    Entity entity(entityId);
    entitiesToCreate.insert(entity);
    if (entityId >= entityComponentsSignatures.size())
    {
        entityComponentsSignatures.resize(entityId + 1);
    }

    Logger::Log("Entity created with id: " + std::to_string(entityId));
    return entity;
}

void Registry::Update()
{
    for (auto entity : entitiesToCreate)
    {
        AddEntityToSystems(entity);
    }
    entitiesToCreate.clear();
}

void Registry::AddEntityToSystems(Entity entity)
{
    const auto entityId = entity.GetId();
    const auto entitySignature = entityComponentsSignatures[entityId];
    for (auto &system : systems)
    {
        const auto &systemSignature = system.second->GetComponentsSignature();
        if ((entitySignature & systemSignature) == systemSignature)
        {
            system.second->AddEntity(entity);
        }
    }
}