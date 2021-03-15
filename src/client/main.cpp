#include <iostream>
#include <fstream>
#include <memory>
#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include "render/opengl_.hpp"
#include "render/IndexBuffer.hpp"
#include "render/VertexBuffer.hpp"
#include "render/VertexArray.hpp"
#include "render/VertexBufferLayout.hpp"
#include "render/shader.hpp"
#include "render/renderer.hpp"
#include "render/Texture.hpp"

#include "ChunkMeshGPU.hpp"
#include "c_game.hpp"

#include "../common/Physics.hpp"
#include "../common/utility.hpp"
#include "../common/chunk.hpp"
#include "../common/noise.hpp"
#include "../common/game.hpp"
#include "../common/Entity.hpp"

class TestRen : public Renderer
{

    C_game game;
    //ChunkMeshGPU mesh;
    glm::mat4 proj;
    glm::mat4 view;

    std::shared_ptr<Entity> player;
    float pitch = 0;
    float yaw = 0;
    const float speed = 10.0f;
public:

    TestRen()
    : Renderer(),game(30020,"127.0.0.1")
    {
        player = game.Player.lock();
    }

    void DrawEast(Chunk* c, int range, Vector2Int pos)
    {
        if (c && range)
        {
            ChunkMeshGPU::Draw(*c, proj * view,pos);
            DrawEast(c->easternChunk, range - 1, { pos.x + 1,pos.y });
        }
        else
        {
            //game.GenerateChunk(pos + player->currentChunk->pos);
        }
    }

    void DrawWest(Chunk* c, int range, Vector2Int pos)
    {
        if (c && range)
        {
            ChunkMeshGPU::Draw(*c, proj * view,pos);
            DrawWest(c->westernChunk, range - 1, { pos.x - 1,pos.y });
        }
        else
        {
            //game.GenerateChunk(pos + player->currentChunk->pos);
        }
    }
    
    void DrawSouth(Chunk* c, int range, Vector2Int pos)
    {
        if (c && range)
        {
            ChunkMeshGPU::Draw(*c, proj * view,pos);
            DrawSouth(c->southernChunk, range - 1, { pos.x,pos.y - 1 });
            DrawWest(c->westernChunk, range - 1, { pos.x - 1,pos.y });
            DrawEast(c->easternChunk, range - 1, { pos.x + 1,pos.y });
        }
        else
        {
            //game.GenerateChunk(pos +player->currentChunk->pos);
        }
    }

    void DrawNorth(Chunk* c, int range, Vector2Int pos)
    {
        if (c && range)
        {
            ChunkMeshGPU::Draw(*c, proj * view,pos);
            DrawNorth(c->northernChunk, range - 1, { pos.x,pos.y + 1 });
            DrawWest(c->westernChunk, range - 1, { pos.x - 1,pos.y });
            DrawEast(c->easternChunk, range - 1, { pos.x + 1,pos.y });
        }
        else
        {
            //game.GenerateChunk(pos+player->currentChunk->pos);
        }
    }

    void DrawChunksInRange(Chunk* c, int range = 10)
    {
        Vector2Int pos = {0,0};
        if (c)
        {
            ChunkMeshGPU::Draw(*c, proj * view,pos);
            DrawNorth(c->northernChunk, range - 1, { pos.x,pos.y + 1 });
            DrawSouth(c->southernChunk, range - 1, { pos.x,pos.y - 1 });
            DrawWest(c->westernChunk, range - 1, { pos.x - 1,pos.y });
            DrawEast(c->easternChunk, range - 1, { pos.x + 1,pos.y });
        }
        else
        {
            //game.GenerateChunk(pos+player->currentChunk->pos);
        }
    }

    void OnStart() override
    {
        auto player_ = std::make_unique<Entity>();
        player_->transform =  Transform({ .5f,.0f,.5f }, { .8f,2.0f,.8f });
        Transform& viewPos = player_->transform;
        viewPos.velocity = Vector3(0, 0, 0);
        proj = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 10000.0f);

        ChunkMeshGPU::staticInit();

    }

    void UpdateCamera(double DeltaT)
    {
        Transform& viewPos = player->transform;
        pitch += -MouseYRaw;
        yaw += MouseXRaw;

        if (pitch > 89)
            pitch = 89;
        else if (pitch < -89)
            pitch = -89;

        //yaw = fmod(yaw,360.0f);
        Vector3 DirVector;
        DirVector.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        DirVector.y = sin(glm::radians(pitch));
        DirVector.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        Vector3 MoveVector = Vector3(0, 0, 0);
        if (keyMap[GLFW_KEY_A])
            MoveVector.z -= 1;
        if (keyMap[GLFW_KEY_D])
            MoveVector.z += 1;
        if (keyMap[GLFW_KEY_W])
            MoveVector.x += 1;
        if (keyMap[GLFW_KEY_S])
            MoveVector.x -= 1;
        if (keyMap[GLFW_KEY_SPACE])
            viewPos.velocity += 1;
        if (keyMap[GLFW_KEY_LEFT_CONTROL])
            MoveVector.y -= 1;

        Vector2 v2 = rotateVectorIndegrees(Vector2(MoveVector.x, MoveVector.z), yaw);
        if (v2.x != 0 && v2.y != 0)
            v2 = glm::normalize(v2);
        MoveVector.x = v2.x;
        MoveVector.z = v2.y;

        MoveVector = MoveVector * speed;
        viewPos.velocity.x = MoveVector.x;
        viewPos.velocity.z = MoveVector.z;
        viewPos.velocity.y += -10.0f * DeltaT;
        //if(auto& c = game.chunks[Chunk::ToChunkCord(viewPos.pos)])
        //ChunkVSAABB(viewPos, *c, DeltaT);
        applyDrag(viewPos, DeltaT);
        Vector3 chunkposFull = Vector3(player->currentChunk->pos.x * chunk_size , 0 ,player->currentChunk->pos.y * chunk_size);
        //std::cout << viewPos.pos.x << ' '<< viewPos.pos.y << ' ' << viewPos.pos.z << '\n';
        view = glm::lookAt(viewPos.GetMidPoint(), viewPos.GetMidPoint() + DirVector, Vector3(0, 1, 0));
    }

    void OnUpdate(double DeltaT) override
    {
        Transform& viewPos = player->transform;

        //game.Tick(DeltaT);
        UpdateCamera(DeltaT);


        DrawChunksInRange(player->currentChunk,10);
    }
};

#include "net/client.hpp"

int main()
{
    
    TestRen t = TestRen();
    if (t.Construct(1280, 720))
        t.Start();
    
}
