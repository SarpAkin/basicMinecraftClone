#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>
#include <vector>

#include "net/message.hpp"

#include "Entity.hpp"
#include "chunk.hpp"
#include "hasher.hpp"
#include "noise.hpp"
#include "vectors.hpp"

#define M_P_ARGS_T Message &m, int ClientID // Arguments for functions
#define M_P_ARGS m, ClientID                // variables in the arguments of the function

#define M_P_CASE(m_enum)                                                                                               \
    case MessageTypes::m_enum:                                                                                         \
        R_##m_enum(M_P_ARGS);                                                                                          \
        break;

enum MessageTypes : uint16_t
{
    EntityMoved,
    EntitySpawned,
    EntityDestroyed,
    PlayerSpawned,
    LoadChunk,
    RequestChunk,
    BlockPlaced,
};

class Game
{
protected:
    class Schedular
    {
    private:
        struct func
        {
            uint64_t time;
            std::function<void()> f;
            inline bool operator<(const func& o)const
            {
                return time < o.time;
            }
        };
        uint64_t time = 0;
        std::set<func> scheduledFunctions;

    public:
        void Tick();
        void SchedulFunc(std::function<void()> f_,uint64_t time_ahead);
        void SchedulRepeatingFunc(std::function<void()> f_,uint64_t time_ahead,std::uint64_t repeatEvery);
    }schedular;

private:
    bool running;

public:
    std::unordered_map<Vector2Int, std::unique_ptr<Chunk>, Hasher<Vector2Int>, Equal<Vector2Int>> chunks;

private:
    void initTerrainGen();
    void genChunks();

protected:
    // Note it moves the message
    inline std::shared_ptr<Message> ToSendableM(Message m)
    {
        m.trim();
        return std::make_shared<Message>(std::move(m));
    }

    void ProcessMessages(M_P_ARGS_T);

    virtual void ProcessMessageCustom(MessageTypes, M_P_ARGS_T) = 0;

    void R_BlockPlaced(M_P_ARGS_T);
    virtual void R_EntityMoved(M_P_ARGS_T);
    void R_EntityDestroyed(M_P_ARGS_T);
    
    
    Message S_BlockPlaced(Chunk::TileRef tile);
    Message S_EntityMoved(Entity& e);
    Message S_EntityDestroyed(EntityID id);

    virtual void OnBlockPlaced(Chunk::TileRef tile,uint32_t ClientID);
public:
    void GenerateChunk(Vector2Int pos);

    virtual std::shared_ptr<Entity> GetEntity(EntityID) = 0;
    virtual void DestroyEntity(EntityID id)=0;

    Game();
    virtual ~Game() = default;
};
