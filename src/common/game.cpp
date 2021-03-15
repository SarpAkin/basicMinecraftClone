#include "game.hpp"
#include "Entity.hpp"

#include <cassert>
#include <chrono>

// CPP

void Game::R_EntityMoved(M_P_ARGS_T)
{
    auto e = GetEntity(m.pop_front<EntityID>());
    if (e)
        m.pop_front(e->transform.pos);
}

Message Game::S_EntityMoved(Entity& e)
{
    Message m;
    m.push_back(MessageTypes::EntityMoved);
    m.push_back(e.transform.pos);
    return m;
}

void Game::ProcessMessages(M_P_ARGS_T)
{

    // Get Messaage Type first
    auto mType = m.pop_front<MessageTypes>();
    switch (mType)
    {
        M_P_CASE(EntityMoved);
    default:
        ProcessMessageCustom(mType, M_P_ARGS);
        break;
    }
}

Game::Game()
{
}
