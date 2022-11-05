#pragma once
#include <vector>
#include <unordered_map>

namespace VCX::Labs::GeometryProcessing {
    struct DCEL {
    public:
        using VertexIdx = std::uint32_t;

        struct Triangle;

        struct HalfEdge {
            VertexIdx To() const { return _to; }
            VertexIdx From() const & { return this[_prev]._to; }

            HalfEdge const * NextEdge() const & { return this + _next; }
            HalfEdge const * PrevEdge() const & { return this + _prev; }
            HalfEdge const * PairEdgeOr(HalfEdge const * defaultValue) const & { return _pair ? (this + _pair) : defaultValue; }
            HalfEdge const * PairEdge() const & { return this + _pair; }

            VertexIdx        OppositeVertex() const & { return reinterpret_cast<VertexIdx const *>(this + 3 - _idx)[_idx]; }
            Triangle const * Face() const & { return reinterpret_cast<Triangle const *>(this - _idx); }
            Triangle const * OppositeFace() const & { return this[_pair].Face(); }
            VertexIdx        PairOppositeVertex() const & { return this[_pair].OppositeVertex(); }

            bool CountOnce() const & { return _pair < 0; }
            int  Index() const { return _idx; }

            HalfEdge()                 = default;
            HalfEdge(HalfEdge &&)      = default;
            HalfEdge(HalfEdge const &) = delete;
            HalfEdge & operator=(HalfEdge const &) = delete;

            friend struct DCEL;

        private:
            VertexIdx   _to;
            int         _pair;
            std::int8_t _prev;
            std::int8_t _next;
            std::int8_t _idx;
            std::int8_t _padding;
        };

        struct Triangle {
        public:
            HalfEdge const *  Edges(int i) const & { return _e + i; }
            VertexIdx const * Indices(int i) const { return _i + i; }

            Triangle const * OppositeFace(int i) const & { return _e[i].OppositeFace(); }
            VertexIdx        OppositeVertex(int i) const & { return _e[i].PairOppositeVertex(); }
            bool             HasOppositeFace(int i) const & { return _e[i].PairEdgeOr(nullptr); }

            int IndiceOfVertex(VertexIdx idx) const {
                if (_i[0] == idx) return 0;
                if (_i[1] == idx) return 1;
                if (_i[2] == idx) return 2;
                return -1;
            }

            bool HasVertex(VertexIdx idx) const {
                return _i[0] == idx || _i[1] == idx || _i[2] == idx;
            }

            Triangle()                 = default;
            Triangle(Triangle &&)      = default;
            Triangle(Triangle const &) = delete;
            Triangle & operator=(Triangle const &) = delete;

            friend struct DCEL;

        private:
            HalfEdge  _e[3];
            VertexIdx _i[3];
        };

        static_assert(sizeof(HalfEdge) == 12U);
        static_assert(sizeof(Triangle) == 48U);

        DCEL(std::uint32_t numVertex, std::uint32_t numFace):
            _verts(numVertex, ~0) {
            _faces.reserve(numFace);
            _pairs.reserve(numFace);
        }
        DCEL() {}

        void ReserveVert(std::uint32_t capacity) { _verts.reserve(capacity); }
        void ReserveFace(std::uint32_t capacity) {
            _faces.reserve(capacity);
            _pairs.reserve(capacity);
        }

        bool AddFaces(std::vector<std::uint32_t> const & faces, bool force = false) {
            for (std::size_t i = 0; i < faces.size(); i += 3U) {
                AddFaceImpl(faces[i + 0U], faces[i + 1U], faces[i + 2U]);
            }
            if (force) return _valid = true;
            if (! _valid) return false;

            std::vector<int8_t> sideCount(_verts.size());
            for (int t = 0; t < _faces.size(); ++t) {
                for (int i = 0; i < 3; ++i) {
                    if (! _faces[t].HasOppositeFace(i)) {
                        VertexIdx j = _faces[t]._i[(i + 1) % 3];
                        VertexIdx k = _faces[t]._i[(i + 2) % 3];
                        if (++sideCount[j] > 2 || ++sideCount[k] > 2) {
                            return _valid = false;
                        }
                        _verts[j] = t * (sizeof(Triangle) / sizeof(HalfEdge)) + i;
                    }
                    auto const * e = _faces[t].Edges(i);
                }
            }
            for (std::size_t vid = 0; vid < _verts.size(); ++vid) {
                if (sideCount[vid] == 1) return _valid = false;
            }
            return true;
        }

        bool IsValid() const { return _valid; }

        struct Vertex {
        public:
            std::pair<VertexIdx, VertexIdx> GetSideNeighbors() const {
                HalfEdge const * e = _e;
                while (true) {
                    e                      = e->PrevEdge();
                    HalfEdge const * trial = e->PairEdgeOr(nullptr);
                    if (! trial) break;
                    e = trial;
                };
                return { _e->To(), e->From() };
            }

            std::vector<VertexIdx> GetNeighbors() const {
                std::vector<VertexIdx> neighbors;
                HalfEdge const *       e = _e;
                do {
                    neighbors.push_back(e->To());
                    e                      = e->PrevEdge();
                    HalfEdge const * trial = e->PairEdgeOr(nullptr);
                    if (! trial) {
                        neighbors.push_back(e->From());
                        break;
                    }
                    e = trial;
                } while (e != _e);
                return neighbors;
            }

            std::vector<Triangle const *> GetFaces() const {
                std::vector<Triangle const *> faces;
                HalfEdge const *              e = _e;
                do {
                    faces.push_back(e->Face());
                    e = e->PrevEdge()->PairEdgeOr(nullptr);
                    if (! e) break;
                } while (e != _e);
                return faces;
            }

            bool IsSide() const {
                return ! _e->PairEdgeOr(nullptr);
            }

            friend struct DCEL;

        private:
            Vertex(HalfEdge const * e):
                _e(e) {}
            HalfEdge const * _e;
        };

        Vertex GetVertex(VertexIdx idx) const {
            return GetEdge(_verts[idx]);
        }

        std::vector<Triangle> const & GetFaces() const {
            return _faces;
        }

        std::vector<HalfEdge const *> GetEdges() const {
            std::vector<HalfEdge const *> results;
            for (std::size_t i = 0; i < _faces.size(); ++i) {
                for (std::size_t j = 0; j < 3U; ++j) {
                    HalfEdge const * e = reinterpret_cast<HalfEdge const *>(_faces.data()) + (i * (sizeof(Triangle) / sizeof(HalfEdge)) + j);
                    if (! e->PairEdgeOr(nullptr) || e->CountOnce()) results.emplace_back(e);
                }
            }
            return results;
        }

        int IndexOf(Triangle const * face) const {
            return face - _faces.data();
        }

    private:
        std::unordered_map<std::size_t, int> _pairs;
        std::vector<int>                     _verts;
        std::vector<Triangle>                _faces;
        bool                                 _valid = true;

        HalfEdge const * GetEdge(int e) const {
            return reinterpret_cast<HalfEdge const *>(_faces.data()) + e;
        }

        int GetPair(VertexIdx vFrom, VertexIdx vTo, int e) {
            std::size_t key  = static_cast<std::size_t>(std::min(vFrom, vTo)) << 32ULL | static_cast<std::size_t>(std::max(vFrom, vTo));
            auto        iter = _pairs.find(key);
            if (iter == _pairs.end()) {
                _pairs[key] = e;
                return 0;
            }
            if (~iter->second) {
                HalfEdge & he = reinterpret_cast<HalfEdge *>(_faces.data())[iter->second];
                he._pair      = e - iter->second;
                _valid        = _valid && he._to == vFrom;
                iter->second  = -1;
                return -he._pair;
            }
            _valid = false;
            return 0;
        }

        Triangle & AddFaceImpl(VertexIdx v0, VertexIdx v1, VertexIdx v2) {
            int        e0      = static_cast<int>(_faces.size()) * (sizeof(Triangle) / sizeof(HalfEdge));
            Triangle & tri     = _faces.emplace_back();
            tri._i[0]          = v0;
            tri._i[1]          = v1;
            tri._i[2]          = v2;
            tri._e[0]._to      = v2;
            tri._e[0]._pair    = GetPair(v1, v2, e0);
            tri._e[0]._prev    = 2;
            tri._e[0]._next    = 1;
            tri._e[0]._idx     = 0;
            tri._e[0]._padding = 0;
            tri._e[1]._to      = v0;
            tri._e[1]._pair    = GetPair(v2, v0, e0 + 1);
            tri._e[1]._prev    = -1;
            tri._e[1]._next    = 1;
            tri._e[1]._idx     = 1;
            tri._e[1]._padding = 0;
            tri._e[2]._to      = v1;
            tri._e[2]._pair    = GetPair(v0, v1, e0 + 2);
            tri._e[2]._prev    = -1;
            tri._e[2]._next    = -2;
            tri._e[2]._idx     = 2;
            tri._e[2]._padding = 0;

            _verts.resize(std::max<std::size_t>({ _verts.size(), v0 + 1, v1 + 1, v2 + 1 }), ~0);

            if (! ~_verts[v0]) _verts[v0] = e0 + 2;
            if (! ~_verts[v1]) _verts[v1] = e0;
            if (! ~_verts[v2]) _verts[v2] = e0 + 1;

            return tri;
        }
    };
}