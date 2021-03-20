#include "s_game.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

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

void S_game::GetChunks()
{
    auto TGenOutChunks = tgen.GetChunks();
    for (auto& c : TGenOutChunks)
    {
        auto pos = c->pos;

        c->Init(chunks);
        auto clientsToSend = std::vector<uint32_t>(requestedChukns[pos].begin(),requestedChukns[pos].end());
        requestedChukns.erase(pos);
        chunks[pos] = std::move(c);
        SendChunk(pos,std::move(clientsToSend));
    }
}

void S_game::Tick(float deltaT)
{
    GetChunks();
    // Tick chunks
    for (auto& chunk : chunks)
    {
        if (chunk.second)
            chunk.second->Tick(deltaT);
    }
    //

    // handle messages
    for (auto& c : clients)
    {
        if (c.connection)
        {
            auto Messages = c.connection->inqueue.GetDeque();
            for (auto& m : Messages)
            {
                ProcessMessages(m, c.id);
            }
        }
    }
    //
    schedular.Tick();
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

void S_game::SendChunk(Vector2Int pos, std::vector<uint32_t> clients)
{
    if (auto& c = chunks[pos])
    {
        std::vector<std::shared_ptr<const Message>> messagesToSend;
        messagesToSend.reserve(c->Entities.size() + 1); // reserve messages for entities and chunk
        messagesToSend.push_back(ToSendableM(S_LoadChunk(*c)));
        for (auto& e : c->Entities)
        {
            messagesToSend.push_back(ToSendableM(S_EntitySpawned(*e)));
        }
        SendMessage(std::move(messagesToSend), std::move(clients));
    }
}

void S_game::SendChunk(Vector2Int pos, uint32_t clients)
{
    if (auto& c = chunks[pos])
    {
        std::vector<std::shared_ptr<const Message>> messagesToSend;
        messagesToSend.reserve(c->Entities.size() + 1); // reserve messages for entities and chunk
        messagesToSend.push_back(ToSendableM(S_LoadChunk(*c)));
        for (auto& e : c->Entities)
        {
            messagesToSend.push_back(ToSendableM(S_EntitySpawned(*e)));
        }
        SendMessage(std::move(messagesToSend), clients);
    }
}

void S_game::OnClientJoin(Client& c)
{
    // Send the chunk first so entities can be spawned
    Chunk& spawnChunk = *chunks[{0, 0}];

    SendChunk({0, 0}, c.id);

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
//Events

void S_game::OnBlockPlaced(Chunk::TileRef tile,uint32_t ClientID)
{
    SendMessageToAll(ToSendableM(S_BlockPlaced(tile)),ClientID);
}

// Messages

void S_game::ProcessMessageCustom(MessageTypes mtype, M_P_ARGS_T)
{
    switch (mtype)
    {
        M_P_CASE(RequestChunk);
    default:
        std::cerr << "Unsupported message type : " << mtype << " aserting\n";
        break;
    }
}

Message S_game::S_EntitySpawned(EntityID id)
{
    Message m;
    m.push_back(MessageTypes::EntitySpawned);

    auto e = GetEntity(id);
    m.push_back(id);
    e->Serialize(m);

    return m;
}

Message S_game::S_EntitySpawned(Entity& e)
{
    Message m;
    m.push_back(MessageTypes::EntitySpawned);

    m.push_back(e.entityID);
    e.Serialize(m);

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

void S_game::R_RequestChunk(M_P_ARGS_T)
{
    std::vector<Vector2Int> chunksposes;
    m.pop_front(chunksposes);
    for (auto pos : chunksposes)
    {
        // std::cout << pos.x << ' ' << pos.y << '\n';//
        if (auto& c = chunks[pos])
        {
            SendChunk(pos, ClientID);
        }
        else
        {

            if (!requestedChukns[pos].size())
                tgen.GenerateChunk(pos);
            requestedChukns[pos].emplace(ClientID);
        }
    }
}

void S_game::R_EntityMoved(M_P_ARGS_T)
{
    auto e = GetEntity(m.pop_front<EntityID>());
    if (e)
    {
        m.pop_front(e->transform.pos);
        Vector2Int c_pos;
        m.pop_front(c_pos);
        if (e->currentChunk->pos != c_pos)
        {
            e->currentChunk->MoveEntity(e->currentChunk->GetEntityIt(e), *chunks[c_pos]);
        }
        SendMessageToAll(ToSendableM(S_EntityMoved(*e)), ClientID);
    }
    else
    {
        std::cerr << "Entity doesn't exist\n";
    }
}

