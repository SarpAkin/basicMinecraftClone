#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "net/message.hpp"

#include "Entity.hpp"
#include "chunk.hpp"
#include "hasher.hpp"
#include "noise.hpp"
#include "vectors.hpp"

#define M_P_ARGS_T Message& m,int ClientID //Arguments for functions
#define M_P_ARGS m,ClientID//variables in the arguments of the function

#define M_P_CASE(m_enum)                                                                                               \
    case MessageTypes::m_enum:                                                                                         \
        R_##m_enum(M_P_ARGS);                                                                                          \
        break;

enum MessageTypes : uint16_t
{
    EntityMoved,
    EntitySpawned,
    PlayerSpawned,
    LoadChunk,
    RequestChunk,
};

class Game
{
private:
    bool running;

public:
    std::unordered_map<Vector2Int, std::unique_ptr<Chunk>, Hasher<Vector2Int>, Equal<Vector2Int>> chunks;

private:
    void initTerrainGen();
    void genChunks();

protected:
    //Note it moves the message 
    inline std::shared_ptr<Message> ToSendableM(Message m){m.trim();return std::make_shared<Message>(std::move(m));} 

    void ProcessMessages(M_P_ARGS_T);

    virtual void ProcessMessageCustom(MessageTypes,M_P_ARGS_T)=0;

    void R_EntityMoved(M_P_ARGS_T);
    Message S_EntityMoved(Entity& e);

public:
    void GenerateChunk(Vector2Int pos);
    

    virtual std::shared_ptr<Entity> GetEntity(EntityID)=0;

    Game();
    virtual ~Game()=default;
};
