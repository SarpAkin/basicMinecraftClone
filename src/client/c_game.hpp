#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "../common/game.hpp"
#include "net/client.hpp"

class C_game : public Game, private Client
{
private:
    std::unordered_map<EntityID, std::weak_ptr<Entity>> Entities;

public:
    std::weak_ptr<Entity> Player;
public: // func
    C_game(uint16_t port, const char* ip);
    void Tick(float deltaT);

    void R_EntitySpawned(M_P_ARGS_T);
    void R_PlayerSpawned(M_P_ARGS_T);
    void R_LoadChunk(M_P_ARGS_T);

    void ProcessMessageCustom(MessageTypes, M_P_ARGS_T) override;
    std::shared_ptr<Entity> GetEntity(EntityID) override;
};
