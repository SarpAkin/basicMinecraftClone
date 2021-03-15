#include "s_game.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

//#include "TGen.hpp"

S_game::S_game(uint16_t port) : Game(), ConnectionAcceptor(port)
{
    tgen.GenerateChunk({0, 0});
    while (!chunks[{0, 0}])
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        GetChunks();
    }
    std::cout << "[SERVER] ready!\n";
}

S_game::~S_game()
{
}

void S_game::ProcessMessageCustom(MessageTypes mtype, M_P_ARGS_T)
{
    switch (mtype)
    {

    default:
        std::cerr << "Unsupported message type : " << mtype << " aserting\n";
        break;
    }
}

void S_game::GetChunks()
{
    auto TGenOutChunks = tgen.GetChunks();
    for (auto& c : TGenOutChunks)
    {
        auto pos = c->pos;

        c->Init(chunks);
        chunks[pos] = std::move(c);
    }
}

void S_game::Tick(float deltaT)
{
    GetChunks();
    for (auto& chunk : chunks)
    {
        if (chunk.second)
            chunk.second->Tick(deltaT);
    }
    ConnectionAcceptor::Tick();
}

std::shared_ptr<Entity> S_game::GetEntity(EntityID id)
{
    return Entities[id].lock();
}

std::shared_ptr<Entity> S_game::SpawnEntity(std::unique_ptr<Entity> e_, Chunk& c)
{
    std::shared_ptr<Entity> e = std::move(e_);
    EntityID eID = Entities.push_pack(e);
    c.Entities.push_back(e);
    e->entityID = eID;
    e->currentChunk = &c;
    SendMessageToAll(ToSendableM(S_EntitySpawned(eID)));
    return e;
}

void S_game::OnClientJoin(Client& c)
{
    // Send all entitites
    for (auto& e_ : Entities.items)
    {
        if (auto e = e_.lock())
        {
            SendMessage(ToSendableM(S_EntitySpawned(e->entityID)), c.id);
        }
    }
    //
    Chunk& spawnChunk = *chunks[{0, 0}];
    SendMessage(ToSendableM(S_LoadChunk(spawnChunk)), c.id);
    auto ent = std::make_unique<Entity>();
    ent->transform.pos = {8, 0, 8};
    // find the first air block available
    for (int i = 0; i < max_block_height; ++i)
    {
        if ((Tile)spawnChunk[{ent->transform.pos.x, i, ent->transform.pos.z}] == air)
        {
            ent->transform.pos.y = i + 2;
            break;
        }
    }
    ent->transform.velocity = {0, 0, 0};
    ent->transform.size = {.8f, 1.8f, .8f};
    auto eID = SpawnEntity(std::move(ent), spawnChunk)->entityID;
    c.c_field.entID = eID;
    SendMessage(ToSendableM(S_PlayerSpawned(eID)), c.id);
}

// Messages

Message S_game::S_EntitySpawned(EntityID id)
{
    Message m;
    m.push_back(MessageTypes::EntitySpawned);

    m.push_back(id);
    auto e = GetEntity(id);
    e->Serialize(m);

    return m;
}

Message S_game::S_PlayerSpawned(EntityID id)
{
    Message m;
    m.push_back(MessageTypes::PlayerSpawned);

    m.push_back(id);
    return m;
}

Message S_game::S_LoadChunk(Chunk& c)
{
    Message m;
    m.push_back(MessageTypes::LoadChunk);

    m.push_back(c.pos);
    m.push_back(c);
    return m;
}