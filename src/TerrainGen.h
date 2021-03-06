#include <functional>
#include <iostream>
#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <map>

#include "vectors.h"

struct ChunkDetails
{
    Vector2Int cord;
    int stage;
    ChunkDetails(Vector2Int cord_, int stage_)
    {
        cord = cord_;
        stage = stage_;
    }
    inline bool operator < (const ChunkDetails& ot) const
    {
        std::hash<std::string> h;
        //std::cout << h(std::string((char*)this, (char*)this + sizeof(ChunkDetails))) << " " << h(std::string((char*)&ot, (char*)&ot + sizeof(ChunkDetails))) << '\n';
        return h(std::string((char*)this, (char*)this + sizeof(ChunkDetails))) > h(std::string((char*)&ot, (char*)&ot + sizeof(ChunkDetails)));
    }
    inline ChunkDetails operator + (Vector2Int v)
    {
        return ChunkDetails(v + cord, stage);
    }
};

const int nsize = 3;
const int nofset = (nsize - 1) / 2;

template<typename Chunk>
using nearbyChunks = std::array<std::array<std::shared_ptr<const Chunk>, nsize>, nsize>;

template<typename Chunk>
struct ChunkGenStage
{
    typedef nearbyChunks<Chunk> nearbyChunks;

    typedef std::function<void(Chunk&)> ModifyCurrent; //Can only modify the current chunk and can't peek to other chunks
    typedef std::function<std::unique_ptr<Chunk>(nearbyChunks)> NewStage; //Gets mutiple chunks and creates a new stage chunk

    NewStage newStagefunc;
    std::vector<ModifyCurrent> modifierRules;
};

template<typename Chunk>
class TerrainGenerator
{
    typedef nearbyChunks<Chunk> nearbyChunks;

    typedef std::function<void(Chunk&)> ModifyCurrent; //Can only modify the current chunk and can't peek to other chunks
    typedef std::function<std::unique_ptr<Chunk>(nearbyChunks)> NewStage; //Gets mutiple chunks and creates a new stage chunk



private://data

    std::vector<ModifyCurrent> chunkInitRules;
    std::vector<ChunkGenStage<Chunk>> stages;
    int LastStageNum = -1;

    std::map<ChunkDetails, std::tuple<int, std::shared_ptr<Chunk> > > cachedChunks;
    std::vector<std::variant<ModifyCurrent, NewStage>> rules;
public:
    std::function<void(Chunk&, Vector2Int)> setPos;

private://functions

    void SetNeighbourChunks(Chunk& c, ChunkDetails details)
    {
        {
            auto cP = cachedChunks.find(details + Vector2Int(0, 1));
            if (cP != cachedChunks.end())
                c.northernChunk = std::get<std::shared_ptr<Chunk>>(cP->second).get();
        }
        {
            auto cP = cachedChunks.find(details + Vector2Int(0, -1));
            if (cP != cachedChunks.end())
                c.southernChunk = std::get<std::shared_ptr<Chunk>>(cP->second).get();
        }
        {
            auto cP = cachedChunks.find(details + Vector2Int(1, 0));
            if (cP != cachedChunks.end())
                c.easternChunk = std::get<std::shared_ptr<Chunk>>(cP->second).get();
        }
        {
            auto cP = cachedChunks.find(details + Vector2Int(-1, 0));
            if (cP != cachedChunks.end())
                c.westernChunk = std::get<std::shared_ptr<Chunk>>(cP->second).get();
        }

        if (c.northernChunk)
            c.northernChunk->southernChunk = &c;
        if (c.southernChunk)
            c.southernChunk->northernChunk = &c;
        if (c.westernChunk)
            c.westernChunk->easternChunk = &c;
        if (c.easternChunk)
            c.easternChunk->westernChunk = &c;
    }

    std::unique_ptr<Chunk> initChunk(Vector2Int cord)
    {
        auto c = std::make_unique<Chunk>(cord);
        for (ModifyCurrent& r : chunkInitRules)
        {
            r(*c);
        }
        SetNeighbourChunks(*c, ChunkDetails(cord, -1));
        return c;
    }

    std::unique_ptr<Chunk> GetChunkAtStage(Vector2Int cord, int stagenum)
    {
        //stagenum -1 is reserved for init
        if (stagenum == -1)
        {
            return initChunk(cord);
        }

        nearbyChunks nb;
        for (int x_ = 0;x_ < nsize;++x_)
        {
            int x = x_ - nofset;
            for (int z_ = 0; z_ < nsize; ++z_)
            {
                int z = z_ - nofset;

                ChunkDetails d = ChunkDetails(Vector2Int(x, z) + cord, stagenum - 1);
                auto c = cachedChunks.find(d);

                if (c != cachedChunks.end())
                {
                    nb[x_][z_] = std::get<std::shared_ptr<Chunk>>(c->second);
                    if (++std::get<int>(c->second) >= nsize * nsize)
                    {
                        cachedChunks.erase(c);
                    }
                }
                else
                {
                    //if it can't ind it creates one
                    std::shared_ptr<Chunk> c_ = GetChunkAtStage(Vector2Int(x, z) + cord, stagenum - 1);
                    nb[x_][z_] = c_;
                    //starts the counter at 1 since it used it once
                    cachedChunks[d] = std::tuple(1, c_);
                    //
                }
            }
        }

        //apply rules
        ChunkGenStage<Chunk> stage = stages[stagenum];
        auto chunk = stage.newStagefunc(std::move(nb));
        for (auto& r : stage.modifierRules)
        {
            r(*chunk);
        }
        SetNeighbourChunks(*chunk, ChunkDetails(cord, stagenum));
        return std::move(chunk);//
    }
public://

    void addRule(ModifyCurrent rule)
    {
        rules.push_back(rule);
    }

    void addRule(NewStage rule)
    {
        rules.push_back(rule);
    }

    void Init()
    {


        int first_stageRule = -1;
        for (int i = 0;i < rules.size();++i)
        {
            if (auto func = std::get_if<ModifyCurrent>(&(rules[i])))
            {
                chunkInitRules.push_back(*func);
            }
            else
            {
                first_stageRule = i;
                break;
            }
        }
        if (first_stageRule == -1)
            return;

        std::vector<ModifyCurrent> modifiers;

        for (int i = first_stageRule;i < rules.size();++i)
        {
            if (auto func = std::get_if<ModifyCurrent>(&(rules[i])))
            {
                //std::cout << "aaaaaaaa\n";
                stages.back().modifierRules.push_back(*func);
            }
            else
            {
                stages.push_back({std::get<NewStage>(rules[i]),std::move(modifiers)});
            }
        }

        LastStageNum = stages.size() - 1;
    }

    inline std::unique_ptr<Chunk> GenerateChunk(Vector2Int cord)
    {
        return GetChunkAtStage(cord, LastStageNum);
    }

};