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
    ChunkMeshGPU mesh;
    glm::mat4 proj;
    glm::mat4 view;

    Vector3 viewPos;
    float pitch = 90;
    float yaw = 0;
    const float speed = 10.0f;
public:
    TestRen()
    {

    }
    void OnStart() override
    {
        proj = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);

        game.GenerateChunk(Vector2Int(0, 0));
        ChunkMeshGPU::staticInit();
        mesh = ChunkMeshGPU(*game.chunks[{0, 0}]);

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
        mesh.Draw(proj * view);


    }
};


int main()
{
    TestRen t = TestRen();
    if (t.Construct())
        t.Start();

}