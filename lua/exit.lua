-- Live exit helpers.

local M = {}

local io = require("io")

M.direction_order = {
  "East", "Southeast", "South", "Southwest", "West",
  "Northwest", "North", "Northeast", "Up", "Down",
}

-- Title-case each word.
function M.title_case(text)
  return (text:gsub("(%S)(%S*)", function(first, rest)
    return first:upper() .. rest:lower()
  end))
end

-- Authored exit title, if any.
local function authored_title(exit)
  local definition = exit:get_definition()
  return definition and definition:get_title() or ""
end

-- Authored door state, if any.
local function authored_door_state(exit)
  local definition = exit:get_definition()
  local door = definition and definition:get_door_state() or ""
  if door == "" then
    return "None"
  end
  return door
end

-- Whether the exit is a door.
function M.is_door(exit)
  return exit and authored_door_state(exit) ~= "None"
end

-- Whether an exit title matches a keyword.
function M.matches_title(exit, keyword)
  if not exit or not keyword or keyword == "" then
    return false
  end
  local title = authored_title(exit)
  if title == "" then
    return false
  end
  return title:lower() == keyword:lower()
end

-- Find an exit by title, optionally at a direction.
function M.find_by_title(specials, keyword)
  if not specials or not keyword or keyword == "" then
    return nil, nil
  end
  local exits = specials:get_exits()
  for _, direction in ipairs(M.direction_order) do
    local exit_obj = exits[direction]
    if exit_obj and exit_obj:get_target()
        and M.matches_title(exit_obj, keyword) then
      return exit_obj, direction
    end
  end
  return nil, nil
end

-- Find a door exit by title and optional direction.
function M.find_door(specials, keyword, direction)
  if not specials or not keyword or keyword == "" then
    return nil, nil
  end
  local exits = specials:get_exits()
  if direction then
    local exit_obj = exits[direction]
    if not exit_obj or not exit_obj:get_target() then
      return nil, nil
    end
    local title = authored_title(exit_obj)
    if title ~= "" and not M.matches_title(exit_obj, keyword) then
      return nil, nil
    end
    return exit_obj, direction
  end
  return M.find_by_title(specials, keyword)
end

-- Keyword and optional trailing direction from a command line.
local function parse_target(line)
  line = line:match("^%s*(.-)%s*$") or ""
  if line == "" then
    return nil, nil
  end
  local keyword, dir_text = line:match("^(%S+)%s*(%S*)")
  if not dir_text or dir_text == "" then
    return keyword, nil
  end
  if parse_direction(dir_text) then
    return keyword, dir_text
  end
  return line, nil
end

-- Keyword, direction, or both; direction-only when the token parses.
function M.parse_move_target(line)
  line = line:match("^%s*(.-)%s*$") or ""
  if line == "" then
    return nil, nil
  end
  local keyword, dir_text = parse_target(line)
  if not keyword then
    return nil, nil
  end
  local direction = dir_text and parse_direction(dir_text) or nil
  if dir_text and not direction then
    return nil, nil, true
  end
  if not direction then
    local only = parse_direction(keyword)
    if only then
      return nil, only
    end
  end
  return keyword, direction
end

-- Player-visible exit command failure.
local function fail_exit_command(actor, keyword, direction, verb, target)
  if verb and target then
    action("Failed", A.TO_CHAR,
      "You can't ${D.Text} the ${I.Text}!", actor, verb, target)
  elseif keyword and direction then
    action("Failed", A.TO_CHAR,
      "You don't see ${D.An} ${D.Text} to ${I.ToPhrase}!",
      actor, keyword, direction)
  elseif keyword then
    action("Failed", A.TO_CHAR,
      "You don't see ${D.An} ${D.Text} here!",
      actor, keyword)
  elseif direction then
    if verb == "enter" then
      action("Failed", A.TO_CHAR,
        "You can't seem to find anything to enter!", actor)
    elseif verb == "leave" then
      action("Failed", A.TO_CHAR,
        "You can't seem to find a way out!", actor)
    else
      action("Failed", A.TO_CHAR,
        "You don't see anything to ${D.ToPhrase}!", actor, direction)
    end
  elseif verb then
    action("Failed", A.TO_CHAR, "${D.Text} what!?", actor, verb)
  end
end

-- Resolve a live exit by keyword and/or direction.
function M.resolve_exit(actor, specials, keyword, direction, verb)
  if direction then
    local exits = specials and specials:get_exits() or {}
    local exit_obj = exits[direction]
    if not exit_obj or not exit_obj:get_target() then
      fail_exit_command(actor, keyword, direction, verb)
      return nil, nil
    end
    if keyword then
      local definition = exit_obj:get_definition()
      local authored = definition and definition:get_title() or ""
      if authored ~= "" and not M.matches_title(exit_obj, keyword) then
        fail_exit_command(actor, keyword, direction, verb)
        return nil, nil
      end
    end
    return exit_obj, direction
  end
  if keyword then
    local exit_obj, dir_name = M.find_by_title(specials, keyword)
    if not exit_obj then
      fail_exit_command(actor, keyword, nil, verb)
      return nil, nil
    end
    return exit_obj, dir_name
  end
  return nil, nil
end

-- Live door state for look labels.
local function display_door_state(exit)
  local door_state = exit:get_door_state()
  if door_state == nil or door_state == "" then
    door_state = "None"
  end
  if door_state == "Locked" then
    door_state = "Closed"
  end
  return door_state
end

-- Exit title for player messages.
function M.title_for_message(exit)
  local title = authored_title(exit)
  if title ~= "" then
    return title:lower()
  end
  if authored_door_state(exit) ~= "None" then
    return "door"
  end
  return "exit"
end

-- Exit list body from door state, title, and direction.
local function format_exit_body(direction, exit)
  local door_title = authored_title(exit)
  local door_state = display_door_state(exit)
  local join_table = {}
  if door_state == "Broken" then
    table.insert(join_table, "Broken")
  elseif door_state ~= "None" then
    table.insert(join_table, door_state)
  end
  if door_title ~= "" then
    table.insert(join_table, M.title_case(door_title))
  elseif door_state ~= "None" then
    table.insert(join_table, "Door")
  end
  table.insert(join_table, direction)
  return table.concat(join_table, " ")
end

-- Exit list label from door state, title, direction, and lighting.
function M.format_exit_label(direction, exit, Q, source_room_inst)
  local body = format_exit_body(direction, exit)
  if not Q then
    return body
  end
  if exit:blocks_passage() or not source_room_inst then
    return Q.EXIT .. body .. Q.NORMAL
  end
  -- Lazy load: room.lua requires exit at module scope.
  local room_mod = require("room")
  local dest = exit:get_target()
  local here_dark = room_mod.is_dark(source_room_inst)
  local prefix = ""
  local there_dark = dest and room_mod.is_dark(dest)
  if not here_dark and there_dark then
    prefix = Q.NIGHTTIME .. "Dark "
  elseif here_dark and not there_dark then
    prefix = Q.DAYTIME .. "Illuminated "
  end
  return prefix .. Q.EXIT .. body .. Q.NORMAL
end

-- Visible exit labels in compass order.
function M.visible_exit_labels(Q, source_room_inst)
  local labels = {}
  if not source_room_inst then
    return labels
  end
  local specials = source_room_inst:get_room_specials()
  if not specials then
    return labels
  end
  local exits = specials:get_exits()
  for _, direction in ipairs(M.direction_order) do
    local exit_obj = exits[direction]
    if exit_obj and not exit_obj:get_secret_bit() then
      labels[#labels + 1] = M.format_exit_label(
        direction, exit_obj, Q, source_room_inst)
    end
  end
  return labels
end

-- No exit or other movement blockers.
function M.fail_no_direction(actor, direction)
  action("Failed", A.TO_CHAR,
    "You can't seem to go ${D.To}!", actor, direction)
end

-- Closed or locked exit.
function M.fail_blocked_passage(actor, direction, exit)
  action("Failed", A.TO_CHAR,
    "The ${I.Text} to ${D.ToPhrase} seems to be closed.",
    actor, direction, M.title_for_message(exit))
end

-- Shared open/close target resolution and failure handling.
local function begin_door_command(actor, command, line, Q)
  local verb = command:get_name():lower()
  local keyword, direction, bad_dir = M.parse_move_target(line)
  if bad_dir then
    local d = actor:get_descriptor()
    if d and Q then
      io.fail(d, Q, "That's not a direction!")
    end
    return nil
  end
  if not keyword and not direction then
    fail_exit_command(actor, nil, nil, verb)
    return nil
  end
  local from = actor:get_parent_room()
  local specials = from and from:get_room_specials()
  local exit_obj, dir_name = M.resolve_exit(actor, specials, keyword, direction)
  if not exit_obj then
    return nil
  end
  local exit_name = M.title_for_message(exit_obj)
  if not M.is_door(exit_obj) then
    local target = keyword and authored_title(exit_obj) ~= "" and exit_name or nil
    fail_exit_command(actor, keyword, direction, verb, target)
    return nil
  end
  return exit_obj, dir_name, exit_name, verb
end

-- Close a door exit.
function M.close_door(actor, command, line, Q)
  local exit_obj, dir_name, exit_name, verb = begin_door_command(
    actor, command, line, Q)
  if not exit_obj then
    return false
  end
  local state = exit_obj:get_door_state()
  if state == "Locked" or state == "Closed" then
    action("Failed", A.TO_CHAR,
      "The ${I.Text} to ${D.ToPhrase} is already closed!",
      actor, dir_name, exit_name)
    return false
  end
  if state ~= "Open" then
    fail_exit_command(actor, nil, nil, verb, exit_name)
    return false
  end
  exit_obj:set_door_state("Closed")
  action("Exit", A.TO_ALL | A.NOREPEAT,
    "${S.Name} close${:s} the ${I.Text} to ${D.ToPhrase}.",
    actor, dir_name, exit_name)
  return true
end

-- Open a door exit.
function M.open_door(actor, command, line, Q)
  local exit_obj, dir_name, exit_name, verb = begin_door_command(
    actor, command, line, Q)
  if not exit_obj then
    return false
  end
  local state = exit_obj:get_door_state()
  if state == "Open" or state == "Broken" then
    action("Failed", A.TO_CHAR,
      "The ${I.Text} to ${D.ToPhrase} is already open!",
      actor, dir_name, exit_name)
    return false
  end
  if state == "Locked" then
    action("Failed", A.TO_CHAR,
      "The ${D.Text} seems to be locked!", actor, exit_name)
    return false
  end
  exit_obj:set_door_state("Open")
  action("Exit", A.TO_ALL | A.NOREPEAT,
    "${S.Name} open${:s} the ${I.Text} to ${D.ToPhrase}.",
    actor, dir_name, exit_name)
  return true
end

return M
