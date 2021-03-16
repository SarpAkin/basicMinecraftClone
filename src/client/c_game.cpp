#include "c_game.hpp"

#include <memory>
#include <utility>

#include "../common/Physics.hpp"

C_game::C_game(uint16_t port, const char* ip) : Game(), Client(port, ip)
{
    while (Player.lock() == nullptr)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        Tick(0.002f);
    }
}

void C_game::Tick(float deltaT)
{
    auto Messages = connection->inqueue.GetDeque();
    for (auto& m : Messages)
    {
        ProcessMessages(m,-1);
    }
    if (auto player_ = Player.lock())
    {
        auto& chunkEnt = player_->currentChunk->Entities;
        for (auto it = chunkEnt.begin();it != chunkEnt.end();++it)
            if (*it == player_)
            {
                ChunkVSAABB(it, deltaT);
                break;
            }
    }

    //Request chunks
    connection->Send(ToSendableM(S_RequestChunk(std::move(requestCBuffer))));
    //requestedChunks.clear();
    //
}

void C_game::requestChunk(Vector2Int pos)
{
    //std::cout << "aa\n";
    if(!requestedChunks.contains(pos))
    {
        requestedChunks.emplace(pos);
        requestCBuffer.push_back(pos);
    }
}

std::shared_ptr<Entity> C_game::GetEntity(EntityID id)
{
    return Entities[id].lock();
}

//Messages
void C_game::ProcessMessageCustom(MessageTypes mtype, M_P_ARGS_T)
{
    switch (mtype)
    {
        M_P_CASE(EntitySpawned);
        M_P_CASE(PlayerSpawned);
        M_P_CASE(LoadChunk);
    default:
        std::cerr << "Unsupported message type : " << mtype << " aserting\n";
        break;
    }
}

void C_game::R_LoadChunk(M_P_ARGS_T)
{
    auto c_pos = m.pop_front<Vector2Int>();
    requestedChunks.erase(c_pos);
    auto chunk = std::make_unique<Chunk>();
    m.pop_front(*chunk);
    chunk->Init(chunks); // init chunk
    chunks[c_pos] = std::move(chunk);
}

void C_game::R_EntitySpawned(M_P_ARGS_T)
{
    EntityID id;
    m.pop_front(id);
    auto e = std::make_shared<Entity>();
    if (e->Deserialize(m, this))
    {
        e->currentChunk->Entities.push_back(e);
        Entities[id] = e;
    }
}

void C_game::R_PlayerSpawned(M_P_ARGS_T)
{
    EntityID id;
    m.pop_front(id);
    Player = GetEntity(id);
}

Message C_game::S_RequestChunk(std::vector<Vector2Int> chunk_poses)
{
    Message m;
    m.push_back(MessageTypes::RequestChunk);

    m.push_back(chunk_poses);
    return m;
}