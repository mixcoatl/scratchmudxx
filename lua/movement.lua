-- Instance relocation helpers.

local M = {}

-- Default staff poof strings.
local function default_poofs(player)
  local poof_in = player and player:get_poof_in() or ""
  local poof_out = player and player:get_poof_out() or ""
  if poof_in == "" then
    poof_in = "appears with an ear-splitting bang."
  end
  if poof_out == "" then
    poof_out = "disappears in a puff of smoke."
  end
  return poof_in, poof_out
end

-- Move instance into destination room.
function M.move_to(instance, dest)
  local dest_world = dest:get_world()
  if not dest_world then
    return false
  end
  local source_world = instance:get_world()
  if source_world == dest_world then
    return dest:add_child(instance)
  end
  if not source_world then
    return false
  end
  source_world:remove_instance(instance)
  if not dest_world:add_instance(instance) then
    return false
  end
  if not dest:add_child(instance) then
    return false
  end
  return true
end

-- Staff relocate with poofs and look.
function M.poof_to(subject, dest)
  local from = subject:get_parent_room()
  local player = subject:get_player()
  local poof_in, poof_out = default_poofs(player)
  if from then
    action("Movement", A.TO_ROOM, "${S.Name} ${D.Text}", subject, poof_out)
  end
  if not M.move_to(subject, dest) then
    return false
  end
  action("Movement", A.TO_ROOM, "${S.Name} ${D.Text}", subject, poof_in)
  dispatch_command(subject, "look")
  return true
end

return M
