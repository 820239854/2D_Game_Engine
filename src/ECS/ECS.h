#ifndef ECS_H
#define ECS_H

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <memory>
#include <deque>
#include "../Logger/Logger.h"

const unsigned int MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent
{
protected:
    static int nextId;
};

template <typename T>
class Component : public IComponent
{
public:
    static int GetId()
    {
        static auto id = nextId++;
        return id;
    }

    Component(int id) : id(id)
    {
    }

private:
    int id;
};

class Entity
{
public:
    Entity(int id) : id(id)
    {
    }

    Entity(const Entity &other) = default;
    void Kill();

    int GetId() const
    {
        return id;
    }

    // Manage entity tags and groups
    void Tag(const std::string &tag);
    bool HasTag(const std::string &tag) const;
    void Group(const std::string &group);
    bool BelongsToGroup(const std::string &group) const;

    template <typename TComponrnt, typename... TArgs>
    void AddComponent(TArgs &&...args);
    template <typename TComponrnt>
    void RemoveComponent();
    template <typename TComponrnt>
    bool HasComponent() const;
    template <typename TComponrnt>
    TComponrnt &GetComponent() const;

    Entity &operator=(const Entity &other) = default;

    bool operator==(const Entity &other) const
    {
        return id == other.id;
    }

    bool operator!=(const Entity &other) const
    {
        return id != other.id;
    }

    bool operator<(const Entity &other) const
    {
        return id < other.id;
    }

    bool operator>(const Entity &other) const
    {
        return id > other.id;
    }

    class Registry *registry;

private:
    int id;
};

class System
{
public:
    System() = default;
    ~System() = default;

    void AddEntity(Entity entity);
    void RemoveEntity(Entity entity);
    std::vector<Entity> GetSystemEntities() const;
    const Signature GetComponentsSignature() const;

    template <typename TComponrnt>
    void RequireComponent();

private:
    Signature componentSignature;
    std::vector<Entity> entities;
};

////////////////////////////////////////////////////////////////////////////////
// Pool
////////////////////////////////////////////////////////////////////////////////
// A pool is just a vector (contiguous data) of objects of type T
////////////////////////////////////////////////////////////////////////////////
class IPool
{
public:
    virtual ~IPool() = default;
    virtual void RemoveEntityFromPool(int entityId) = 0;
};

template <typename T>
class Pool : public IPool
{
private:
    // We keep track of the vector of objects and the current number of elements
    std::vector<T> data;
    int size;

    // Helper maps to keep track of entity ids per index, so the vector is always packed
    std::unordered_map<int, int> entityIdToIndex;
    std::unordered_map<int, int> indexToEntityId;

public:
    Pool(int capacity = 100)
    {
        size = 0;
        data.resize(capacity);
    }

    virtual ~Pool() = default;

    bool IsEmpty() const
    {
        return size == 0;
    }

    int GetSize() const
    {
        return size;
    }

    void Clear()
    {
        data.clear();
        entityIdToIndex.clear();
        indexToEntityId.clear();
        size = 0;
    }

    void Set(int entityId, T object)
    {
        if (entityIdToIndex.find(entityId) != entityIdToIndex.end())
        {
            // If the element already exists, simply replace the component object
            int index = entityIdToIndex[entityId];
            data[index] = object;
        }
        else
        {
            // When adding a new object, we keep track of the entity ids and their vector index
            int index = size;
            entityIdToIndex.emplace(entityId, index);
            indexToEntityId.emplace(index, entityId);
            if (index >= static_cast<int>(data.capacity()))
            {
                // If necessary, we resize by always doubling the current capacity
                data.resize(size * 2);
            }
            data[index] = object;
            size++;
        }
    }

    void Remove(int entityId)
    {
        // Copy the last element to the deleted position to keep the array packed
        int indexOfRemoved = entityIdToIndex[entityId];
        int indexOfLast = size - 1;
        data[indexOfRemoved] = data[indexOfLast];

        // Update the index-entity maps to point to the correct elements
        int entityIdOfLastElement = indexToEntityId[indexOfLast];
        entityIdToIndex[entityIdOfLastElement] = indexOfRemoved;
        indexToEntityId[indexOfRemoved] = entityIdOfLastElement;

        entityIdToIndex.erase(entityId);
        indexToEntityId.erase(indexOfLast);

        size--;
    }

    void RemoveEntityFromPool(int entityId) override
    {
        if (entityIdToIndex.find(entityId) != entityIdToIndex.end())
        {
            Remove(entityId);
        }
    }

    T &Get(int entityId)
    {
        int index = entityIdToIndex[entityId];
        return static_cast<T &>(data[index]);
    }

    T &operator[](unsigned int index)
    {
        return data[index];
    }
};

class Registry
{
public:
    Registry()
    {
        Logger::Log("Registry created");
    };
    ~Registry()
    {
        Logger::Log("Registry destroyed");
    };

    Entity CreateEntity();
    void KillEntity(Entity entity);

    // Tag management
    void TagEntity(Entity entity, const std::string &tag);
    bool EntityHasTag(Entity entity, const std::string &tag) const;
    Entity GetEntityByTag(const std::string &tag) const;
    void RemoveEntityTag(Entity entity);

    // Group management
    void GroupEntity(Entity entity, const std::string &group);
    bool EntityBelongsToGroup(Entity entity, const std::string &group) const;
    std::vector<Entity> GetEntitiesByGroup(const std::string &group) const;
    void RemoveEntityGroup(Entity entity);

    void Update();
    void AddEntityToSystem(Entity entity, System *system);
    void RemoveEntityFromSystem(Entity entity, System *system);

    template <typename TComponrnt, typename... TArgs>
    void AddComponent(Entity entity, TArgs &&...args);
    template <typename TComponrnt>
    void RemoveComponent(Entity entity);
    template <typename TComponrnt>
    bool HasComponent(Entity entity) const;
    template <typename TComponrnt>
    TComponrnt &GetComponent(Entity entity) const;

    template <typename TSystem, typename... TArgs>
    void AddSystem(TArgs &&...args);
    template <typename TSystem>
    void RemoveSystem();
    template <typename TSystem>
    bool HasSystem() const;
    template <typename TSystem>
    TSystem &GetSystem() const;

    void AddEntityToSystems(Entity entity);
    void RemoveEntityFromSystems(Entity entity);

private:
    int numEntities = 0;
    std::vector<std::shared_ptr<IPool>> componentPools;
    std::vector<Signature> entityComponentSignatures;
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

    std::set<Entity> entitiesToBeAdded;
    std::set<Entity> entitiesToBeKilled;

    // Entity tags (one tag name per entity)
    std::unordered_map<std::string, Entity> entityPerTag;
    std::unordered_map<int, std::string> tagPerEntity;

    // Entity groups (a set of entities per group name)
    std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup;
    std::unordered_map<int, std::string> groupPerEntity;

    // List of free entity ids that were previously removed
    std::deque<int> freeIds;
};

template <typename TComponrnt>
void System::RequireComponent()
{
    const auto componentId = Component<TComponrnt>::GetId();
    componentSignature.set(componentId);
}

template <typename TComponent, typename... TArgs>
void Registry::AddComponent(Entity entity, TArgs &&...args)
{
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    if (componentId >= static_cast<int>(componentPools.size()))
    {
        componentPools.resize(componentId + 1, nullptr);
    }

    if (!componentPools[componentId])
    {
        std::shared_ptr<Pool<TComponent>> newComponentPool(new Pool<TComponent>());
        componentPools[componentId] = newComponentPool;
    }

    std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

    TComponent newComponent(std::forward<TArgs>(args)...);

    componentPool->Set(entityId, newComponent);

    entityComponentSignatures[entityId].set(componentId);

    Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " + std::to_string(entityId));
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity)
{
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    // Remove the component from the component list for that entity
    std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
    componentPool->Remove(entityId);

    // Set this component signature for that entity to false
    entityComponentSignatures[entityId].set(componentId, false);

    Logger::Log("Component id = " + std::to_string(componentId) + " was removed from entity id " + std::to_string(entityId));
}

template <typename TComponrnt>
bool Registry::HasComponent(Entity entity) const
{
    const auto componentId = Component<TComponrnt>::GetId();
    const auto entityId = entity.GetId();
    return entityComponentSignatures[entityId].test(componentId);
};

template <typename TComponrnt>
TComponrnt &Registry::GetComponent(Entity entity) const
{
    const auto componentId = Component<TComponrnt>::GetId();
    const auto entityId = entity.GetId();
    auto componentPool = std::static_pointer_cast<Pool<TComponrnt>>(componentPools[componentId]);
    return componentPool->Get(entityId);
};

template <typename TSystem, typename... TArgs>
void Registry::AddSystem(TArgs &&...args)
{
    std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
    systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
};

template <typename TSystem>
void Registry::RemoveSystem()
{
    const auto systemId = systems.find(std::type_index(typeid(TSystem)));
    systems.erase(systemId);
};

template <typename TSystem>
bool Registry::HasSystem() const
{
    const auto systemId = systems.find(std::type_index(typeid(TSystem)));
    return systemId != systems.end();
};

template <typename TSystem>
TSystem &Registry::GetSystem() const
{
    const auto systemId = systems.find(std::type_index(typeid(TSystem)));
    return *(std::static_pointer_cast<TSystem>(systemId->second));
};

template <typename TComponrnt, typename... TArgs>
void Entity::AddComponent(TArgs &&...args)
{
    registry->AddComponent<TComponrnt>(*this, std::forward<TArgs>(args)...);
};

template <typename TComponrnt>
void Entity::RemoveComponent()
{
    registry->RemoveComponent<TComponrnt>(*this);
};

template <typename TComponrnt>
bool Entity::HasComponent() const
{
    return registry->HasComponent<TComponrnt>(*this);
};

template <typename TComponrnt>
TComponrnt &Entity::GetComponent() const
{
    return registry->GetComponent<TComponrnt>(*this);
};

#endif