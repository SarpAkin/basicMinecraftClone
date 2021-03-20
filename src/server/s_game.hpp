#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>
#include <set>
#include <vector>

#include "../common/game.hpp"
#include "../common/net/connectionAcceptor.hpp"
#include "../common/utility.hpp"

#include "TGen.hpp"

struct customClientFields
{
    EntityID entID;
};

class S_game : public Game, private ConnectionAcceptor<customClientFields>
{
private:
    TGen tgen;

    vectorBasedDic<std::weak_ptr<Entity>> Entities;
    
    //chunk request
    std::unordered_map<Vector2Int,std::set<uint32_t>,Hasher<Vector2Int>, Equal<Vector2Int>> requestedChukns;

private:
    void GetChunks();//Gets the generated chunks from tgen.
public:
    // Messages
    Message S_EntitySpawned(EntityID id);
    Message S_EntitySpawned(Entity& e);
    Message S_PlayerSpawned(EntityID id);
    Message S_LoadChunk(Chunk& c);//use send chunks instead of this

    void R_RequestChunk(M_P_ARGS_T);
    void R_EntityMoved(M_P_ARGS_T) override;
    
    
    void OnBlockPlaced(Chunk::TileRef tile,uint32_t ClientID) override;
    //
    S_game(uint16_t port);
    ~S_game();


    void Tick(float deltaT);

    void SendChunk(Vector2Int pos,std::vector<uint32_t> clients);
    void SendChunk(Vector2Int pos,uint32_t client);
    void OnClientJoin(Client& c) override;

    void ProcessMessageCustom(MessageTypes, M_P_ARGS_T) override;

    std::shared_ptr<Entity> GetEntity(EntityID) override;

    std::shared_ptr<Entity> SpawnEntity(std::unique_ptr<Entity> e, Chunk& c);
};

// CPP
