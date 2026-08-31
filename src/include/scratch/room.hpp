//! \file room.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_ROOM_HPP_
#define _SCRATCH_ROOM_HPP_

#include <scratch/direction.hpp>
#include <scratch/room_exit.hpp>
#include <scratch/scratch.hpp>
#include <scratch/string.hpp>
#include <scratch/thing.hpp>

namespace Scratch {
namespace Utility {
class Data;
}; // namespace Utility
}; // namespace Scratch

namespace Scratch {
namespace Core {

class Zone;

using Data = Scratch::Utility::Data;
using DataPtr = std::shared_ptr<Data>;

//! The room class. \{
class Room : public Thing {
public:
    //! Default constructor.
    Room() noexcept;

    //! Copy constructor.
    //! \param other the room to copy
    Room(const Room& other) noexcept;

    //! Destructor.
    virtual ~Room() noexcept;

    //! Default assignment.
    //! \param other the room to assign
    Room& operator=(const Room& other) noexcept;

    //! Gets the description.
    String GetDescription() const noexcept {
	return description_;
    }

    //! Gets an exit.
    //! \param direction the exit direction
    //! \return the exit, or \c nullptr
    ExitPtr GetExit(Direction::DirectionEnum direction) const noexcept;

    //! Gets the defined exits.
    std::map<Direction::DirectionEnum, ExitPtr> GetExits() const noexcept {
	return exits_;
    }

    //! Gets the zone-qualified name.
    String GetQualifiedName() const noexcept;

    //! Gets the title.
    String GetTitle() const noexcept {
	return title_;
    }

    //! Gets the owning zone.
    //! \return the zone, or \c nullptr
    std::shared_ptr<Zone> GetZone() const noexcept {
	return zone_.lock();
    }

    //! Reads this room from a data node.
    //! \param data the room data
    void ReadData(const DataPtr& data) noexcept;

    //! Reads exits from a data node.
    //! \param data the exits data
    void ReadExitsData(const DataPtr& data) noexcept;

    //! Removes an exit.
    //! \param direction the exit direction
    void EraseExit(Direction::DirectionEnum direction) noexcept {
	exits_.erase(direction);
    }

    //! Sets the description.
    //! \param description the description
    void SetDescription(const String& description) {
	description_ = description;
    }

    //! Sets an exit.
    //! \param direction the exit direction
    //! \param exit the exit, or null to erase it
    void SetExit(
	    Direction::DirectionEnum direction,
	    const ExitPtr& exit) noexcept {
	if (exit) {
	    auto committed = std::make_shared<RoomExit>(*exit);
	    committed->SetDirection(direction);
	    exits_[direction] = std::move(committed);
	} else {
	    this->EraseExit(direction);
	}
    }

    //! Sets the title.
    //! \param title the title
    void SetTitle(const String& title) {
	title_ = title;
    }

    //! Sets the owning zone.
    //! \param zone the zone, or null
    void SetZone(const std::shared_ptr<Zone>& zone) noexcept {
	zone_ = zone;
    }

    //! Writes this room to a data node.
    //! \param data the room data
    void WriteData(const DataPtr& data) const noexcept;

    //! Writes exits to a data node.
    //! \param data the exits data
    void WriteExitsData(const DataPtr& data) const noexcept;

private:
    String description_;
    std::map<Direction::DirectionEnum, ExitPtr> exits_;
    String title_;
    std::weak_ptr<Zone> zone_;
};
//! \}

using RoomPtr = std::shared_ptr<Room>;

}; // namespace Core
}; // namespace Scratch

#endif // _SCRATCH_ROOM_HPP_
