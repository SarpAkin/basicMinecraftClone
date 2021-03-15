#include "c_game.hpp"
#include <memory>

C_game::C_game(uint16_t port, const char* ip) : Game(), Client(port, ip)
{
    std::cout << "aaaaaaa\n";
    while (Player.lock() == nullptr)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        Tick(0.002f);
    }
    std::cout << "baaaaaaa\n";
}

void C_game::Tick(float deltaT)
{
    auto Messages = connection->inqueue.GetDeque();
    for(auto& m : Messages)
    {
        std::cout << "got m\n";
        ProcessMessages(m);
    }
}

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

std::shared_ptr<Entity> C_game::GetEntity(EntityID id)
{
    return Entities[id].lock();
}

void C_game::R_LoadChunk(M_P_ARGS_T)
{
    auto c_pos = m.pop_front<Vector2Int>();
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