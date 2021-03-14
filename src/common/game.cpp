#include "game.hpp"

#include <chrono>
#include <cassert>


//CPP

Game::~Game()
{

}

Game::Game()
{

}

std::shared_ptr<Entity> GetEntity()
{
    std::cerr << "this method shouldn't be used!\n";
    assert(0);
    return nullptr;
}

std::shared_ptr<Entity> Game::SpawnEntity(std::unique_ptr<Entity> e_, Chunk& c)
{
    std::shared_ptr<Entity> e = std::move(e_);
    EntityID eID;
    if(deletedEntityPositions.size())
    {
        eID = deletedEntityPositions.back();
        deletedEntityPositions.pop_back();
        Entities[eID] = e;
    }
    else
    {
        eID = Entities.size();
        Entities.push_back(e);
    }
    e->entityID = eID;
    e->currentChunk = &c;
    c.Entities.push_back(e);
    return e;
}
