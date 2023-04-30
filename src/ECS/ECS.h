#define pragma once

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>

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
        return static_cast<T &> data[index];
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
    Registry() = default;
    ~Registry() = default;

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

    template <typename TSystem, typename... TArgs>
    void AddSystem(TArgs &&...args);
    template <typename TSystem>
    void RemoveSystem();
    template <typename TSystem>
    bool HasSystem() const;
    template <typename TSystem>
    TSystem &GetSystem() const;

    void AddEntityToSystems(Entity entity);

private:
    int numEntities;
    std::vector<IPool *> componentPools;
    std::vector<Signature> entityComponentsSignatures;
    std::unordered_map<std::type_index, System *> systems;

    std::set<Entity> entitiesToCreate;
    std::set<Entity> entitiesToKill;
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
    const auto componentId = GetComponentId<TComponrnt>().GetId();
    const auto entityId = entity.GetId();

    if (componentId >= componentPools.size())
    {
        componentPools.resize(componentId + 1, nullptr);
    }

    if (!componentPools[componentId])
    {
        componentPools[componentId] = new Pool<TComponrnt>();
    }

    Pool<TComponrnt> *componentPool = componentPools[componentId];
    if (entityId >= componentPool->GetSize())
    {
        componentPool->Resize(numEntities);
    }

    TComponrnt newComponent(std::forward<TArgs>(args)...);
    componentPool->Set(entityId, newComponent);
    entityComponentsSignatures[entityId].set(componentId);
};

template <typename TComponrnt>
void Registry::RemoveComponent(Entity entity)
{
    const auto componentId = GetComponentId<TComponrnt>().GetId();
    const auto entityId = entity.GetId();
    entityComponentsSignatures[entityId].set(componentId, false);
};

template <typename TComponrnt>
bool Registry::HasComponent(Entity entity) const
{
    const auto componentId = GetComponentId<TComponrnt>().GetId();
    const auto entityId = entity.GetId();
    return entityComponentsSignatures[entityId].test(componentId);
};

template <typename TSystem, typename... TArgs>
void Registry::AddSystem(TArgs &&...args)
{
    TSystem *newSystem(new TSystem(std::forward<TArgs>(args)...));
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
    return *(static_cast<TSystem*>(systemId->second));
};