#pragma once

#include <queue>
#include <random>
#include <spdlog/spdlog.h>

#include "Engine/Scene.h"
#include "Labs/Photon_Mapping/Intersecter.h"
#include "Labs/Photon_Mapping/Ray.h"

namespace VCX::Labs::Rendering {

    using VCX::Labs::Rendering::Photon;

    class kDTree {
        struct Node {
            Photon * p      = nullptr;
            Node *   son[2] = { nullptr };
        };

        struct PhotonCompareByAxis {
            int axis;
            PhotonCompareByAxis(int _axis):
                axis(_axis) {}
            bool operator()(const Photon & A, const Photon & B) const {
                return A.Origin[axis] < B.Origin[axis];
            }
        };

        struct disNode {
            float    dis;
            Photon * p;
            bool     operator<(const disNode & B) const {
                return dis < B.dis;
            }
            disNode(float _dis, Photon * _p):
                dis(_dis), p(_p) {}
        };

        typedef std::priority_queue<disNode> NearestQueue;

        static const int    nDims = 3;
        std::vector<Photon> internelPhotons;
        Node *              root = nullptr;

        void BuildTree(Node *& u, int L, int R, int dep) {
            if (L >= R)
                return;
            if (! u)
                u = new Node;
            if (R - L == 1) {
                u->p      = internelPhotons.data() + L;
                u->son[0] = u->son[1] = nullptr;
                return;
            }
            int axis = dep % nDims;
            int mid  = (L + R) / 2;
            std::nth_element(internelPhotons.begin() + L, internelPhotons.begin() + mid, internelPhotons.begin() + R, PhotonCompareByAxis(axis));
            u->p = internelPhotons.data() + mid;
            BuildTree(u->son[0], L, mid, dep + 1);
            BuildTree(u->son[1], mid + 1, R, dep + 1);
        }
        void CheckAndAdd(const glm::vec3 & pos, Photon * p, int K, NearestQueue & Q, float mx_dis) const {
            if (Q.size() < K) {
                float dis2 = glm::dot(pos - p->Origin, pos - p->Origin);
                if (mx_dis < 0 || dis2 <= mx_dis * mx_dis)
                    Q.push(disNode(dis2, p));
                return;
            }
            float mx   = Q.empty() ? mx_dis : Q.top().dis;
            float dis2 = glm::dot(pos - p->Origin, pos - p->Origin);
            if (dis2 < mx) {
                Q.pop();
                Q.push(disNode(dis2, p));
            }
        }
        void FindNearestKPhotons(Node * u, int L, int R, int dep, const glm::vec3 & pos, int K, NearestQueue & Q, float mx_dis) const {
            if (L >= R)
                return;
            if (R - L == 1) {
                CheckAndAdd(pos, u->p, K, Q, mx_dis);
                return;
            }
            int mid  = (L + R) / 2;
            int axis = dep % nDims;
            if (pos[axis] <= u->p->Origin[axis])
                FindNearestKPhotons(u->son[0], L, mid, dep + 1, pos, K, Q, mx_dis);
            else
                FindNearestKPhotons(u->son[1], mid + 1, R, dep + 1, pos, K, Q, mx_dis);
            CheckAndAdd(pos, u->p, K, Q, mx_dis);
            float mx   = Q.empty() ? mx_dis : Q.top().dis;
            float dis2 = (u->p->Origin[axis] - pos[axis]) * (u->p->Origin[axis] - pos[axis]);
            if(dis2 > mx_dis * mx_dis)
                return;
            if (Q.size() < K || mx >= dis2) {
                if (pos[axis] > u->p->Origin[axis])
                    FindNearestKPhotons(u->son[0], L, mid, dep + 1, pos, K, Q, mx_dis);
                else
                    FindNearestKPhotons(u->son[1], mid + 1, R, dep + 1, pos, K, Q, mx_dis);
            }
        }

        void ClearTree(Node * u) {
            if (u) {
                ClearTree(u->son[0]);
                ClearTree(u->son[1]);
                delete u;
            }
        }

    public:
        void Clear() {
            ClearTree(root);
        }
        ~kDTree() {
            Clear();
        }
        void Build(const std::vector<Photon> photons) {
            internelPhotons = photons;
            BuildTree(root, 0, internelPhotons.size(), 0);
        }
        void NearestKPhotons(const glm::vec3 & pos, int K, std::vector<Photon> & ans, float mx_dis) const {
            NearestQueue Q;
            FindNearestKPhotons(root, 0, internelPhotons.size(), 0, pos, K, Q, mx_dis);
            while (! Q.empty()) {
                ans.push_back(*Q.top().p);
                Q.pop();
            }
        }
    };

    struct PhotonMapping {
        Engine::Scene const * InternalScene = nullptr;
        std::vector<Photon>   photons;
        kDTree                tree;

        float * progress = nullptr;
        bool *  onInit   = nullptr;

        PhotonMapping() = default;

        Photon    GeneratePhoton(Engine::Light light, int totalNum);
        void      InitScene(Engine::Scene const * scene, const RayIntersector & intersector, bool useDirect, int nEmittedPhotons = 300000, float P_RR = 0.7f);
        void      InitCaustic(Engine::Scene const * scene, const RayIntersector & intersector, int nEmittedPhotons = 300000, float P_RR = 0.7f);
        void      InitCausticDispersion(Engine::Scene const * scene, const RayIntersector & intersector, int nEmittedPhotons = 300000, float P_RR = 0.7f);
        glm::vec3 CollatePhotons(const RayHit & rayHit, const glm::vec3 & out_dir, int numPhotons = 600, float mx_dis = -1) const;
    };

} // namespace VCX::Labs::Rendering