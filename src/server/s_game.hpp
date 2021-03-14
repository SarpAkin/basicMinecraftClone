#pragma once

#include "../common/game.hpp"
#include "../common/net/connectionAcceptor.hpp"
#include "TGen.hpp"
#include <cstdint>

class S_game : public Game, public ConnectionAcceptor
{
private:
    TGen tgen;
public:
    S_game(uint16_t port);
    void Tick(float deltaT);
};

//CPP
