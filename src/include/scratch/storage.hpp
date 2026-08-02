//! \file storage.hpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#ifndef _SCRATCH_STORAGE_HPP_
#define _SCRATCH_STORAGE_HPP_

#include <scratch/scratch.hpp>
#include <scratch/string.hpp>

namespace Scratch {
namespace Storage {

//! The storage base class. \{
//! \tparam ThingT the domain thing type
template<typename ThingT>
class Storage {
public:
    //! The type of a shared thing pointer.
    using ThingPtr = std::shared_ptr<ThingT>;

    //! The type of the thing map.
    using Map = StringMapCi<ThingPtr>;

    //! Constructor.
    //! \param label the singular type label
    explicit Storage(String label) noexcept :
	    label_(std::move(label)) {
	// Nothing.
    }

    //! Destructor.
    virtual ~Storage() noexcept {
	// Nothing.
    }

    //! Gets the type label.
    const String& GetLabel() const noexcept {
	return label_;
    }

    //! Gets the thing ID from a domain object.
    //! \param thing the thing to read
    //! \remark Default uses \c ThingT::GetName().
    virtual String GetThingId(const ThingT& thing) const noexcept {
	return thing.GetName();
    }

    //! Sets the thing ID on a domain object.
    //! \param thing the thing to update
    //! \param thingId the thing ID to assign
    //! \remark Default uses \c ThingT::SetName().
    virtual void SetThingId(ThingT& thing, const String& thingId) const noexcept {
	thing.SetName(thingId);
    }

protected:
    //! The singular type label.
    //! \sa #GetLabel() const
    String label_;
};
//! \}

}; // namespace Storage
}; // namespace Scratch

#endif // _SCRATCH_STORAGE_HPP_
