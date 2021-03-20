#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "../common/game.hpp"
#include "net/client.hpp"

class C_game : public Game, private Client
{
private:
    std::unordered_map<EntityID, std::weak_ptr<Entity>> Entities;

    std::unordered_set<Vector2Int,Hasher<Vector2Int>> requestedChunks;
    std::vector<Vector2Int> requestCBuffer;
public:
    std::weak_ptr<Entity> Player;

protected: // func
    // Message
    void R_EntitySpawned(M_P_ARGS_T);
    void R_PlayerSpawned(M_P_ARGS_T);
    void R_LoadChunk(M_P_ARGS_T);

    Message S_RequestChunk(std::vector<Vector2Int> chunk_poses);

    void ProcessMessageCustom(MessageTypes, M_P_ARGS_T) override;
    //

    void OnBlockPlaced(Chunk::TileRef tile,uint32_t ClientID) override;
public:
    C_game(uint16_t port, const char* ip);
    void Tick(float deltaT);

    void requestChunk(Vector2Int pos);

    std::shared_ptr<Entity> GetEntity(EntityID) override;

    inline void PlaceBlock(Chunk::TileRef& tile,Tile block)
    {
        tile = block;
        connection->Send(ToSendableM(S_BlockPlaced(tile)));
        tile.chunk.blockMeshUpdate(tile.pos);
    }
};
