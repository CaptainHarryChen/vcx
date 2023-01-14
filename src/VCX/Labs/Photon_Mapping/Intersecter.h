#pragma once

#include <spdlog/spdlog.h>

#include "Engine/Scene.h"
#include "Labs/Photon_Mapping/Ray.h"

namespace VCX::Labs::Rendering {

    constexpr float EPS1 = 1e-2f; // distance to prevent self-intersection
    constexpr float EPS2 = 1e-8f; // angle for parallel judgement
    constexpr float EPS3 = 1e-4f; // relative distance to enlarge kdtree

    struct RayHit {
        bool              IntersectState;
        Engine::BlendMode IntersectMode;
        glm::vec3         IntersectPosition;
        glm::vec3         IntersectNormal;
        glm::vec4         IntersectAlbedo;   // [Albedo   (vec3), Alpha     (float)]
        glm::vec4         IntersectMetaSpec; // [Specular (vec3), Shininess (float)]
        glm::vec3         IntersectTrans;
        float             IntersectIor;
    };

    struct Intersection {
        float t, u, v; // ray parameter t, barycentric coordinates (u, v)
    };

    enum class ReflectType {
        None,
        Set,
        Diffuse,
        Specular,
        Refraction,
    };

    struct RayReflect {
        ReflectType Type;
        glm::vec3   Direction;
        glm::vec3   Attenuation;
    };

    glm::vec4 GetTexture(Engine::Texture2D<Engine::Formats::RGBA8> const & texture, glm::vec2 const & uvCoord);
    glm::vec4 GetAlbedo(Engine::Material const & material, glm::vec2 const & uvCoord);

    bool IntersectTriangle(Intersection & output, Ray const & ray, glm::vec3 const & p1, glm::vec3 const & p2, glm::vec3 const & p3);

    struct TrivialRayIntersector {
        Engine::Scene const * InternalScene = nullptr;

        TrivialRayIntersector() = default;

        void InitScene(Engine::Scene const * scene) {
            InternalScene = scene;
        }

        RayHit IntersectRay(Ray const & ray) const {
            RayHit result;
            if (! InternalScene) {
                spdlog::warn("VCX::Labs::Rendering::RayIntersector::IntersectRay(..): uninitialized intersector.");
                result.IntersectState = false;
                return result;
            }
            int          modelIdx, meshIdx;
            Intersection its;
            float        tmin     = 1e7, umin, vmin;
            int          maxmodel = InternalScene->Models.size();
            for (int i = 0; i < maxmodel; ++i) {
                auto const & model  = InternalScene->Models[i];
                int          maxidx = model.Mesh.Indices.size();
                for (int j = 0; j < maxidx; j += 3) {
                    std::uint32_t const * face = model.Mesh.Indices.data() + j;
                    glm::vec3 const &     p1   = model.Mesh.Positions[face[0]];
                    glm::vec3 const &     p2   = model.Mesh.Positions[face[1]];
                    glm::vec3 const &     p3   = model.Mesh.Positions[face[2]];
                    if (! IntersectTriangle(its, ray, p1, p2, p3)) continue;
                    if (its.t < EPS1 || its.t > tmin) continue;
                    tmin = its.t, umin = its.u, vmin = its.v, modelIdx = i, meshIdx = j;
                }
            }
            if (tmin == 1e7) {
                result.IntersectState = false;
                return result;
            }
            auto const &          model     = InternalScene->Models[modelIdx];
            auto const &          normals   = model.Mesh.IsNormalAvailable() ? model.Mesh.Normals : model.Mesh.ComputeNormals();
            auto const &          texcoords = model.Mesh.IsTexCoordAvailable() ? model.Mesh.TexCoords : model.Mesh.GetEmptyTexCoords();
            std::uint32_t const * face      = model.Mesh.Indices.data() + meshIdx;
            glm::vec3 const &     p1        = model.Mesh.Positions[face[0]];
            glm::vec3 const &     p2        = model.Mesh.Positions[face[1]];
            glm::vec3 const &     p3        = model.Mesh.Positions[face[2]];
            glm::vec3 const &     n1        = normals[face[0]];
            glm::vec3 const &     n2        = normals[face[1]];
            glm::vec3 const &     n3        = normals[face[2]];
            glm::vec2 const &     uv1       = texcoords[face[0]];
            glm::vec2 const &     uv2       = texcoords[face[1]];
            glm::vec2 const &     uv3       = texcoords[face[2]];
            result.IntersectState           = true;
            auto const & material           = InternalScene->Materials[model.MaterialIndex];
            result.IntersectMode            = material.Blend;
            result.IntersectPosition        = (1.0f - umin - vmin) * p1 + umin * p2 + vmin * p3;
            result.IntersectNormal          = (1.0f - umin - vmin) * n1 + umin * n2 + vmin * n3;
            glm::vec2 uvCoord               = (1.0f - umin - vmin) * uv1 + umin * uv2 + vmin * uv3;
            result.IntersectAlbedo          = GetAlbedo(material, uvCoord);
            result.IntersectMetaSpec        = GetTexture(material.MetaSpec, uvCoord);
            result.IntersectIor             = material.Ior;
            result.IntersectTrans           = material.Trans;

            return result;
        }
    };

    class BVH {
    public:
        struct Face {
            VCX::Engine::Model const * model;
            std::uint32_t const *      indice;

            Face() = default;
            Face(VCX::Engine::Model const * _model, std::uint32_t const * _indice):
                model(_model), indice(_indice) {}

            glm::vec3 operator[](int i) const {
                return model->Mesh.Positions[indice[i]];
            }

            glm::vec3 at(int i) const {
                return model->Mesh.Positions[indice[i]];
            }

            std::pair<float, float> range(int dim) const {
                float mn = std::min(std::min(at(0)[dim], at(1)[dim]), at(2)[dim]);
                float mx = std::max(std::max(at(0)[dim], at(1)[dim]), at(2)[dim]);
                return std::make_pair(mn, mx);
            }

            std::pair<glm::vec3, glm::vec3> range() const {
                glm::vec3 mn, mx;
                for (int dim = 0; dim < 3; dim++) {
                    mn[dim] = std::min(std::min(at(0)[dim], at(1)[dim]), at(2)[dim]);
                    mx[dim] = std::max(std::max(at(0)[dim], at(1)[dim]), at(2)[dim]);
                }
                return std::make_pair(mn, mx);
            }
        };

        struct FaceCompare {
            int cmp_dim;
            FaceCompare(int cmpDim):
                cmp_dim(cmpDim) {}
            bool operator()(const Face & A, const Face & B) const {
                return A.range(cmp_dim) < B.range(cmp_dim);
            }
        };

        struct Node {
            glm::vec3 min_pos;
            glm::vec3 max_pos;
            bool      is_leaf;
            union {
                Node * son[2];
                Face   face;
            };
        };

    private:
        std::vector<Face> internelFaces;
        Node *            root = nullptr;

        void BuildTree(Node *& u, int L, int R) {
            u          = new Node();
            auto rg    = internelFaces[L].range();
            u->min_pos = rg.first;
            u->max_pos = rg.second;
            if (L + 1 == R) {
                u->is_leaf = true;
                u->face    = internelFaces[L];
                return;
            }
            u->is_leaf = false;
            for (int i = L + 1; i < R; i++) {
                rg = internelFaces[i].range();
                for (int d = 0; d < 3; d++) {
                    u->min_pos[d] = std::min(u->min_pos[d], rg.first[d]);
                    u->max_pos[d] = std::max(u->max_pos[d], rg.second[d]);
                }
            }
            int split_dim = 0;
            for (int d = 1; d < 3; d++)
                if (u->max_pos[d] - u->min_pos[d] > u->max_pos[split_dim] - u->min_pos[split_dim])
                    split_dim = d;
            int mid = (L + R) / 2;
            std::nth_element(internelFaces.begin() + L, internelFaces.begin() + mid, internelFaces.begin() + R, FaceCompare(split_dim));
            BuildTree(u->son[0], L, mid);
            BuildTree(u->son[1], mid, R);
        }
        void free(Node * u) {
            if (! u->is_leaf) {
                if (u->son[0])
                    free(u->son[0]);
                if (u->son[1])
                    free(u->son[1]);
            }
            delete u;
        }
        bool PointInBox(glm::vec3 p, glm::vec3 min_pos, glm::vec3 max_pos) const {
            for (int d = 0; d < 3; d++)
                if (min_pos[d] > p[d] || p[d] > max_pos[d])
                    return false;
            return true;
        }
        bool RayInBox(const Ray & ray, glm::vec3 min_pos, glm::vec3 max_pos) const {
            float tmin = 0, tmax = 1e10f;
            for (int d = 0; d < 3; d++) {
                if (abs(ray.Direction[d]) < EPS3) {
                    if (min_pos[d] < ray.Origin[d] && ray.Origin[d] < max_pos[d])
                        continue;
                    return false;
                }
                float t1 = (min_pos[d] - ray.Origin[d]) / ray.Direction[d];
                float t2 = (max_pos[d] - ray.Origin[d]) / ray.Direction[d];
                if (t1 > t2)
                    std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
            }
            return tmin <= tmax;
        }
        bool FindInNode(Node * u, Intersection & result, Face & resFace, const Ray & ray) const {
            if (u->is_leaf) {
                if (IntersectTriangle(result, ray, u->face[0], u->face[1], u->face[2])) {
                    resFace = u->face;
                    return true;
                }
                return false;
            }
            if (! RayInBox(ray, u->min_pos, u->max_pos))
                return false;
            Intersection res1, res2;
            Face         resf1, resf2;
            bool         flag1, flag2;
            flag1 = FindInNode(u->son[0], res1, resf1, ray);
            flag2 = FindInNode(u->son[1], res2, resf2, ray);
            if (flag1 && flag2) {
                if (res1.t < res2.t)
                    result = res1, resFace = resf1;
                else
                    result = res2, resFace = resf2;
            } else if (flag1)
                result = res1, resFace = resf1;
            else if (flag2)
                result = res2, resFace = resf2;
            else
                return false;
            return true;
        }

    public:
        BVH() = default;
        ~BVH() {
            if (root) free(root);
        }
        void Clear() {
            internelFaces.clear();
            if (root)
                free(root);
        }
        void Build(std::vector<Face> faces) {
            internelFaces = faces;
            BuildTree(root, 0, internelFaces.size());
        }
        bool FindIntersection(Intersection & result, Face & resFace, const Ray & ray) const {
            return FindInNode(root, result, resFace, ray);
        }
    };

    struct BVHRayIntersector {
        Engine::Scene const * InternalScene = nullptr;
        BVH                   bvh;

        BVHRayIntersector() = default;

        void InitScene(Engine::Scene const * scene) {
            InternalScene = scene;
            std::vector<BVH::Face> faces;
            int                    maxmodel = InternalScene->Models.size();

            for (int i = 0; i < maxmodel; ++i) {
                auto const & model  = InternalScene->Models[i];
                int          maxidx = model.Mesh.Indices.size();
                for (int j = 0; j < maxidx; j += 3)
                    faces.push_back(BVH::Face(&model, model.Mesh.Indices.data() + j));
            }
            bvh.Clear();
            bvh.Build(faces);
        }

        RayHit IntersectRay(Ray const & ray) const {
            RayHit result;
            if (! InternalScene) {
                spdlog::warn("VCX::Labs::Rendering::RayIntersector::IntersectRay(..): uninitialized intersector.");
                result.IntersectState = false;
                return result;
            }
            Intersection its;
            BVH::Face    resface;
            if (! bvh.FindIntersection(its, resface, ray)) {
                result.IntersectState = false;
                return result;
            }
            auto const &          normals   = resface.model->Mesh.IsNormalAvailable() ? resface.model->Mesh.Normals : resface.model->Mesh.ComputeNormals();
            auto const &          texcoords = resface.model->Mesh.IsTexCoordAvailable() ? resface.model->Mesh.TexCoords : resface.model->Mesh.GetEmptyTexCoords();
            std::uint32_t const * face      = resface.indice;
            glm::vec3 const &     p1        = resface.model->Mesh.Positions[face[0]];
            glm::vec3 const &     p2        = resface.model->Mesh.Positions[face[1]];
            glm::vec3 const &     p3        = resface.model->Mesh.Positions[face[2]];
            glm::vec3 const &     n1        = normals[face[0]];
            glm::vec3 const &     n2        = normals[face[1]];
            glm::vec3 const &     n3        = normals[face[2]];
            glm::vec2 const &     uv1       = texcoords[face[0]];
            glm::vec2 const &     uv2       = texcoords[face[1]];
            glm::vec2 const &     uv3       = texcoords[face[2]];
            result.IntersectState           = true;
            auto const & material           = InternalScene->Materials[resface.model->MaterialIndex];
            result.IntersectMode            = material.Blend;
            result.IntersectPosition        = (1.0f - its.u - its.v) * p1 + its.u * p2 + its.v * p3;
            result.IntersectNormal          = (1.0f - its.u - its.v) * n1 + its.u * n2 + its.v * n3;
            glm::vec2 uvCoord               = (1.0f - its.u - its.v) * uv1 + its.u * uv2 + its.v * uv3;
            result.IntersectAlbedo          = GetAlbedo(material, uvCoord);
            result.IntersectMetaSpec        = GetTexture(material.MetaSpec, uvCoord);
            result.IntersectIor             = material.Ior;
            result.IntersectTrans           = material.Trans;

            return result;
        }
    };

    using RayIntersector = BVHRayIntersector;

    glm::vec3 RandomDirection();
    glm::vec3 RandomHemiDirection(const glm::vec3 & normal);
    glm::vec3 RandomCosineDirection(const glm::vec3 & n, const glm::vec3 &lx);

    RayReflect DirectionFromBSDF(const Ray & ray, const RayHit & rayHit);
    glm::vec3  DirectLight(const RayIntersector & intersector, const Ray & ray, const RayHit & rayHit, bool enableShadow);

} // namespace VCX::Labs::Rendering
