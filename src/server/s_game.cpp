#include "s_game.hpp"

#include <chrono>

//#include "TGen.hpp"

S_game::S_game(uint16_t port) : ConnectionAcceptor::ConnectionAcceptor(port), Game::Game()
{
    
}

void S_game::Tick(float deltaT)
{
    auto TGenOutChunks = tgen.GetChunks();
    for (auto& c : TGenOutChunks)
    {
        auto pos = c->pos;
        
        c->Init(chunks);
        chunks[pos] = std::move(c);
    }
    TGenOutChunks.clear();
    for (auto& chunk : chunks)
    {
        if (chunk.second)
            chunk.second->Tick(deltaT);
    }
}
