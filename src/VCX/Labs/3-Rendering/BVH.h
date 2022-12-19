#pragma once

namespace VCX::Labs::Rendering {
    class BVH {
    public:
        struct Face {
            VCX::Engine::Model const * model;
            std::uint32_t const *      indice;

            Face() = default;
            Face(VCX::Engine::Model const * _model, std::uint32_t const * _indice):
                model(_model), indice(_indice) {}

            std::pair<float, float> range(int dim) const {
                float mn = std::min(std::min(model->Mesh.Positions[indice[0]][dim], model->Mesh.Positions[indice[1]][dim]), model->Mesh.Positions[indice[2]][dim]);
                float mx = std::max(std::max(model->Mesh.Positions[indice[0]][dim], model->Mesh.Positions[indice[1]][dim]), model->Mesh.Positions[indice[2]][dim]);
                return std::make_pair(mn, mx);
            }

            std::pair<glm::vec3, glm::vec3> range() const {
                glm::vec3 mn, mx;
                for (int dim = 0; dim < 3; dim++) {
                    mn[dim] = std::min(std::min(model->Mesh.Positions[indice[0]][dim], model->Mesh.Positions[indice[1]][dim]), model->Mesh.Positions[indice[2]][dim]);
                    mx[dim] = std::max(std::max(model->Mesh.Positions[indice[0]][dim], model->Mesh.Positions[indice[1]][dim]), model->Mesh.Positions[indice[2]][dim]);
                }
                return std::make_pair(mn, mx);
            }

            const glm::vec3 & point(int idx) const {
                return model->Mesh.Positions[indice[idx]];
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
            const float EPS  = 1e-6;
            float       tmin = 0, tmax = 1e10f;
            for (int d = 0; d < 3; d++) {
                if (abs(ray.Direction[d]) < EPS) {
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
                if (IntersectTriangle(result, ray, u->face.point(0), u->face.point(1), u->face.point(2))) {
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
        ~BVH() { free(root); }
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
} // namespace VCX::Labs::Rendering