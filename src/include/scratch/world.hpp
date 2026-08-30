//! \file world.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_WORLD_HPP_
    #define _SCRATCH_WORLD_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Core {

// Forward declarations.
class Instance;
class World;

// ScratchMUD types.
using InstancePtr = std::shared_ptr<Instance>;
using InstancePtrSet = std::set<InstancePtr>;
using WorldPtr = std::shared_ptr<World>;

//! The World class. \{
class World : public std::enable_shared_from_this<World> {
public:
    //! Constructor.
    //! \param id the world object identity
    explicit World(const String& id = String()) noexcept;

    //! Destructor.
    virtual ~World() noexcept;

    //! Adds an instance.
    //! \param instance the instance to add
    //! \return \c true if added
    //! \sa #RemoveInstance(const InstancePtr&)
    bool AddInstance(const InstancePtr& instance) noexcept;

    //! Gets the world object identity.
    //! \sa #World(const String&)
    String GetId() const noexcept {
	return id_;
    }

    //! Gets an instance.
    //! \param instanceName the instance name
    //! \return the instance, or \c nullptr
    //! \sa #GetInstances() const
    InstancePtr GetInstance(const String& instanceName) const noexcept;

    //! Gets the instances.
    //! \sa #GetInstance(const String&) const
    InstancePtrSet GetInstances() const noexcept;

    //! Removes an instance.
    //! \param instance the instance to remove
    //! \sa #AddInstance(const InstancePtr&)
    void RemoveInstance(const InstancePtr& instance) noexcept;

protected:
    //! The world object identity.
    String id_;

    //! The instance objects.
    InstancePtrSet instances_;
};
//! \}

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_WORLD_HPP_
