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

struct PointLight
{
    Vector3 pos;
    Vector3 col;
};

struct DirectionalLight
{
    Vector3 dir;
    Vector3 col;
};

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

    int selected_hotbar_slot = 0;
    std::array<Tile, 9> hotbar = {
        Tile::TileMap["stone"],
        Tile::TileMap["sand"],
        Tile::TileMap["dirt"],
        Tile::TileMap["wood"],
        Tile::TileMap["glass"],

    };

    // shadow
    uint32_t gBuffer;
    uint32_t gPosition, gNormal, gAlbedoSpec;

    VertexBuffer quad_vb;
    Shader lightning_shader;

public:
    TestRen() : Renderer(), game(30020, "127.0.0.1")
    {
        player = game.Player.lock();
    }

    void OnScreenResize() override
    {
        proj = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 10000.0f);

        destroy_gbuffers();
        create_gbuffers();

        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        ChunkMeshGPU::bind_atlas();
    }

    void OnStart() override
    {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        GLCALL(glDisable(GL_BLEND));
        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glCullFace(GL_BACK));

        clear_color = {0.3f, 0.4f, 0.8f, 1.0f};

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

        for (int i = 0; i < 9; ++i)
        {
            OnKey_Press_Funcs[GLFW_KEY_1 + i] = [this, i]() { selected_hotbar_slot = i; };
        }

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
                        game.PlaceBlock(hitBlock, hotbar[selected_hotbar_slot]);
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
            t.pos.y -= .02f;
            t.size.y = .02f;

            auto boundry_trim = Vector3(0.1f, 0.0f, 0.1f) * 1.0f;

            t.pos += boundry_trim / 2.0f;
            t.size -= boundry_trim;

            if (pChunk.doesCollide(t).size())
                player->transform.velocity.y += 6.5f;
        };

        InitShadows();

        ChunkMeshGPU::bind_atlas();
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

    void create_gbuffers()
    {
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

        // - position color buffer
        glGenTextures(1, &gPosition);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

        // - normal color buffer
        glGenTextures(1, &gNormal);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

        // - color + specular color buffer
        glGenTextures(1, &gAlbedoSpec);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

        // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        uint32_t attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);

        // create and attach depth buffer (renderbuffer)
        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void destroy_gbuffers()
    {
        uint32_t arr[] = {gPosition, gNormal, gAlbedoSpec};
        GLCALL(glDeleteTextures(3, arr));

        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        GLCALL(glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w));
        GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    void InitShadows()
    {
        create_gbuffers();

            // init quad
            VertexBufferLayout vl;
        vl.Push<float>(3);
        vl.Push<float>(2);

        std::vector<float> verticies = {
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,

            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,

            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,

            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };

        quad_vb = VertexBuffer(verticies, vl);
        lightning_shader =
            Shader(readFile("res/shaders/deffered_light.vert"), readFile("res/shaders/deffered_light.frag"));
    }

    ~TestRen()
    {
        destroy_gbuffers();
    }

    void Draw(double DeltaT)
    {
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        ChunksInRange(
            player->currentChunk,
            [this](Chunk& c, Vector2Int r_pos, Vector2Int f_pos) { ChunkMeshGPU::Draw(c, proj * view, r_pos, *this); },
            [](Chunk& c, Vector2Int r_pos, Vector2Int f_pos) {}, 10);
    }

    void DefferedDraw(double DeltaT)
    {
        // GPass

        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));
        GLCALL(glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w));
        GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        auto a = ChunksInRange(
            player->currentChunk,
            [this](Chunk& c, Vector2Int r_pos, Vector2Int f_pos) { ChunkMeshGPU::DrawG(c, proj * view, r_pos, *this); },
            [](Chunk& c, Vector2Int r_pos, Vector2Int f_pos) {}, 10);

        // lightning
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        GLCALL(glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w));
        GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        GLCALL(glActiveTexture(GL_TEXTURE0));
        GLCALL(glBindTexture(GL_TEXTURE_2D, gPosition));
        GLCALL(glActiveTexture(GL_TEXTURE1));
        GLCALL(glBindTexture(GL_TEXTURE_2D, gNormal));
        GLCALL(glActiveTexture(GL_TEXTURE2));
        GLCALL(glBindTexture(GL_TEXTURE_2D, gAlbedoSpec));

        std::vector<PointLight> p_lights = {{player->transform.pos, {1.0f, 1.0f, 1.0f}}};
        DirectionalLight d_light = {{0.3f, -0.7f, 0.05f}, {0.8f, 0.7f, 0.4f}};

        const int max_point_light = 32;

        int point_light_count = std::min((int)p_lights.size(), max_point_light);

        lightning_shader.Bind();

        lightning_shader.SetUniform3f("p_lights[0].pos", reinterpret_cast<glm::vec3*>(p_lights.data()),
            point_light_count * 2);

        for (int i = 0; i < point_light_count; ++i)
        {
            std::stringstream ss;
            ss << "p_lights[" << i << "]";
            lightning_shader.SetUniform3f(ss.str() + ".pos", p_lights[i].pos);
            lightning_shader.SetUniform3f(ss.str() + ".col", p_lights[i].col);
        }

        lightning_shader.SetUniform3f("dir_light.dir", Vector3(1) - d_light.dir);
        lightning_shader.SetUniform3f("dir_light.col", d_light.col);
        // lightning_shader.SetUniform3f("view_pos_", player->transform.pos);

        lightning_shader.SetUniform1i("gPosition", 0);
        lightning_shader.SetUniform1i("gNormal", 1);
        lightning_shader.SetUniform1i("gAlbedoSpec", 2);

        if (wireframe)
        {
            GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        quad_vb.Bind();

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        if (wireframe)
        {
            wireframe = true;
            GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

    }

    void OnUpdate(double DeltaT) override
    {
        Transform& viewPos = player->transform;

        game.Tick(DeltaT);
        UpdateCamera(DeltaT);

        if (false)
        {
            DefferedDraw(DeltaT);
        }
        else
        {
            Draw(DeltaT);
        }
    }

   
};

#include "net/client.hpp"

int main()
{

    // MEASURE_TIME(std::this_thread::sleep_for(std::chrono::seconds(1)));
    {
        TestRen t = TestRen();
        if (t.Construct(1280, 720))
            t.Start();
    }
    GLCALL(glfwTerminate());
}
