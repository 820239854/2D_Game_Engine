#define pragma once

#include <bitset>
#include <vector>

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

private:
    int numEntities;
    std::vector<IPool *> componentPools;
};

template <typename TComponrnt>
void System::RequireComponent()
{
    const auto componentId = Component<TComponrnt>::GetId();
    componentSignature.set(componentId);
}