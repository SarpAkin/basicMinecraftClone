#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>
#include <set>

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
    std::unordered_map<Vector2Int,std::set<int>,Hasher<Vector2Int>, Equal<Vector2Int>> requestedChukns;

private:
    void GetChunks();//Gets the generated chunks from tgen.
public:
    // Messages
    Message S_EntitySpawned(EntityID id);
    Message S_PlayerSpawned(EntityID id);
    Message S_LoadChunk(Chunk& c);

    void R_RequestChunk(M_P_ARGS_T);
    //
    S_game(uint16_t port);
    ~S_game();

    void Tick(float deltaT);

    void OnClientJoin(Client& c) override;

    void ProcessMessageCustom(MessageTypes, M_P_ARGS_T) override;
    std::shared_ptr<Entity> GetEntity(EntityID) override;

    std::shared_ptr<Entity> SpawnEntity(std::unique_ptr<Entity> e, Chunk& c);
};

// CPP
