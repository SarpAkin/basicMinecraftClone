#include "game.hpp"
#include "Entity.hpp"
#include "vectors.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>

// CPP

// schedular
void Game::Schedular::Tick()
{
    ++time;
    if (scheduledFunctions.size())
        if (time >= scheduledFunctions.begin()->time)
        {
            auto func_to_ex = scheduledFunctions.begin()->f;
            scheduledFunctions.erase(scheduledFunctions.begin());
            func_to_ex();
        }
}

void Game::Schedular::SchedulFunc(std::function<void()> f_, uint64_t time_ahead)
{
    func new_f = {time_ahead + time, f_};
    scheduledFunctions.emplace(new_f);
}

void Game::Schedular::SchedulRepeatingFunc(std::function<void()> f_, uint64_t time_ahead, uint64_t RepeatEvery)
{
    auto repeatingFunc = std::make_shared<std::function<void()>>();
    *repeatingFunc = [=, this]() {
        f_();
        SchedulFunc(*repeatingFunc, RepeatEvery);
    };
    SchedulFunc(*repeatingFunc, time_ahead);
}
//

void Game::R_EntityMoved(M_P_ARGS_T)
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
    }
    else
    {
        std::cerr << "Entity doesn't exist\n";
    }
}

Message Game::S_EntityMoved(Entity& e)
{
    Message m;
    m.push_back(MessageTypes::EntityMoved);

    m.push_back(e.entityID);
    m.push_back(e.transform.pos);
    m.push_back(e.currentChunk->pos);
    return m;
}

void Game::R_BlockPlaced(M_P_ARGS_T)
{
    Vector2Int c_pos;
    Vector3Int b_pos;

    m.pop_front(c_pos);
    m.pop_front(b_pos);
    if(auto& c = chunks[c_pos])
    {
        auto tRef = (*c)[b_pos];
        tRef = m.pop_front<Tile>();
        OnBlockPlaced(tRef,ClientID);
    }
    else
    {
        std::cerr << "client " << ClientID << "send invalid chunk pos!\n";
    }
}

Message Game::S_BlockPlaced(Chunk::TileRef tile)
{
    Message m;
    m.push_back(MessageTypes::BlockPlaced);

    m.push_back(tile.chunk.pos);
    m.push_back(tile.pos);
    m.push_back((Tile)tile);

    return m;
}

void Game::ProcessMessages(M_P_ARGS_T)
{

    // Get Messaage Type first
    auto mType = m.pop_front<MessageTypes>();
    switch (mType)
    {
        M_P_CASE(EntityMoved);
        M_P_CASE(BlockPlaced);
    default:
        ProcessMessageCustom(mType, M_P_ARGS);
        break;
    }
}

Game::Game()
{
}

void Game::OnBlockPlaced(Chunk::TileRef tile,uint32_t ClientID)
{

}