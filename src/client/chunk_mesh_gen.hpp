#pragma once

#include <array>
#include <bitset>
#include <type_traits>
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
    VoxelRect(Vector3Int block_pos_start, Vector3Int block_pos_end, uint32_t textureID,
        std::integral_constant<Direction, dir> dir_)
    {

        // vertex data layout
        // texture bits 12 , plane bits 2, cornet bits 3, position bits 15 (each di 5 bit)

        static const uint32_t dir_to_plane_bits_table[6] = {
            1 << 18, // up
            1 << 18, // down
            2 << 18, // north
            2 << 18, // south
            0 << 18, // east
            0 << 18, // west
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
            m_data[2] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[1] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
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
            m_data[2] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[1] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
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
            m_data[2] = texture_and_plane_bits | top_right_corner_bits | compress_vec3int(block_pos_end);

            m_data[1] = texture_and_plane_bits | bottom_left_corner_bits | compress_vec3int(block_pos_start);
            m_data[5] = texture_and_plane_bits | bottom_right_corner_bits | compress_vec3int(bottom_right_pos);

            m_data[3] = m_data[2];
            m_data[4] = m_data[1];
        }
        else if constexpr (dir == Direction::west)
        {
            Vector3Int top_left_pos = Vector3Int(block_pos_start.x, block_pos_start.y, block_pos_end.z);
            Vector3Int bottom_right_pos = Vector3Int(block_pos_start.x, block_pos_end.y, block_pos_start.z);

            // clang-format off
            constexpr uint32_t bottom_left_corner_bits  = (0b000 << 15);
            constexpr uint32_t bottom_right_corner_bits = (0b010 << 15);
            constexpr uint32_t top_left_corner_bits     = (0b001 << 15);
            constexpr uint32_t top_right_corner_bits    = (0b011 << 15);
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
            static_assert(dir < 6, "invalid Direction");
        }

        // for (int i = 0; i < 6; ++i)
        // {
        //     uint32_t vertex_data = m_data[i];

        //     using namespace glm;

        //     vec4 position;

        //     position.x = float((vertex_data >> 10) & 31); // 0b11111
        //     position.y = float((vertex_data >> 05) & 31); // 0b11111
        //     position.z = float((vertex_data >> 00) & 31); // 0b11111
        //     position.w = 1.0;

        //     uint corner = (vertex_data >> 15) & 7; // 0b111

        //     position.x += float((corner >> 2) & 1);
        //     position.y += float((corner >> 1) & 1);
        //     position.z += float((corner >> 0) & 1);

        //     const uint first_comp[4] = {1, 0, 0, 0};
        //     const uint second_comp[4] = {2, 2, 1, 1};

        //     uint plane_bits = (corner >> 17) & 3; // 0b11

        //     vec2 v_TexCord = vec2(position[first_comp[plane_bits]], position[second_comp[plane_bits]]);
        //     // a block face ranges in -0.5 to 0.5
        //     position += vec4(-0.5, -0.5, -0.5, 0.0);

        //     std::bitset<32> bits = vertex_data;

        //     std::cout << v_TexCord.x << ' ' << v_TexCord.y << ' ' << std::bitset<2>(plane_bits) << ' '
        //               << bits << '\n';
        // }

        // exit(-1);
    }
};

class GreedyMesher
{

public:
    static std::vector<VoxelRect> create_inner_mesh(const std::array<Tile, chunk_volume>& tiles)
    {

        VoxelRect face(Vector3Int(0, 0, 0), Vector3Int(0, 0, 0), 0, std::integral_constant<Direction, Direction::up>());

        std::array<uint16_t, chunk_area> plane;
        std::vector<VoxelRect> mesh;
        mesh.reserve(1000);

        for (int i = 0; i < chunk_size - 1; ++i)
        {
            create_plane<Direction ::up>(tiles, plane, i);
            plane_to_mesh<Direction::up>(plane, mesh, i);

            create_plane<Direction ::north>(tiles, plane, i);
            plane_to_mesh<Direction::north>(plane, mesh, i);

            create_plane<Direction ::east>(tiles, plane, i);
            plane_to_mesh<Direction::east>(plane, mesh, i);
        }

        for (int i = 1; i < chunk_size; ++i)
        {
            create_plane<Direction ::down>(tiles, plane, i);
            plane_to_mesh<Direction::down>(plane, mesh, i);

            create_plane<Direction ::south>(tiles, plane, i);
            plane_to_mesh<Direction::south>(plane, mesh, i);

            create_plane<Direction ::west>(tiles, plane, i);
            plane_to_mesh<Direction::west>(plane, mesh, i);
        }

        // std::cout << mesh.size() << '\n';

        return mesh;
    }

    static void append_outer_mesh(const Chunk& c, int vertical_index, std::vector<VoxelRect>& mesh_to_append)
    {
        assert(c.grid[vertical_index] && "can't append from null chunk");

        const std::array<Tile, chunk_volume>& tiles = *c.grid[vertical_index];
        std::array<uint16_t, chunk_area> plane;

        {
            const std::array<Tile, chunk_volume>* other_ptr = &Chunk::air_vertical_chunk;

            if (vertical_index < vertical_chunk_count - 1)
                if (auto& other_ptr_ = c.grid[vertical_index + 1])
                    other_ptr = other_ptr_.get();

            create_outer_plane<Direction::up>(tiles, *other_ptr, plane);
            plane_to_mesh<Direction::up>(plane, mesh_to_append, chunk_size - 1);
        }

        {
            const std::array<Tile, chunk_volume>* other_ptr = &Chunk::air_vertical_chunk;

            if (vertical_index > 0)
                if (auto& other_ptr_ = c.grid[vertical_index - 1])
                    other_ptr = other_ptr_.get();

            create_outer_plane<Direction::down>(tiles, *other_ptr, plane);
            plane_to_mesh<Direction::down>(plane, mesh_to_append, 0);
        }

        {
            const std::array<Tile, chunk_volume>* other_ptr = &Chunk::air_vertical_chunk;

            if (c.northernChunk)
                if (auto& other_ptr_ = c.northernChunk->grid[vertical_index])
                    other_ptr = other_ptr_.get();

            create_outer_plane<Direction::north>(tiles, *other_ptr, plane);
            plane_to_mesh<Direction::north>(plane, mesh_to_append, chunk_size - 1);
        }

        {
            const std::array<Tile, chunk_volume>* other_ptr = &Chunk::air_vertical_chunk;

            if (c.southernChunk)
                if (auto& other_ptr_ = c.southernChunk->grid[vertical_index])
                    other_ptr = other_ptr_.get();

            create_outer_plane<Direction::south>(tiles, *other_ptr, plane);
            plane_to_mesh<Direction::south>(plane, mesh_to_append, 0);
        }

        {
            const std::array<Tile, chunk_volume>* other_ptr = &Chunk::air_vertical_chunk;

            if (c.easternChunk)
                if (auto& other_ptr_ = c.easternChunk->grid[vertical_index])
                    other_ptr = other_ptr_.get();

            create_outer_plane<Direction::east>(tiles, *other_ptr, plane);
            plane_to_mesh<Direction::east>(plane, mesh_to_append, chunk_size - 1);
        }

        {
            const std::array<Tile, chunk_volume>* other_ptr = &Chunk::air_vertical_chunk;

            if (c.westernChunk)
                if (auto& other_ptr_ = c.westernChunk->grid[vertical_index])
                    other_ptr = other_ptr_.get();

            create_outer_plane<Direction::west>(tiles, *other_ptr, plane);
            plane_to_mesh<Direction::west>(plane, mesh_to_append, 0);
        }
    }

private:
    struct Group
    {
        Vector2Int start;
        Vector2Int end;
        uint16_t textureID;
    };

    template <Direction dir>
    static void append_mesh_from_groups(std::vector<Group>& gruops, std::vector<VoxelRect>& mesh_to_append,
        int plane_index)
    {
        for (Group& group : gruops)
        {
            Vector3Int start;
            Vector3Int end;

            if constexpr (dir == Direction::up || dir == Direction::down)
            {
                start = {group.start.x, plane_index, group.start.y};
                end = {group.end.x, plane_index, group.end.y};
            }
            else if constexpr (dir == Direction::north || dir == Direction::south)
            {
                start = {group.start.x, group.start.y, plane_index};
                end = {group.end.x, group.end.y, plane_index};
            }
            else if constexpr (dir == Direction::east || dir == Direction::west)
            {
                start = {plane_index, group.start.x, group.start.y};
                end = {plane_index, group.end.x, group.end.y};
            }

            mesh_to_append.emplace_back(start, end, group.textureID, std::integral_constant<Direction, dir>());
        }
    }

    template <Direction dir>
    static void plane_to_mesh(std::array<uint16_t, chunk_area>& plane, std::vector<VoxelRect>& mesh_to_append,
        int plane_index)
    {

        std::vector<Group> previous_groups;
        std::vector<Group> current_groups;
        current_groups.reserve(10);
        previous_groups.reserve(10);

        for (int y = 0; y < chunk_size; ++y)
        {
            int y_offset = y * chunk_size;
            Group current{{0, y}, {}, 0};
            for (int x = 0; x < chunk_size; ++x)
            {
                if (current.textureID != plane[y_offset + x])
                {
                    current.end = {x - 1, y};
                    if (current.textureID)
                    {
                        for (auto it = previous_groups.begin(); it != previous_groups.end(); ++it)
                        {
                            Group other = *it;
                            if (current.textureID == other.textureID && current.start.x == other.start.x &&
                                current.end.x == other.end.x)
                            {
                                current.start.y = other.start.y;
                                previous_groups.erase(it);
                                break;
                            }
                        }
                        current_groups.push_back(current);
                    }

                    // write new content to current
                    current = {{x, y}, {}, plane[y_offset + x]};
                }
            }

            if (current.textureID)
            {
                current.end = {chunk_size - 1, y};
                for (auto it = previous_groups.begin(); it != previous_groups.end(); ++it)
                {
                    Group other = *it;
                    if (current.textureID == other.textureID && current.start.x == other.start.x &&
                        current.end.x == other.end.x)
                    {
                        current.start.y = other.start.y;
                        previous_groups.erase(it);
                        break;
                    }
                }
                current_groups.push_back(current);
            }

            append_mesh_from_groups<dir>(previous_groups, mesh_to_append, plane_index);

            std::swap(current_groups, previous_groups);
            current_groups.resize(0);
        }

        append_mesh_from_groups<dir>(previous_groups, mesh_to_append, plane_index);
    }

    struct ConstantVals
    {
        int next_slice_offset;
        int plane_x_to_chunk_offset;
        int plane_y_to_chunk_offset;
    };

    template <Direction dir>
    static ConstantVals get_constants()
    {
        if constexpr (dir == Direction::up || dir == Direction::down)
        {
            return {dir == Direction::up ? chunk_size : -chunk_size, 1, chunk_area};
        }
        else if constexpr (dir == Direction::north || dir == Direction::south)
        {
            return {dir == Direction::north ? chunk_area : -chunk_area, 1, chunk_size};
        }
        else if constexpr (dir == Direction::east || dir == Direction::west)
        {
            return {dir == Direction::east ? 1 : -1, chunk_size, chunk_area};
        }
    }

    template <Direction dir>
    static void create_outer_plane(const std::array<Tile, chunk_volume>& tiles,
        const std::array<Tile, chunk_volume>& other_tiles, std::array<uint16_t, chunk_area>& plane)
    {
        const size_t plane_slice_index = chunk_size - 1;

        const ConstantVals constants = get_constants<dir>();

        const int next_slice_offset = constants.next_slice_offset;
        const int plane_x_to_chunk_offset = constants.plane_x_to_chunk_offset;
        const int plane_y_to_chunk_offset = constants.plane_y_to_chunk_offset;

        const int slice_tile_offset = plane_slice_index * std::abs(next_slice_offset);

        for (int plane_y = 0; plane_y < chunk_size; ++plane_y)
        {
            int tile_offset = plane_y * plane_y_to_chunk_offset;
            int plane_offset = plane_y * chunk_size;
            for (int plane_x = 0; plane_x < chunk_size; ++plane_x)
            {
                int tile_index = tile_offset + plane_x * plane_x_to_chunk_offset;

                if constexpr (dir % 2 == 0)
                {
                    plane[plane_offset + plane_x] = tiles[tile_index + slice_tile_offset].properties().TextureID *
                                                    other_tiles[tile_index].properties().isTransparent;
                }
                else
                {
                    plane[plane_offset + plane_x] =
                        tiles[tile_index].properties().TextureID *
                        other_tiles[tile_index + slice_tile_offset].properties().isTransparent;
                }
            }
        }
    }

    template <Direction dir>
    static void create_plane(const std::array<Tile, chunk_volume>& tiles, std::array<uint16_t, chunk_area>& plane,
        size_t plane_slice_index)
    {
        const ConstantVals constants = get_constants<dir>();

        const int next_slice_offset = constants.next_slice_offset;
        const int plane_x_to_chunk_offset = constants.plane_x_to_chunk_offset;
        const int plane_y_to_chunk_offset = constants.plane_y_to_chunk_offset;

        // int plane_x = 0,plane_y = 0;

        for (int plane_y = 0; plane_y < chunk_size; ++plane_y)
        {
            int tile_offset = plane_y * plane_y_to_chunk_offset + plane_slice_index * std::abs(next_slice_offset);
            int plane_offset = plane_y * chunk_size;
            for (int plane_x = 0; plane_x < chunk_size; ++plane_x)
            {
                int tile_index = tile_offset + plane_x * plane_x_to_chunk_offset;
                plane[plane_offset + plane_x] = tiles[tile_index].properties().TextureID *
                                                tiles[tile_index + next_slice_offset].properties().isTransparent;
            }
        }
    }
};