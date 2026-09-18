-- Instance relocation helpers.

local M = {}

local io = require("io")
local exit = require("exit")
local room = require("room")

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

-- Whether a mortal walk-in would exceed the room occupant limit.
function M.at_occupant_limit(actor, dest)
  local player = actor:get_player()
  if not player or not player:is_mortal() then
    return false
  end
  local room_proto = dest:get_room()
  if not room_proto then
    return false
  end
  local limit = room_proto:get_occupant_limit()
  if not limit then
    return false
  end
  local count = room.count_mortal_players(dest)
  if limit == 0 then
    return true
  end
  return count >= limit
end

local function dest_indoors(exit_obj)
  local dest = exit_obj:get_target()
  local dest_specials = dest and dest:get_room_specials()
  return dest_specials and dest_specials:get_indoors()
end

-- Enter a keyword or directional exit, or the first open way indoors.
function M.enter(actor, command, line, Q)
  local verb = command:get_name():lower()
  local keyword, direction, bad_dir = exit.parse_move_target(line)
  if bad_dir then
    local d = actor:get_descriptor()
    if d and Q then
      io.fail(d, Q, "That's not a direction!")
    end
    return false
  end
  local from = actor:get_parent_room()
  local specials = from and from:get_room_specials()
  if keyword or direction then
    local exit_obj, dir_name = exit.resolve_exit(
      actor, specials, keyword, direction, verb)
    if not exit_obj then
      return false
    end
    if not dest_indoors(exit_obj) then
      action("Failed", A.TO_CHAR,
        "You can't seem to find anything to enter!", actor)
      return false
    end
    return M.move_direction(actor, dir_name, Q)
  end
  if specials and specials:get_indoors() then
    action("Failed", A.TO_CHAR, "You are already indoors!", actor)
    return false
  end
  if specials then
    local exits = specials:get_exits()
    for _, dir_name in ipairs(exit.direction_order) do
      local exit_obj = exits[dir_name]
      if exit_obj and exit_obj:get_target() and not exit_obj:blocks_passage()
          and dest_indoors(exit_obj) then
        return M.move_direction(actor, dir_name, Q)
      end
    end
  end
  action("Failed", A.TO_CHAR,
    "You can't seem to find anything to enter!", actor)
  return false
end

-- Leave through a keyword or directional exit, or the first open way outdoors.
function M.leave(actor, command, line, Q)
  local verb = command:get_name():lower()
  local keyword, direction, bad_dir = exit.parse_move_target(line)
  if bad_dir then
    local d = actor:get_descriptor()
    if d and Q then
      io.fail(d, Q, "That's not a direction!")
    end
    return false
  end
  local from = actor:get_parent_room()
  local specials = from and from:get_room_specials()
  if keyword or direction then
    if not specials or not specials:get_indoors() then
      action("Failed", A.TO_CHAR,
        "You are outside... where do you want to go!?", actor)
      return false
    end
    local exit_obj, dir_name = exit.resolve_exit(
      actor, specials, keyword, direction, verb)
    if not exit_obj then
      return false
    end
    if dest_indoors(exit_obj) then
      action("Failed", A.TO_CHAR,
        "I see no obvious exits to the outside!", actor)
      return false
    end
    return M.move_direction(actor, dir_name, Q)
  end
  if not specials or not specials:get_indoors() then
    action("Failed", A.TO_CHAR,
      "You are outside... where do you want to go!?", actor)
    return false
  end
  local exits = specials:get_exits()
  for _, dir_name in ipairs(exit.direction_order) do
    local exit_obj = exits[dir_name]
    if exit_obj and exit_obj:get_target() and not exit_obj:blocks_passage()
        and not dest_indoors(exit_obj) then
      return M.move_direction(actor, dir_name, Q)
    end
  end
  action("Failed", A.TO_CHAR,
    "I see no obvious exits to the outside!", actor)
  return false
end

-- Directional move with occupant-limit enforcement.
function M.move_direction(actor, direction, Q)
  local from = actor:get_parent_room()
  if not from then
    return false
  end
  local specials = from:get_room_specials()
  local exit_obj = specials and specials:get_exit(direction)
  if not exit_obj or not exit_obj:get_target() then
    exit.fail_no_direction(actor, direction)
    return false
  end
  if exit_obj:blocks_passage() then
    exit.fail_blocked_passage(actor, direction, exit_obj)
    return false
  end
  local dest = exit_obj:get_target()
  if dest and M.at_occupant_limit(actor, dest) then
    local d = actor:get_descriptor()
    if d and Q then
      io.fail(d, Q, "Sorry, that room is full.")
    end
    return false
  end
  if not actor:can_move(direction) then
    exit.fail_no_direction(actor, direction)
    return false
  end
  if from then
    action("Movement", A.TO_ROOM, "${S.Name} leave${:s} ${D.To}.", actor, direction)
  end
  if actor:move(direction) then
    action("Movement", A.TO_ROOM, "${S.Name} arrive${:s} from ${D.FromPhrase}.", actor, direction)
    dispatch_command(actor, "look")
    return true
  end
  return false
end

-- Staff relocate with poofs and look.
function M.poof_to(subject, dest, Q)
  local room_proto = dest:get_room()
  if room_proto and room_proto:get_private_bit()
      and room.count_players(dest) > 1 then
    local d = subject:get_descriptor()
    if d and Q then
      io.fail(d, Q,
        "There's a private conversation going on in that room!")
    end
    return false
  end

  local from = subject:get_parent_room()
  local player = subject:get_player()
  local poof_in, poof_out = default_poofs(player)
  if from then
    action("Movement", A.TO_ROOM, "${S.Name} ${D.Text}", subject, poof_out)
  end
  if not subject:move_to(dest) then
    return false
  end
  action("Movement", A.TO_ROOM, "${S.Name} ${D.Text}", subject, poof_in)
  dispatch_command(subject, "look")
  return true
end

return M
