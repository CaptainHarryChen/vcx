#include <iostream>
#include <list>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>

#include <glm/gtc/matrix_inverse.hpp>
#include <spdlog/spdlog.h>

#include "Labs/2-GeometryProcessing/DCEL.hpp"
#include "Labs/2-GeometryProcessing/tasks.h"

namespace VCX::Labs::GeometryProcessing {

#include "Labs/2-GeometryProcessing/marching_cubes_table.h"

    /******************* 1. Mesh Subdivision *****************/
    void SubdivisionMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, std::uint32_t numIterations) {
        // your code here
        // for(int i = 0; i < input.Positions.size(); i++)
        // {
        //     printf("(%f, %f, %f)\n",input.Positions[i].x,input.Positions[i].y,input.Positions[i].z);
        // }
        // for(int i = 0; i < input.Normals.size(); i++)
        // {
        //     printf("[%f, %f, %f]\n",input.Normals[i].x,input.Normals[i].y,input.Normals[i].z);
        // }
        // for(int i = 0; i < input.TexCoords.size(); i++)
        // {
        //     printf("(%f, %f)\n",input.TexCoords[i].x,input.TexCoords[i].y);
        // }
        // for(int i = 0; i < input.Indices.size(); i++)
        // {
        //     printf("[%u]\n",input.Indices[i]);
        // }
        Engine::SurfaceMesh tmp = input;
        for(std::uint32_t iter = 0; iter < numIterations; iter++)
        {
            DCEL links;
            links.AddFaces(tmp.Indices); // initialize
            assert(links.IsValid());
            std::size_t n = tmp.Positions.size();
            std::size_t m = tmp.Indices.size();
            std::vector<int> deg(n);
            std::vector<glm::vec3> sum(n);
            std::unordered_map<unsigned long long, int> ev_id;
            for(std::size_t i = 0; i < n; i++)
            {
                sum[i] = {0, 0, 0};
                deg[i] = 0;
            }

            for (DCEL::HalfEdge const * e : links.GetEdges())
            {
                if(ev_id.count(1LLU * e->From() * n + e->To())) // make sure not to repeat adding a vertex
                    continue;
                //printf("(%d, %d)\n", e->From(), e->To());
                //printf("(%d, %d, %d)   (%d, %d, %d)\n", *e->Face()->Indices(0), *e->Face()->Indices(1), *e->Face()->Indices(2), *e->OppositeFace()->Indices(0), *e->OppositeFace()->Indices(1), *e->OppositeFace()->Indices(2));
                glm::vec3 ev(0, 0, 0);
                ev += tmp.Positions[e->From()] + tmp.Positions[e->To()];
                assert(e->PairEdgeOr(NULL));
                for(int i = 0; i < 3; i++)
                {
                    ev += tmp.Positions[*e->Face()->Indices(i)];
                    ev += tmp.Positions[*e->OppositeFace()->Indices(i)];
                }
                ev /= 8.0f;   
                deg[e->From()]++;
                deg[e->To()]++;
                sum[e->From()] += ev;
                sum[e->To()] += ev;
                tmp.Positions.push_back(ev);
                ev_id[1LLU * e->From() * n + e->To()] = tmp.Positions.size() - 1;
                ev_id[1LLU * e->To() * n + e->From()] = tmp.Positions.size() - 1;
            }

            float pi = acos(-1);
            for(std::size_t i = 0; i < n; i++)
            {
                float alpha = deg[i] == 3 ? 3.0f / 16 : 3.0f / (8 * deg[i]);
                tmp.Positions[i] = (1 - deg[i] * alpha) * tmp.Positions[i] + alpha * sum[i];
            }

            tmp.Indices.clear();
            for(DCEL::Triangle const &f : links.GetFaces())
            {
                //printf("[%u %u %u]", *f.Indices(0), *f.Indices(1), *f.Indices(2));
                //printf("(%u %u %u %u %u %u)\n", f.Edges(0)->From(), f.Edges(0)->To(), f.Edges(1)->From(), f.Edges(1)->To(), f.Edges(2)->From(), f.Edges(2)->To());
                
                int ev[3];
                for(int i = 0; i < 3; i++)
                {
                    assert(ev_id.count(1LLU * f.Edges(i)->From() * n + f.Edges(i)->To()));
                    ev[i] = ev_id[1LLU * f.Edges(i)->From() * n + f.Edges(i)->To()];
                }
                //printf("[%u %u %u]\n", ev[0], ev[1], ev[2]);

                tmp.Indices.push_back(*f.Indices(0)); tmp.Indices.push_back(ev[2]); tmp.Indices.push_back(ev[1]);
                tmp.Indices.push_back(*f.Indices(1)); tmp.Indices.push_back(ev[0]); tmp.Indices.push_back(ev[2]);
                tmp.Indices.push_back(*f.Indices(2)); tmp.Indices.push_back(ev[1]); tmp.Indices.push_back(ev[0]);
                tmp.Indices.push_back(ev[0]); tmp.Indices.push_back(ev[1]); tmp.Indices.push_back(ev[2]);
            }

            // for(int i = 0; i < tmp.Positions.size(); i++)
            // {
            //     printf("(%f, %f, %f)\n",tmp.Positions[i].x,tmp.Positions[i].y,tmp.Positions[i].z);
            // }
            // for(int i = 0; i < tmp.Indices.size(); i++)
            // {
            //     printf("[%u]\n",tmp.Indices[i]);
            // }
            // DCEL check;
            // check.AddFaces(tmp.Indices); // initialize
            // assert(check.IsValid());
        }

        output = tmp;
    }

    /******************* 2. Mesh Parameterization *****************/
    void Parameterization(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, const std::uint32_t numIterations) {
        // your code here
        DCEL links;
        links.AddFaces(input.Indices); // initialize
        assert(links.IsValid());
        std::size_t n = input.Positions.size();
        int u = 0;
        while(!links.GetVertex(u).IsSide())
            u++;
        std::vector<bool> vis(n);
        for(std::size_t i = 0; i < n; i++)
            vis[i] = false;
        std::vector<int> side_v;
        do
        {
            side_v.push_back(u);
            vis[u] = true;
            int v = -1;
            std::pair<DCEL::VertexIdx, DCEL::VertexIdx> nxt = links.GetVertex(u).GetSideNeighbors();
            if(!vis[nxt.first])
                v = nxt.first;
            if(!vis[nxt.second])
                v = nxt.second;
            u = v;
        }while(u != -1);

        output = input;
        output.TexCoords.resize(n);
        for(std::size_t i = 0; i < n; i++)
            output.TexCoords[i] = {0.0f, 0.0f};
        
        int side_num = (side_v.size() + 3) / 4;
        for(std::size_t i = 0; i < side_num; i++)
            output.TexCoords[side_v[i]] = {1.0f * i / side_num, 0.0f};
        for(std::size_t i = 0; i < side_num; i++)
            output.TexCoords[side_v[i + side_num]] = {1.0f, 1.0f * i / side_num};
        for(std::size_t i = 0; i < side_num; i++)
            output.TexCoords[side_v[i + side_num * 2]] = {1.0f - 1.0f * i / side_num, 1.0f};
        for(std::size_t i = 0; i + side_num * 3 < side_v.size(); i++)
            output.TexCoords[side_v[i + side_num * 3]] = {0.0f, 1.0f - 1.0f * i / side_num};
        
        for(std::uint32_t iter = 0; iter < numIterations; iter++)
        {
            for(std::size_t i = 0; i < n; i++)
            {
                if(vis[i])
                    continue;
                auto neighbors = links.GetVertex(i).GetNeighbors();
                glm::vec2 sum(0.0f, 0.0f);
                for(DCEL::VertexIdx v : neighbors)
                    sum += output.TexCoords[v];
                sum /= 1.0f * neighbors.size();
                output.TexCoords[i] = sum;
            }
        }
        // for(int i = 0; i < output.TexCoords.size(); i++)
        // {
        //     printf("(%f, %f)\n", output.TexCoords[i].x, output.TexCoords[i].y);
        // }
    }

    /******************* 3. Mesh Simplification *****************/
    void SimplifyMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, float valid_pair_threshold, float simplification_ratio) {
        // your code here
    }

    /******************* 4. Mesh Smoothing *****************/
    void SmoothMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, std::uint32_t numIterations, float lambda, bool useUniformWeight) {
        // your code here
    }

    /******************* 5. Marching Cubes *****************/
    void MarchingCubes(Engine::SurfaceMesh & output, const std::function<float(const glm::vec3 &)> & sdf, const glm::vec3 & grid_min, const float dx, const int n) {
        // your code here
    }
} // namespace VCX::Labs::GeometryProcessing
