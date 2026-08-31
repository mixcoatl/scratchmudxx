-- Room look helpers.

local M = {}

local io = require("io")

-- Player or instance display name.
local function format_name(player_inst)
  local player = player_inst:get_player()
  if player then
    return player:get_name()
  end
  return player_inst:get_name()
end

-- Builder+ instance and qualified room names for title line.
local function get_staff_room_labels(actor, room_inst)
  local player = actor:get_player()
  if not player or not trust_allows(player:get_trust(), "Builder") then
    return
  end
  return "%" .. room_inst:get_name(), room_inst:get_qualified_room_name()
end

-- Oxford-comma name list for NOTICE lines.
local function format_oxford_names(Q, names)
  local n = #names
  if n == 0 then
    return ""
  end
  if n == 1 then
    return Q.NAME .. names[1]
  end
  local labels = {}
  for i, name in ipairs(names) do
    labels[i] = Q.NAME .. name
  end
  if n == 2 then
    return table.concat(labels, Q.PUNCTUATION .. " and " .. Q.NOTICE)
  end
  return table.concat(labels, Q.PUNCTUATION .. ", " .. Q.NOTICE, 1, n - 1)
    .. Q.PUNCTUATION .. ", and " .. Q.NOTICE .. labels[n]
end

-- Room occupant notice.
local function list_room_occupants(d, Q, names)
  if #names == 0 then
    return
  end
  d:print_format("%sYou notice %s%s here.%s\r\n",
    Q.NOTICE, format_oxford_names(Q, names), Q.NOTICE, Q.NORMAL)
end

-- Container contents notice.
local function list_container_contents(d, Q, container_name, names)
  if #names == 0 then
    return
  end
  d:print_format("%sInside %s%s%s you see %s%s.%s\r\n",
    Q.NOTICE, Q.NAME, container_name, Q.NOTICE,
    format_oxford_names(Q, names), Q.NOTICE, Q.NORMAL)
end

-- Room exit directions.
local function list_room_exits(d, Q, directions)
  if #directions == 0 then
    return
  end
  local labels = {}
  for i, dir in ipairs(directions) do
    labels[i] = Q.EXIT .. dir .. Q.NORMAL
  end
  local exit_text = table.concat(labels, Q.PUNCTUATION .. ", " .. Q.NORMAL)
  d:print_format("%sExits%s:%s %s%s.%s\r\n",
    Q.EXIT, Q.PUNCTUATION, Q.NORMAL, exit_text, Q.PUNCTUATION, Q.NORMAL)
end

-- Look line mode and target text.
local function parse_look_line(line)
  local rest = line
  local mode = "at"
  if rest:match("^inside%s+") then
    mode = "in"
    rest = rest:match("^inside%s+(.+)$")
  elseif rest:match("^in%s+") then
    mode = "in"
    rest = rest:match("^in%s+(.+)$")
  elseif rest:match("^at%s+") then
    rest = rest:match("^at%s+(.+)$")
  end
  if not rest or rest == "" then
    return nil, nil
  end
  return mode, rest
end

-- Look command dispatcher.
function M.look(d, Q, actor, line)
  local room_inst = actor:get_parent_room()
  if line == "" then
    return M.look_at_room(d, Q, actor, room_inst)
  end
  local mode, target = parse_look_line(line)
  if not target then
    io.fail(d, Q, "You don't see that here.")
    return false
  end
  local words = {}
  for word in target:gmatch("%S+") do
    words[#words + 1] = word
  end
  if mode == "in" then
    local instance = actor:find(words)
    if not instance then
      io.fail(d, Q, "You don't see that here.")
      return false
    end
    return M.look_inside_instance(d, Q, instance)
  end
  if parse_direction(target) then
    return M.look_in_direction(d, Q, actor, room_inst, target)
  end
  local instance = actor:find(words)
  if not instance then
    io.fail(d, Q, "You don't see that here.")
    return false
  end
  return M.look_at_instance(d, Q, actor, instance)
end

-- Single instance look display.
function M.look_at_instance(d, Q, actor, instance)
  if instance:get_room() then
    return M.look_at_room(d, Q, actor, instance)
  end
  if instance == actor then
    action("Peek", A.TO_NOTVICT, "${S.Name} look${:s} at ${S.Reflexive}.", actor)
  else
    action("Peek", A.TO_ROOM, "${S.Name} look${:s} at ${D.Name}.", actor, instance)
  end
  local name = format_name(instance)
  if name == "" then
    io.fail(d, Q, "You don't see that here.")
    return false
  end
  d:print_format("%s%s%s\r\n", Q.NAME, name, Q.NORMAL)
  return true
end

-- Full room look display.
function M.look_at_room(d, Q, actor, room_inst)
  if not room_inst then
    io.fail(d, Q, "Nothing special there...")
    return false
  end
  local room = room_inst:get_room()
  if not room then
    return false
  end
  local self_player = actor:get_player()
  local self_name = self_player and self_player:get_name() or ""
  local title = room:get_title()
  local instance_name, ref = get_staff_room_labels(actor, room_inst)
  if instance_name ~= nil then
    if title ~= "" and ref ~= "" then
      d:print_format("%s[%s%s%s] %s%s%s %s[%s%s%s]%s\r\n",
        Q.PUNCTUATION, Q.NAME, instance_name, Q.PUNCTUATION,
        Q.TITLE, title, Q.TITLE,
        Q.PUNCTUATION, Q.TEXT, ref, Q.PUNCTUATION, Q.NORMAL)
    elseif title ~= "" then
      d:print_format("%s[%s%s%s] %s%s%s\r\n",
        Q.PUNCTUATION, Q.NAME, instance_name, Q.PUNCTUATION,
        Q.TITLE, title, Q.NORMAL)
    elseif ref ~= "" then
      d:print_format("%s[%s%s%s] %s[%s%s%s]%s\r\n",
        Q.PUNCTUATION, Q.NAME, instance_name, Q.PUNCTUATION,
        Q.PUNCTUATION, Q.TEXT, ref, Q.PUNCTUATION, Q.NORMAL)
    else
      d:print_format("%s[%s%s%s]%s\r\n",
        Q.PUNCTUATION, Q.NAME, instance_name, Q.PUNCTUATION, Q.NORMAL)
    end
  elseif title ~= "" then
    d:print_format("%s%s%s\r\n", Q.TITLE, title, Q.NORMAL)
  end
  local description = room:get_description()
  if description ~= "" then
    description = description:gsub("[\r\n]+$", "")
    d:print_format("%s%s%s\r\n", Q.DESCRIPTION, description, Q.NORMAL)
  end
  local others = {}
  for _, occupant in ipairs(room_inst:get_contents()) do
    local player = occupant:get_player()
    if player and player:get_name() ~= self_name then
      others[#others + 1] = format_name(occupant)
    end
  end
  list_room_occupants(d, Q, others)
  local specials = room_inst:get_room_specials()
  if specials then
    local exits = specials:get_exits()
    local labels = {}
    for dir, exit in pairs(exits) do
      if exit and exit:get_target() then
        labels[#labels + 1] = dir
      end
    end
    table.sort(labels)
    list_room_exits(d, Q, labels)
  end
  return true
end

-- Directional peek and adjacent room look.
function M.look_in_direction(d, Q, actor, room_inst, direction_text)
  local specials = room_inst and room_inst:get_room_specials()
  local direction = parse_direction(direction_text)
  if not direction then
    io.fail(d, Q, "Nothing special there...")
    return false
  end
  local exit = specials and specials:get_exit(direction)
  if not exit or not exit:get_target() then
    io.fail(d, Q, "You don't see anything that way.")
    return false
  end
  action("Peek", A.TO_ROOM, "${S.Name} peek${:s} ${D.Text}.", actor, direction:lower())
  local target_inst = exit:get_target()
  local opp_name = get_opposite_direction(direction):lower()
  for _, occupant in ipairs(target_inst:get_contents()) do
    if occupant:get_player() and occupant:get_descriptor() then
      action("Peek", A.TO_VICT, "${S.Name} peek${:s} in from the ${I.Text}.", actor, occupant, opp_name)
    end
  end
  return M.look_at_room(d, Q, actor, target_inst)
end

-- Container contents look display.
function M.look_inside_instance(d, Q, container)
  local names = {}
  for _, child in ipairs(container:get_contents()) do
    names[#names + 1] = format_name(child)
  end
  if #names == 0 then
    io.fail(d, Q, "It is empty.")
    return false
  end
  list_container_contents(d, Q, format_name(container), names)
  return true
end

return M
