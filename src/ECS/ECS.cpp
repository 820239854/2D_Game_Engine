#include <algorithm>
#include "ECS.h"
#include "../Logger/Logger.h"

int IComponent::nextId = 0;

void Entity::Kill()
{
    registry->KillEntity(*this);
}

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

std::vector<Entity> System::GetSystemEntities() const
{
    return entities;
}

const Signature System::GetComponentsSignature() const
{
    return componentSignature;
}

Entity Registry::CreateEntity()
{
    int entityId;
    if (freeIds.empty())
    {
        entityId = numEntities++;
        if (entityId >= static_cast<int>(entityComponentsSignatures.size()))
        {
            entityComponentsSignatures.resize(entityId + 1);
        }
    }
    else
    {
        entityId = freeIds.front();
        freeIds.pop_front();
    }
    Entity entity(entityId);
    entity.registry = this;
    entitiesToCreate.insert(entity);

    Logger::Log("Entity created with id: " + std::to_string(entityId));
    return entity;
}

void Registry::KillEntity(Entity entity)
{
    entitiesToBeKilled.insert(entity);
    Logger::Log("Entity " + std::to_string(entity.GetId()) + " was killed");
}

void Registry::Update()
{
    for (auto entity : entitiesToCreate)
    {
        AddEntityToSystems(entity);
    }
    entitiesToCreate.clear();

    for (auto entity : entitiesToBeKilled)
    {
        RemoveEntityFromSystems(entity);
        entityComponentsSignatures[entity.GetId()].reset();
        freeIds.push_back(entity.GetId());
    }
    entitiesToBeKilled.clear();
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

void Registry::RemoveEntityFromSystems(Entity entity)
{
    for (auto &system : systems)
    {
        system.second->RemoveEntity(entity);
    }
}