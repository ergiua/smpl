////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017, Andrew Dornbush
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     1. Redistributions of source code must retain the above copyright notice
//        this list of conditions and the following disclaimer.
//     2. Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//     3. Neither the name of the copyright holder nor the names of its
//        contributors may be used to endorse or promote products derived from
//        this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////

/// \author Andrew Dornbush

#ifndef SMPL_DISTANCE_MAP_H
#define SMPL_DISTANCE_MAP_H

// standard includes
#include <array>
#include <utility>
#include <vector>

// system includes
#include <Eigen/Dense>
#include <Eigen/StdVector>

// project includes
#include <smpl/grid.h>
#include <smpl/forward.h>
#include <smpl/distance_map/distance_map_interface.h>

#include "detail/distance_map_base.h"

namespace sbpl {

template <typename Derived>
class DistanceMap : public DistanceMapInterface
{
public:

    DistanceMap(
        double origin_x, double origin_y, double origin_z,
        double size_x, double size_y, double size_z,
        double resolution,
        double max_dist);

    double maxDistance() const;

    double getDistance(double x, double y, double z) const;
    double getDistance(int x, int y, int z) const;

    /// \name Required Functions from DistanceMapInterface
    ///@{
    void addPointsToMap(const std::vector<Eigen::Vector3d>& points) override;
    void removePointsFromMap(const std::vector<Eigen::Vector3d>& points) override;
    void updatePointsInMap(
        const std::vector<Eigen::Vector3d>& old_points,
        const std::vector<Eigen::Vector3d>& new_points) override;

    void reset() override;

    int numCellsX() const override;
    int numCellsY() const override;
    int numCellsZ() const override;

    double getUninitializedDistance() const override;

    double getMetricDistance(double x, double y, double z) const override;
    double getCellDistance(int x, int y, int z) const override;

    void gridToWorld(
        int x, int y, int z,
        double& world_x, double& world_y, double& world_z) const override;

    void worldToGrid(
        double world_x, double world_y, double world_z,
        int& x, int& y, int& z) const override;

    bool isCellValid(int x, int y, int z) const;
    ///@}

    friend Derived;

private:

    struct Cell
    {
        int x;
        int y;
        int z;

        int dist;
        int dist_new;
#if SMPL_DMAP_RETURN_CHANGED_CELLS
        int dist_old;
#endif
        Cell* obs;
        int bucket;
        int dir;

        int pos;
    };

    Grid3<Cell> m_cells;

    double m_max_dist;
    double m_inv_res;

    int m_dmax_int;
    int m_dmax_sqrd_int;

    int m_bucket;
    int m_no_update_dir;

    std::array<Eigen::Vector3i, 27> m_neighbors;
    std::array<int, NEIGHBOR_LIST_SIZE> m_indices;
    std::array<std::pair<int, int>, NUM_DIRECTIONS> m_neighbor_ranges;
    std::array<int, NEIGHBOR_LIST_SIZE> m_neighbor_offsets;
    std::array<int, NEIGHBOR_LIST_SIZE> m_neighbor_dirs;

    std::vector<double> m_sqrt_table;

    typedef std::vector<Cell*> bucket_type;
    typedef std::vector<bucket_type> bucket_list;
    bucket_list m_open;

    std::vector<Cell*> m_rem_stack;

    void initBorderCells();

    void updateVertex(Cell* c);

    int distance(const Cell& n, const Cell& s);

    void lower(Cell* s);
    void raise(Cell* s);
    void waveout(Cell* n);
    void propagate();

    void lowerBounded(Cell* s);
    void propagateRemovals();
    void propagateBorder();

    void resetCell(Cell& c) const;
};

} // namespace sbpl

#include "detail/distance_map.hpp"

#endif
