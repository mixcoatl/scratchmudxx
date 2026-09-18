-- Search command helpers.

local M = {}

local exit = require("exit")

-- Tell the room someone is searching.
local function announce_search(actor, direction)
  if direction then
    action("Notice", A.TO_ROOM,
      "${S.Name} search${:s} to ${D.ToPhrase}.", actor, direction)
  else
    action("Notice", A.TO_ROOM,
      "${S.Name} search${:s} the area.", actor)
  end
end

-- Search a direction for a secret exit.
local function search_direction(actor, direction)
  announce_search(actor, direction)
  local from = actor:get_parent_room()
  local specials = from and from:get_room_specials()
  local exit_obj = specials and specials:get_exit(direction)
  if not exit_obj or not exit_obj:get_target() then
    action("Failed", A.TO_CHAR,
      "Your search reveals nothing.", actor)
    return false
  end
  if not exit_obj:get_secret_bit() then
    action("Failed", A.TO_CHAR,
      "The ${I.Text} to ${D.ToPhrase} doesn't appear to be hidden.",
      actor, direction, exit.title_for_message(exit_obj))
    return false
  end
  if math.random(2) ~= 1 then
    action("Failed", A.TO_CHAR,
      "Your search reveals nothing.", actor)
    return false
  end
  exit_obj:set_secret_bit(false)
  action("Okay", A.TO_CHAR,
    "Your search reveals a hidden ${I.Text} to the ${D.To}!",
    actor, direction, exit.title_for_message(exit_obj))
  action("Exit", A.TO_ROOM,
    "${S.Name} discover${:s} a hidden ${D.Text} to the ${I.To}!",
    actor, exit.title_for_message(exit_obj), direction)
  return true
end

-- Multipurpose search: room, direction, or target.
function M.search(actor, line, Q)
  line = line:match("^%s*(.-)%s*$") or ""
  if line == "" then
    announce_search(actor, nil)
    action("Failed", A.TO_CHAR,
      "You search the area carefully but discover nothing hidden.", actor)
    return false
  end
  local direction = parse_direction(line)
  if direction then
    return search_direction(actor, direction)
  end
  action("Failed", A.TO_CHAR,
    "You can't search ${D.Text}!", actor, line)
  return false
end

return M
