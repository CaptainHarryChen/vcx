#include <iostream>
#include <list>
#include <map>
#include <set>
#include <queue>
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
    struct ErrorVertexPair
    {
        float error;
        std::size_t u, v;
        DCEL::HalfEdge const *e;
        glm::vec3 newv;
        bool operator < (const ErrorVertexPair &t) const {
            return error < t.error;
        }
        bool operator > (const ErrorVertexPair &t) const {
            return error > t.error;
        }
    };

    ErrorVertexPair NewQuadricError(std::size_t uid, std::size_t vid, const glm::vec3 &u, const glm::vec3 &v, const glm::mat4 &uQ, const glm::mat4 &vQ, DCEL::HalfEdge const *e)
    {
        ErrorVertexPair ret;
        glm::vec4 newv;
        ret.u = uid; ret.v = vid;
        ret.e = e;
        glm::mat4 eQ = uQ + vQ;
        glm::mat4 dQ = eQ;
        dQ[3][0] = dQ[3][1] = dQ[3][2] = 0; dQ[3][3] = 1;
        if(abs(glm::determinant(dQ)) > 0.1)
        {
            glm::mat4 inv = glm::inverse(dQ);
            // newv = inv * glm::vec4(0, 0, 0, 1);
            newv = {inv[0][3], inv[1][3], inv[2][3], inv[3][3]};
            newv /= newv[3];
        }
        else
        {
            glm::vec3 mid = u + v;
            mid /= 2.0f;
            newv = {mid.x, mid.y, mid.z, 1};
        }
        ret.newv = {newv[0], newv[1], newv[2]};
        ret.error = 0.0f;
        for(int i = 0; i < 4; i++)
            for(int j = 0; j < 4; j++)
                ret.error += newv[i] * eQ[i][j] * newv[j];
        return ret;
    }

    void SimplifyMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, float valid_pair_threshold, float simplification_ratio) {
        std::size_t origin_n = input.Positions.size();
        std::vector<glm::mat4> vertexQ;
        std::vector<bool> delv;
        std::vector<std::size_t> newid;
        std::vector<std::size_t> del_id;
        std::priority_queue< ErrorVertexPair, std::vector<ErrorVertexPair>, std::greater<ErrorVertexPair> > valid_pairs;
        Engine::SurfaceMesh mesh = input;
        std::size_t cur_n = mesh.Positions.size();
        
        while(cur_n > origin_n * simplification_ratio)
        {
            DCEL links;
            links.AddFaces(mesh.Indices);
            assert(links.IsValid());
            
            // calculate the initial Q of vertexes
            vertexQ.clear();
            for(std::size_t i = 0; i < mesh.Positions.size(); i++)
            {
                glm::mat4 vQ(0);
                for(DCEL::Triangle const *f : links.GetVertex(i).GetFaces())
                {
                    glm::vec3 a = mesh.Positions[*f->Indices(0)], b = mesh.Positions[*f->Indices(1)], c = mesh.Positions[*f->Indices(2)];
                    glm::vec3 normal = glm::normalize(glm::cross(a - b, c - b));
                    glm::vec4 ni(normal.x, normal.y, normal.z, - glm::dot(normal, a));
                    glm::mat4 fQ;
                    for(int i = 0; i < 4; i++)
                        for(int j = 0; j < 4; j++)
                            fQ[i][j] = ni[i] * ni[j];
                    vQ += fQ;
                }
                vertexQ.push_back(vQ);
            }

            // find valid pairs
            for(std::size_t i = 0; i < cur_n; i++)
                for(std::size_t j = i + 1; j < cur_n; j++)
                    if((mesh.Positions[i] - mesh.Positions[j]).length() < valid_pair_threshold)
                    {
                        // if there is a neighbor k with both i and j, this pair is illegal
                        bool legal = true;
                        auto ni = links.GetVertex(i).GetNeighbors();
                        std::sort(ni.begin(), ni.end());
                        auto nj = links.GetVertex(j).GetNeighbors();
                        for(auto t: nj)
                            if(std::find(ni.begin(), ni.end(), t) != ni.end())
                            {
                                legal = false;
                                break;
                            }
                        if(legal)
                            valid_pairs.push(NewQuadricError(i, j, mesh.Positions[i], mesh.Positions[j], vertexQ[i], vertexQ[j], NULL));
                    }
            for(DCEL::HalfEdge const *e : links.GetEdges())
            {
                int i = e->From(), j = e->To();
                // if there is a neighbor k with both i and j, and ijk is not a face of the mesh, then the pair is illegal
                bool legal = true;
                auto ni = links.GetVertex(i).GetNeighbors();
                std::sort(ni.begin(), ni.end());
                auto nj = links.GetVertex(j).GetNeighbors();
                for(auto t: nj)
                    if(std::find(ni.begin(), ni.end(), t) != ni.end())
                        if(t != e->OppositeVertex() && t != e->PairOppositeVertex())
                        {
                            legal = false;
                            break;
                        }
                if(legal)
                    valid_pairs.push(NewQuadricError(i, j, mesh.Positions[i], mesh.Positions[j], vertexQ[i], vertexQ[j], e));
            }
            // delete old vertex pair and add new vertex
            delv.clear();
            delv.resize(cur_n);
            newid.clear();
            newid.resize(cur_n);
            std::size_t rest_num = cur_n;
            while(rest_num > cur_n * simplification_ratio && !valid_pairs.empty())
            {
                ErrorVertexPair evp;
                do
                {
                    evp = valid_pairs.top();
                    valid_pairs.pop();
                    if(delv[evp.u] || delv[evp.v])
                        continue;
                    // check if the vertex pair is collapse legal
                    int i = evp.u, j = evp.v;
                    bool legal = true;
                    auto ni = links.GetVertex(i).GetNeighbors();
                    for(int x = 0; x < ni.size(); x++)
                        ni[x] = delv[ni[x]] ? newid[ni[x]] : ni[x];
                    std::sort(ni.begin(), ni.end());
                    auto nj = links.GetVertex(j).GetNeighbors();
                    for(int x = 0; x < nj.size(); x++)
                        nj[x] = delv[nj[x]] ? newid[nj[x]] : nj[x];
                    for(auto t: nj)
                        if(std::find(ni.begin(), ni.end(), t) != ni.end())
                        {
                            if(evp.e)
                            {
                                std::size_t facev1 = evp.e->OppositeVertex(), facev2 = evp.e->PairOppositeVertex();
                                facev1 = delv[facev1] ? newid[facev1] : facev1;
                                facev2 = delv[facev2] ? newid[facev2] : facev2;
                                if(t != facev1 && t != facev2)
                                {
                                    legal = false;
                                    break;
                                }
                            }
                            else
                            {
                                legal = false;
                                break;
                            }
                        }
                    if(legal)
                       break; 
                }while(!valid_pairs.empty());
                if(delv[evp.u] || delv[evp.v])
                    break;
                delv[evp.u] = delv[evp.v] = true;
                mesh.Positions.push_back(evp.newv);
                newid[evp.u] = newid[evp.v] = mesh.Positions.size() - 1;
                rest_num--;
            }
            // reconstruct all the faces
            mesh.Indices.clear();
            for(DCEL::Triangle const &f : links.GetFaces())
            {
                std::size_t ind[3];
                for(int i = 0; i < 3; i++)
                {
                    if(delv[*f.Indices(i)])
                        ind[i] = newid[*f.Indices(i)];
                    else
                        ind[i] = *f.Indices(i);
                }
                if(ind[0] != ind[1] && ind[0] != ind[2] && ind[1] != ind[2])
                {
                    for(int i = 0; i < 3; i++)
                        mesh.Indices.push_back(ind[i]);
                }
            }
            // delete the vertex from the mesh.Positions
            del_id.clear();
            del_id.resize(mesh.Positions.size());
            for(std::size_t i = 0, j = 0; i < mesh.Positions.size(); i++)
                if(i >= delv.size() || !delv[i])
                {
                    del_id[i] = j;
                    mesh.Positions[j] = mesh.Positions[i];
                    j++;
                }
            mesh.Positions.resize(rest_num);
            for(std::size_t i = 0; i < mesh.Indices.size(); i++)
                mesh.Indices[i] = del_id[mesh.Indices[i]];
            
            cur_n = rest_num;
        }
        
        output = mesh;
    }

    /******************* 4. Mesh Smoothing *****************/
    void SmoothMesh(Engine::SurfaceMesh const & input, Engine::SurfaceMesh & output, std::uint32_t numIterations, float lambda, bool useUniformWeight) {
        // your code here
        std::vector<glm::vec3> newv;
        std::vector<float> weight;
        Engine::SurfaceMesh mesh = input;
        DCEL links;
        links.AddFaces(mesh.Indices);
        assert(links.IsValid());
        for(std::uint32_t iter = 0; iter < numIterations; iter++)
        {
            newv.clear();
            newv.resize(mesh.Positions.size());
            weight.clear();
            weight.resize(mesh.Positions.size());
            for(auto e: links.GetEdges())
            {
                std::size_t v1_id = e->From(), v2_id = e->To();
                glm::vec3 v1 = mesh.Positions[v1_id], v2 = mesh.Positions[v2_id];
                float w = 1;
                if(!useUniformWeight)
                {
                    std::size_t u1_id = e->OppositeVertex(), u2_id = e->PairOppositeVertex();
                    glm::vec3 u1 = mesh.Positions[u1_id], u2 = mesh.Positions[u2_id];
                    float alpha = abs(glm::dot(u1 - v1, u1 - v2) / glm::cross(u1 - v1, u1 - v2).length());
                    float beta = abs(glm::dot(u2 - v1, u2 - v2) / glm::cross(u2 - v1, u2 - v2).length());
                    w = alpha + beta;
                }
                newv[v1_id] += v2 * w;
                weight[v1_id] += w;
                newv[v2_id] += v1 * w;
                weight[v2_id] += w;
            }
            for(std::size_t i = 0; i < mesh.Positions.size(); i++)
                mesh.Positions[i] = mesh.Positions[i] * (1 - lambda) + newv[i] / weight[i] * lambda;
        }
        output = mesh;
    }

    /******************* 5. Marching Cubes *****************/
    glm::vec3 weightMidPoint(glm::vec3 u, glm::vec3 v, float w1, float w2)
    {
        // return (u + v) / 2.0f;    // if you want Sci-fi style, use this ~~~///(^v^)\\\~~~
        float sum = w1 - w2;
        return (- w2 * u + w1 * v) / sum;
    }
    
    void MarchingCubes(Engine::SurfaceMesh & output, const std::function<float(const glm::vec3 &)> & sdf, const glm::vec3 & grid_min, const float dx, const int n) {
        std::vector<std::vector<std::vector<std::vector<int> > > > v_id;
        v_id.resize(n);
        for(int i = 0; i < n; i++)
        {
            v_id[i].resize(n);
            for(int j = 0; j < n; j++)
            {
                v_id[i][j].resize(n);
                for(int k = 0; k < n; k++)
                {
                    v_id[i][j][k].resize(12);
                    for(int l = 0; l < 12; l++)
                        v_id[i][j][k][l]=-1;
                }
            }
        }
        for(int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                for(int k = 0; k < n; k++)
                {
                    int state = 0;
                    glm::vec3 cube_pos = {i * dx, j * dx, k * dx};
                    cube_pos += grid_min;
                    glm::vec3 v[8];
                    for(int l = 0; l < 8; l++)
                    {
                        v[l] = {(l&1)*dx, (l>>1&1)*dx, (l>>2&1)*dx};
                        v[l] += cube_pos;
                        if(sdf(v[l]) < 0.0f)
                            state |= (1<<l);
                    }
                    int estate = c_EdgeStateTable[state];
                    for(int l = 0; l < 12; l++)
                        if(estate&(1<<l))
                        {
                            int &id = v_id[i][j][k][l];
                            glm::vec3 newv;
                            id = -1;
                            switch(l)
                            {
                            case 0:
                                if(k>0)
                                    id = v_id[i][j][k-1][2];
                                else if(j>0)
                                    id = v_id[i][j-1][k][1];
                                else
                                    newv = weightMidPoint(v[0], v[1], sdf(v[0]), sdf(v[1]));
                                break;
                            case 1:
                                if(k>0)
                                    id = v_id[i][j][k-1][3];
                                else
                                    newv = weightMidPoint(v[2], v[3], sdf(v[2]), sdf(v[3]));
                                break;
                            case 2:
                                if(j>0)
                                    id = v_id[i][j-1][k][3];
                                else
                                    newv = weightMidPoint(v[4], v[5], sdf(v[4]), sdf(v[5]));
                                break;
                            case 3:
                                newv = weightMidPoint(v[6], v[7], sdf(v[6]), sdf(v[7]));
                                break;
                            case 4:
                                if(k>0)
                                    id = v_id[i][j][k-1][5];
                                else if(i>0)
                                    id = v_id[i-1][j][k][6];
                                else
                                    newv = weightMidPoint(v[0], v[2], sdf(v[0]), sdf(v[2]));
                                break;
                            case 5:
                                if(i>0)
                                    id = v_id[i-1][j][k][7];
                                else
                                    newv = weightMidPoint(v[4], v[6], sdf(v[4]), sdf(v[6]));
                                break;
                            case 6:
                                if(k>0)
                                    id = v_id[i][j][k-1][7];
                                else
                                    newv = weightMidPoint(v[1], v[3], sdf(v[1]), sdf(v[3]));
                                break;
                            case 7:
                                newv = weightMidPoint(v[5], v[7], sdf(v[5]), sdf(v[7]));
                                break;
                            case 8:
                                if(j>0)
                                    id = v_id[i][j-1][k][10];
                                else if(i>0)
                                    id = v_id[i-1][j][k][9];
                                else
                                    newv = weightMidPoint(v[0], v[4], sdf(v[0]), sdf(v[4]));
                                break;
                            case 9:
                                if(j>0)
                                    id = v_id[i][j-1][k][11];
                                else
                                    newv = weightMidPoint(v[1], v[5], sdf(v[1]), sdf(v[5]));
                                break;
                            case 10:
                                if(i>0)
                                    id = v_id[i-1][j][k][11];
                                else
                                    newv = weightMidPoint(v[2], v[6], sdf(v[2]), sdf(v[6]));
                                break;
                            case 11:
                                newv = weightMidPoint(v[3], v[7], sdf(v[3]), sdf(v[7]));
                                break;
                            }
                            if(id == -1)
                            {
                                id = output.Positions.size();
                                output.Positions.push_back(newv);
                            }
                        }
                    for(int l = 0; l < 12; l++)
                    {
                        if(c_EdgeOrdsTable[state][l] == -1)
                            break;
                        output.Indices.push_back(v_id[i][j][k][c_EdgeOrdsTable[state][l]]);
                    }
                }
    }
} // namespace VCX::Labs::GeometryProcessing
