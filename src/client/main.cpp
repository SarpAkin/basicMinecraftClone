#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

#include <glm/gtc/matrix_transform.hpp>
#include <thread>

#include "render/IndexBuffer.hpp"
#include "render/Texture.hpp"
#include "render/VertexArray.hpp"
#include "render/VertexBuffer.hpp"
#include "render/VertexBufferLayout.hpp"
#include "render/opengl_.hpp"
#include "render/renderer.hpp"
#include "render/shader.hpp"

#include "ChunkMeshGPU.hpp"
#include "c_game.hpp"

#include "../common/Entity.hpp"
#include "../common/Physics.hpp"
#include "../common/chunk.hpp"
#include "../common/game.hpp"
#include "../common/noise.hpp"
#include "../common/utility.hpp"

class TestRen : public Renderer
{

    C_game game;
    // ChunkMeshGPU mesh;
    glm::mat4 proj;
    glm::mat4 view;
    Vector3 DirVector;

    std::shared_ptr<Entity> player;

    float pitch = 0;
    float yaw = 0;
    const float speed = 10.0f;

    bool wireframe = false;

public:
    TestRen() : Renderer(), game(30020, "127.0.0.1")
    {
        player = game.Player.lock();
    }

    void OnScreenResize() override
    {
        proj = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 10000.0f);
    }

    void OnStart() override
    {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glCullFace(GL_BACK));

        auto player_ = std::make_unique<Entity>();
        player_->transform = Transform({.5f, .0f, .5f}, {.8f, 2.0f, .8f});
        Transform& viewPos = player_->transform;
        viewPos.velocity = Vector3(0, 0, 0);
        proj = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 10000.0f);

        ChunkMeshGPU::staticInit();
        Entity::StaticInit();

        OnMB_Press_Funcs[GLFW_MOUSE_BUTTON_LEFT] = [this]() {
            if (!isCursorLocked)
            {
                // if cursor isn't locked lock cursor and do nothing else
                LockCursor();
                return;
            }

            Vector3Int hitPos;
            Vector3Int facing;
            Vector3 pPos = player->transform.GetMidPoint();
            Chunk& pChunk = *(player->currentChunk);
            if (pChunk.RayCast(pPos, pPos + (DirVector * 20.0f), hitPos, facing))
            {
                auto hitBlock = pChunk[hitPos];
                game.PlaceBlock(hitBlock, air);
            }
        };

        OnMB_Press_Funcs[GLFW_MOUSE_BUTTON_RIGHT] = [this]() {
            Vector3Int hitPos;
            Vector3Int facing;
            Vector3 pPos = player->transform.GetMidPoint();
            Chunk& pChunk = *(player->currentChunk);
            if (pChunk.RayCast(pPos, pPos + (DirVector * 20.0f), hitPos, facing))
            {
                auto hitBlock = pChunk[hitPos + facing];
                if (hitBlock.pos.y < max_block_height && hitBlock.pos.y >= 0)
                {
                    Transform blockT;
                    blockT.pos = (Vector3)hitBlock.pos - Vector3(.5f, .5f, .5f);
                    blockT.size = Vector3(1, 1, 1);
                    if (!AABBCheck(player->transform, blockT))
                        game.PlaceBlock(hitBlock, sand);
                }
            }
        };

        OnKey_Press_Funcs[GLFW_KEY_ESCAPE] = [this]() { UnlockCursor(); };

        OnKey_Press_Funcs[GLFW_KEY_Y] = [this]() {
            if (wireframe)
            {
                wireframe = false;
                GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }
            else
            {
                wireframe = true;
                GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            }
        };

        OnKey_Press_Funcs[GLFW_KEY_SPACE] = [this]() {
            Chunk& pChunk = *(player->currentChunk);
            Transform t;
            t = player->transform;
            t.pos.y -= .2f;
            t.size.y = .2f;
            if (pChunk.doesCollide(t).size())
                player->transform.velocity.y += 10;
        };
    }

    void UpdateCamera(double DeltaT)
    {
        game.Player.lock()->InitMesh();
        Transform& viewPos = player->transform;
        pitch += -MouseYRaw;
        yaw += MouseXRaw;

        if (pitch > 89)
            pitch = 89;
        else if (pitch < -89)
            pitch = -89;

        // yaw = fmod(yaw,360.0f);
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

        Vector2 v2 = rotateVectorIndegrees(Vector2(MoveVector.x, MoveVector.z), yaw);
        if (v2.x != 0 && v2.y != 0)
            v2 = glm::normalize(v2);
        MoveVector.x = v2.x;
        MoveVector.z = v2.y;

        MoveVector = MoveVector * speed;
        viewPos.velocity.x = MoveVector.x;
        viewPos.velocity.z = MoveVector.z;
        viewPos.velocity.y += -10.0f * DeltaT;
        // if(auto& c = game.chunks[Chunk::ToChunkCord(viewPos.pos)])
        // ChunkVSAABB(viewPos, *c, DeltaT);
        applyDrag(viewPos, DeltaT);
        Vector3 chunkposFull =
            Vector3(player->currentChunk->pos.x * chunk_size, 0, player->currentChunk->pos.y * chunk_size);
        // std::cout << viewPos.pos.x << ' '<< viewPos.pos.y << ' ' << viewPos.pos.z << '\n';
        view = glm::lookAt(viewPos.GetMidPoint(), viewPos.GetMidPoint() + DirVector, Vector3(0, 1, 0));
    }

    void OnUpdate(double DeltaT) override
    {
        Transform& viewPos = player->transform;

        game.Tick(DeltaT);
        UpdateCamera(DeltaT);

        ChunksInRange(
            player->currentChunk,
            [this](Chunk& c, Vector2Int r_pos, Vector2Int f_pos) { ChunkMeshGPU::Draw(c, proj * view, r_pos, *this); },
            [](Chunk& c, Vector2Int r_pos, Vector2Int f_pos) {}, 10);
    }
};

#include "net/client.hpp"

int main()
{
    std::cout << std::hex << glGetError() << std::dec << '\n';

    // MEASURE_TIME(std::this_thread::sleep_for(std::chrono::seconds(1)));
    {
        TestRen t = TestRen();
        if (t.Construct(1280, 720))
            t.Start();
    }
    GLCALL(glfwTerminate());
}
