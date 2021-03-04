#include <iostream>
#include <fstream>
#include <memory>
#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include "render/opengl_.h"
#include "render/IndexBuffer.h"
#include "render/VertexBuffer.h"
#include "render/VertexArray.h"
#include "render/VertexBufferLayout.h"
#include "render/shader.h"
#include "render/renderer.h"
#include "render/Texture.h"

#include "ChunkMeshGPU.h"
#include "utility.h"
#include "chunk.h"
#include "noise.h"
#include "game.h"

class TestRen : public Renderer
{

    Game game;
    //ChunkMeshGPU mesh;
    glm::mat4 proj;
    glm::mat4 view;

    Vector3 viewPos;
    float pitch = 90;
    float yaw = 0;
    const float speed = 10.0f;
public:

    void DrawEast(Chunk* c, int range, Vector2Int pos)
    {
        if (c && range)
        {
            ChunkMeshGPU::Draw(*c, proj * view);
            DrawEast(c->easternChunk, range - 1, { pos.x + 1,pos.y});
        }
        else
        {
            game.GenerateChunk(pos);
        }
    }

    void DrawWest(Chunk* c, int range, Vector2Int pos)
    {
        if (c && range)
        {
            ChunkMeshGPU::Draw(*c, proj * view);
            DrawWest(c->westernChunk, range - 1, { pos.x - 1,pos.y});
        }
        else
        {
            game.GenerateChunk(pos);
        }
    }
    void DrawSouth(Chunk* c, int range, Vector2Int pos)
    {
        if (c && range)
        {
            ChunkMeshGPU::Draw(*c, proj * view);
            DrawSouth(c->southernChunk, range - 1, { pos.x,pos.y - 1});
            DrawWest(c->westernChunk, range - 1, { pos.x - 1,pos.y});
            DrawEast(c->easternChunk, range - 1, { pos.x + 1,pos.y});
        }
        else
        {
            game.GenerateChunk(pos);
        }
    }

    void DrawNorth(Chunk* c, int range, Vector2Int pos)
    {
        if (c && range)
        {
            ChunkMeshGPU::Draw(*c, proj * view);
            DrawNorth(c->northernChunk, range - 1, { pos.x,pos.y + 1});
            DrawWest(c->westernChunk, range - 1, { pos.x - 1,pos.y});
            DrawEast(c->easternChunk, range - 1, { pos.x + 1,pos.y});
        }
        else
        {
            game.GenerateChunk(pos);
        }
    }

    void DrawChunksInRange(Chunk* c, Vector2Int pos, int range = 3)
    {
        if (c)
        {
            ChunkMeshGPU::Draw(*c, proj * view);
            DrawNorth(c->northernChunk, range - 1, { pos.x,pos.y + 1});
            DrawSouth(c->southernChunk, range - 1, { pos.x,pos.y - 1});
            DrawWest(c->westernChunk, range - 1, { pos.x - 1,pos.y});
            DrawEast(c->easternChunk, range - 1, { pos.x + 1,pos.y});
        }
        else
        {
            game.GenerateChunk(pos);
        }
    }

    void OnStart() override
    {
        proj = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 200.0f);

        ChunkMeshGPU::staticInit();

        int range = 5;

        for (int x = -range;x < range;++x)
            for (int y = -range;y < range;++y)
            {
                game.GenerateChunk(Vector2Int(x, y));

            }
        /*
    for (int x = -range;x < range;++x)
        for (int y = -range;y < range;++y)
            game.chunks[{x, y}]->GPUMesh = new ChunkMeshGPU(*game.chunks[{x, y}]);*/
            //game.chunks[{1,1}] = nullptr;

            //game.GenerateChunk(Vector2Int(0, 0));
            //game.chunks[{0, 0}]->GPUMesh = new ChunkMeshGPU(*game.chunks[{0, 0}]);
    }

    void UpdateCamera(double DeltaT)
    {
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
            MoveVector.y += 1;
        if (keyMap[GLFW_KEY_LEFT_CONTROL])
            MoveVector.y -= 1;

        Vector2 v2 = rotateVectorIndegrees(Vector2(MoveVector.x, MoveVector.z), yaw);
        if (v2.x != 0 && v2.y != 0)
            v2 = glm::normalize(v2);
        MoveVector.x = v2.x;
        MoveVector.z = v2.y;

        viewPos += MoveVector * (float)DeltaT * speed;
        view = glm::lookAt(viewPos, viewPos + DirVector, Vector3(0, 1, 0));
    }

    void OnUpdate(double DeltaT) override
    {
        UpdateCamera(DeltaT);
        game.Tick();
        //std::cout << "aa" << '\n';
        //game.chunks[{0,0}]->GPUMesh->Draw(proj*view);
        DrawChunksInRange(game.chunks[Chunk::ToChunkCord({viewPos.x,viewPos.z})].get(),Chunk::ToChunkCord({viewPos.x,viewPos.z}), 10);
    }
};


int main()
{
    TestRen t = TestRen();
    if (t.Construct(1280, 720))
        t.Start();

}