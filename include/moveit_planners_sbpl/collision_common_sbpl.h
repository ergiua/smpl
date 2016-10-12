////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Andrew Dornbush
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////

#ifndef collision_detection_collision_common_sbpl_h
#define collision_detection_collision_common_sbpl_h

// standard includes
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// system includes
#include <ros/ros.h>
#include <moveit/collision_detection/collision_world.h>
#include <moveit/robot_model/robot_model.h>
#include <moveit/robot_state/robot_state.h>
#include <moveit_msgs/CollisionObject.h>
#include <sbpl_collision_checking/robot_collision_model.h>
#include <sbpl_collision_checking/robot_collision_state.h>
#include <sbpl_collision_checking/allowed_collisions_interface.h>

namespace collision_detection {

bool LoadJointCollisionGroupMap(
    ros::NodeHandle& nh,
    std::unordered_map<std::string, std::string>& _jcgm_map);

struct CollisionGridConfig
{
    std::string frame_id;
    double size_x;
    double size_y;
    double size_z;
    double origin_x;
    double origin_y;
    double origin_z;
    double res_m;
    double max_distance_m;
};

void LoadCollisionGridConfig(
    ros::NodeHandle& nh,
    const std::string& param_name,
    CollisionGridConfig& config);

// Represents an efficient pipeline for converting RobotStates into
// RobotCollisionStates for collision checking routines
class CollisionStateUpdater
{
public:

    CollisionStateUpdater();

    bool init(
        const moveit::core::RobotModel& robot,
        const sbpl::collision::RobotCollisionModelConstPtr& rcm);

    void update(const moveit::core::RobotState& state);
    void updateInternal(const moveit::core::RobotState& state);

    const sbpl::collision::RobotCollisionStatePtr& collisionState() { return m_rcs; }
    sbpl::collision::RobotCollisionStateConstPtr collisionState() const { return m_rcs; }

private:

    // robot-only joint variable names
    std::vector<std::string> m_var_names;

    // ...their indices in the robot state
    std::vector<int> m_var_indices;

    // whether the indices are contiguous
    bool m_vars_contiguous;

    // offset into robot state, if variables are contiguous
    int m_vars_offset;

    // corresponding joint variables indices in robot collision model/state
    std::vector<int> m_rcm_var_indices;

    // storage for extract internal robot state, used if not contiguous
    std::vector<double> m_rm_vars;

    // robot collision state joint variables for batch updating
    std::vector<double> m_rcm_vars;

    // the final RobotCollisionState
    sbpl::collision::RobotCollisionStatePtr m_rcs;

    bool extractRobotVariables(
        const moveit::core::RobotModel& model,
        std::vector<std::string>& variable_names,
        std::vector<int>& variable_indices,
        bool& are_variables_contiguous,
        int& variables_offset);

    bool getRobotCollisionModelJointIndices(
        const std::vector<std::string>& joint_names,
        const sbpl::collision::RobotCollisionModel& rcm,
        std::vector<int>& rcm_joint_indices);

    bool getRobotVariableNames(
        const moveit::core::RobotModel& robot_model,
        std::vector<std::string>& var_names,
        std::vector<int>& var_indices);
};

typedef std::shared_ptr<CollisionStateUpdater> CollisionStateUpdaterPtr;
typedef std::shared_ptr<const CollisionStateUpdater> CollisionStateUpdaterConstPtr;

// proxy class to interface with CollisionSpace
class AllowedCollisionMatrixInterface :
    public sbpl::collision::AllowedCollisionsInterface
{
public:

    AllowedCollisionMatrixInterface(const AllowedCollisionMatrix& acm) :
        AllowedCollisionsInterface(),
        m_acm(acm)
    { }

    virtual bool getEntry(
        const std::string& name1,
        const std::string& name2,
        sbpl::collision::AllowedCollision::Type& type) const override
    {
        return m_acm.getEntry(name1, name2, type);
    }

private:

    const AllowedCollisionMatrix& m_acm;
};

bool WorldObjectToCollisionObjectMsgFull(
    const World::Object& object,
    moveit_msgs::CollisionObject& collision_object);

bool WorldObjectToCollisionObjectMsgName(
    const World::Object& object,
    moveit_msgs::CollisionObject& collision_object);

visualization_msgs::MarkerArray
GetCollisionMarkers(sbpl::collision::RobotCollisionState& rcs, int gidx);

} // namespace collision_detection

#endif
