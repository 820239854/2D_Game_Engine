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

    template <typename TComponrnt, typename... TArgs>
    void AddComponent(TArgs &&...args);
    template <typename TComponrnt>
    void RemoveComponent();
    template <typename TComponrnt>
    bool HasComponent() const;
    template <typename TComponrnt>
    TComponrnt &GetComponent() const;

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
    std::vector<Entity> GetEntities() const;
    const Signature GetComponentsSignature() const;

    template <typename TComponrnt>
    void RequireComponent();

private:
    Signature componentSignature;
    std::vector<Entity> entities;
};

class IPool
{
public:
    virtual ~IPool(){};
};

template <typename T>
class Pool : public IPool
{
public:
    Pool(int size = 100)
    {
        data.reserve(size);
    }

    virtual ~Pool() = default;

    bool isEmpty() const
    {
        return data.empty();
    }

    int GetSize() const
    {
        return data.size();
    }

    void Resize(int size)
    {
        data.resize(size);
    }

    void Clear()
    {
        data.clear();
    }

    void Add(T object)
    {
        data.push_back(object);
    }

    void Set(int index, T object)
    {
        data[index] = object;
    }

    T &Get(int index)
    {
        return static_cast<T &>(data[index]);
    }

    T &operator[](int index)
    {
        return data[index];
    }

private:
    std::vector<T> data;
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
    std::vector<Signature> entityComponentsSignatures;
    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

    std::set<Entity> entitiesToCreate;
    std::set<Entity> entitiesToBeKilled;

    // List of free entity ids that were previously removed
    std::deque<int> freeIds;
};

template <typename TComponrnt>
void System::RequireComponent()
{
    const auto componentId = Component<TComponrnt>::GetId();
    componentSignature.set(componentId);
}

template <typename TComponrnt, typename... TArgs>
void Registry::AddComponent(Entity entity, TArgs &&...args)
{
    const auto componentId = Component<TComponrnt>::GetId();
    const auto entityId = entity.GetId();

    if (componentId >= static_cast<int>(componentPools.size()))
    {
        componentPools.resize(componentId + 1, nullptr);
    }

    if (!componentPools[componentId])
    {
        componentPools[componentId] = std::make_shared<Pool<TComponrnt>>();
    }

    std::shared_ptr<Pool<TComponrnt>> componentPool = std::static_pointer_cast<Pool<TComponrnt>>(componentPools[componentId]);
    if (entityId >= componentPool->GetSize())
    {
        componentPool->Resize(numEntities);
    }

    TComponrnt newComponent(std::forward<TArgs>(args)...);
    componentPool->Set(entityId, newComponent);
    entityComponentsSignatures[entityId].set(componentId);

    Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " + std::to_string(entityId));
};

template <typename TComponrnt>
void Registry::RemoveComponent(Entity entity)
{
    const auto componentId = Component<TComponrnt>::GetId();
    const auto entityId = entity.GetId();
    entityComponentsSignatures[entityId].set(componentId, false);

    Logger::Log("Component id = " + std::to_string(componentId) + " was removed from entity id " + std::to_string(entityId));
};

template <typename TComponrnt>
bool Registry::HasComponent(Entity entity) const
{
    const auto componentId = Component<TComponrnt>::GetId();
    const auto entityId = entity.GetId();
    return entityComponentsSignatures[entityId].test(componentId);
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