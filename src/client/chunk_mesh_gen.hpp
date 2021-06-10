#pragma once

#include <array>
#include <bitset>
#include <vector>

#include "../common/chunk.hpp"

// this represents 6 contigiuos verticies forming an square
struct VoxelRect
{
    uint32_t m_data[6];

    // components should range between 0-31
    static inline uint32_t compress_vec3int(Vector3Int vec)
    {
        // std::cout << vec.z << '\n';

        return (vec.x << 10) | (vec.y << 5) | (vec.z << 0);
    }

    template <Direction dir>
    void init(Vector3Int block_pos_start, Vector3Int block_pos_end, uint32_t textureID)
    {

        // vertex data layout
        // texture bits 12 , plane bits 2, cornet bits 3, position bits 15 (each di 5 bit)

        static const uint32_t dir_to_plane_bits_table[6] = {
            1 << 18, // up
            1 << 18, // down
            2 << 18, // north
            2 << 18, // south
            0 << 18, // west
            0 << 18, // south
        };

        uint32_t texture_and_plane_bits = (textureID << 20) | dir_to_plane_bits_table[dir];

        // Vector3Int top_left_pos; // top left corner vertex pos
        // Vector3Int bottom_right_pos; // bottom right corner vertex pos

        if constexpr (dir == Direction::up)
        {
            Vector3Int top_left_pos = Vector3Int(block_pos_start.x, block_pos_start.y, block_pos_end.z);
            Vector3Int bottom_right_pos = Vector3Int(block_pos_end.x, block_pos_start.y, block_pos_start.z);

            // clang-format off
            constexpr uint32_t bottom_left_corner_bits  = (0b010 << 15);
            constexpr uint32_t bottom_right_corner_bits = (0b110 << 15);
            constexpr uint32_t top_left_corner_bits     = (0b011 << 15);
            constexpr uint32_t top_right_corner_bits    = (0b111 << 15);
            // clang-format on

            // same code in all
            m_data[0] = texture_and_plane_bits | top_left_corner_bits | compress_vec3int(top_left_pos);
            m_data[1] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[2] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
            m_data[5] = texture_and_plane_bits | bottom_right_corner_bits | compress_vec3int(bottom_right_pos);

            m_data[3] = m_data[2];
            m_data[4] = m_data[1];
        }
        else if constexpr (dir == Direction::down)
        {
            Vector3Int top_left_pos = Vector3Int(block_pos_start.x, block_pos_start.y, block_pos_end.z);
            Vector3Int bottom_right_pos = Vector3Int(block_pos_end.x, block_pos_start.y, block_pos_start.z);

            // clang-format off
            constexpr uint32_t bottom_left_corner_bits  = (0b000 << 15);
            constexpr uint32_t bottom_right_corner_bits = (0b100 << 15);
            constexpr uint32_t top_left_corner_bits     = (0b001 << 15);
            constexpr uint32_t top_right_corner_bits    = (0b101 << 15);
            // clang-format on

            m_data[0] = texture_and_plane_bits | top_left_corner_bits | compress_vec3int(top_left_pos);
            m_data[1] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[2] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
            m_data[5] = texture_and_plane_bits | bottom_right_corner_bits | compress_vec3int(bottom_right_pos);

            m_data[3] = m_data[2];
            m_data[4] = m_data[1];
        }
        else if constexpr (dir == Direction::north)
        {
            Vector3Int top_left_pos = Vector3Int(block_pos_start.x, block_pos_end.y, block_pos_start.z);
            Vector3Int bottom_right_pos = Vector3Int(block_pos_end.x, block_pos_start.y, block_pos_start.z);

            // clang-format off
            constexpr uint32_t bottom_left_corner_bits  = (0b001 << 15);
            constexpr uint32_t bottom_right_corner_bits = (0b101 << 15);
            constexpr uint32_t top_left_corner_bits     = (0b011 << 15);
            constexpr uint32_t top_right_corner_bits    = (0b111 << 15);
            // clang-format on

            m_data[0] = texture_and_plane_bits | top_left_corner_bits | compress_vec3int(top_left_pos);
            m_data[1] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[2] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
            m_data[5] = texture_and_plane_bits | bottom_right_corner_bits | compress_vec3int(bottom_right_pos);

            m_data[3] = m_data[2];
            m_data[4] = m_data[1];
        }
        else if constexpr (dir == Direction::south)
        {
            Vector3Int top_left_pos = Vector3Int(block_pos_start.x, block_pos_end.y, block_pos_start.z);
            Vector3Int bottom_right_pos = Vector3Int(block_pos_end.x, block_pos_start.y, block_pos_start.z);

            // clang-format off
            constexpr uint32_t bottom_left_corner_bits  = (0b000 << 15);
            constexpr uint32_t bottom_right_corner_bits = (0b100 << 15);
            constexpr uint32_t top_left_corner_bits     = (0b010 << 15);
            constexpr uint32_t top_right_corner_bits    = (0b110 << 15);
            // clang-format on

            m_data[0] = texture_and_plane_bits | top_left_corner_bits | compress_vec3int(top_left_pos);
            m_data[1] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[2] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
            m_data[5] = texture_and_plane_bits | bottom_right_corner_bits | compress_vec3int(bottom_right_pos);

            m_data[3] = m_data[2];
            m_data[4] = m_data[1];
        }
        else if constexpr (dir == Direction::east)
        {
            Vector3Int top_left_pos = Vector3Int(block_pos_start.x, block_pos_start.y, block_pos_end.z);
            Vector3Int bottom_right_pos = Vector3Int(block_pos_start.x, block_pos_end.y, block_pos_start.z);

            // clang-format off
            constexpr uint32_t bottom_left_corner_bits  = (0b100 << 15);
            constexpr uint32_t bottom_right_corner_bits = (0b110 << 15);
            constexpr uint32_t top_left_corner_bits     = (0b101 << 15);
            constexpr uint32_t top_right_corner_bits    = (0b111 << 15);
            // clang-format on

            m_data[0] = texture_and_plane_bits | top_left_corner_bits | compress_vec3int(top_left_pos);
            m_data[1] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[2] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
            m_data[5] = texture_and_plane_bits | bottom_right_corner_bits | compress_vec3int(bottom_right_pos);

            m_data[3] = m_data[2];
            m_data[4] = m_data[1];
        }
        else if constexpr (dir == Direction::west)
        {
            Vector3Int top_left_pos = Vector3Int(block_pos_start.x, block_pos_start.y, block_pos_end.z);
            Vector3Int bottom_right_pos = Vector3Int(block_pos_start.x, block_pos_end.y, block_pos_start.z);

            // clang-format off
            constexpr uint32_t bottom_left_corner_bits  = (0b100 << 15);
            constexpr uint32_t bottom_right_corner_bits = (0b110 << 15);
            constexpr uint32_t top_left_corner_bits     = (0b101 << 15);
            constexpr uint32_t top_right_corner_bits    = (0b111 << 15);
            // clang-format on

            m_data[0] = texture_and_plane_bits | top_left_corner_bits | compress_vec3int(top_left_pos);
            m_data[1] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[2] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
            m_data[5] = texture_and_plane_bits | bottom_right_corner_bits | compress_vec3int(bottom_right_pos);

            m_data[3] = m_data[2];
            m_data[4] = m_data[1];
        }
        else
        {
            static_assert(dir < 6, "invalid direction");
        }

        // for(int i = 0;i < 6;++i)
        // {
        //     uint32_t vertex_data = m_data[i];

        //         Vector3Int position;

        //     position.x = (vertex_data >> 10) & 31; // 0b11111
        //     position.y = (vertex_data >> 5) & 31;  // 0b11111
        //     position.z = vertex_data & 31;         // 0b11111

        //     uint corner = (vertex_data >> 15) & 7; // 0b111

        //     // for (int i = 0; i < 3; ++i)
        //     // {
        //     //     position[i] += ((corner >> i) & 1); // 0b1
        //     // }

        //     std::bitset<32> bits = vertex_data;

        //     std::cout << position.x << ' ' << position.y << ' ' << position.z << ' ' << corner << ' ' << bits << '\n';
        // }

        // exit(-1);
    }
};

class GreedyMesher
{

public:
    void create_mesh()
    {

        VoxelRect face;
        face.init<Direction::up>(Vector3Int(0, 0, 0), Vector3Int(0, 0, 0), 0);
    }
};