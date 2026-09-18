-- Room look helpers.

local M = {}

local io = require("io")
local exit = require("exit")

local lighting_none = parse_lighting("None")

-- Builder+ dark-room vision bypass.
local function can_see_in_dark(actor)
  local player = actor:get_player()
  return player and trust_allows(player:get_trust(), "Builder")
end

-- Pitch-black room message.
local function show_pitch_black(d, Q)
  d:print_format("%sIt's pitch black!%s\r\n", Q.NOTICE, Q.NORMAL)
end

-- Whether the actor can see room contents.
function M.can_see_room(actor, room_inst)
  return not M.is_dark(room_inst) or can_see_in_dark(actor)
end

-- Player characters in direct room contents.
function M.count_players(room_inst)
  if not room_inst then
    return 0
  end
  local count = 0
  for _, occupant in ipairs(room_inst:get_contents()) do
    if occupant:get_player() then
      count = count + 1
    end
  end
  return count
end

-- Mortal player characters in direct room contents.
function M.count_mortal_players(room_inst)
  if not room_inst then
    return 0
  end
  local count = 0
  for _, occupant in ipairs(room_inst:get_contents()) do
    local player = occupant:get_player()
    if player and player:is_mortal() then
      count = count + 1
    end
  end
  return count
end

-- Whether the room is dark.
function M.is_dark(room_inst)
  local proto = room_inst and room_inst:get_room()
  if not proto then
    return false
  end
  return proto:get_lighting() == lighting_none
end

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

-- Room exit list.
local function list_room_exits(d, Q, labels)
  if #labels == 0 then
    return
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
  if room_inst and not M.can_see_room(actor, room_inst) then
    show_pitch_black(d, Q)
    return true
  end
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
    if not instance or instance:get_parent_room() ~= actor:get_parent_room() then
      io.fail(d, Q, "You don't see that here.")
      return false
    end
    return M.look_inside_instance(d, Q, instance)
  end
  if parse_direction(target) then
    return M.look_in_direction(d, Q, actor, room_inst, target)
  end
  local instance = actor:find(words)
  if not instance or instance:get_parent_room() ~= actor:get_parent_room() then
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
  if not M.can_see_room(actor, room_inst) then
    show_pitch_black(d, Q)
    return true
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
  list_room_exits(d, Q, exit.visible_exit_labels(Q, room_inst))
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
  local exit_obj = specials and specials:get_exit(direction)
  if not exit_obj or not exit_obj:get_target() then
    io.fail(d, Q, "You don't see anything that way.")
    return false
  end
  if exit_obj:blocks_passage() then
    exit.fail_blocked_passage(actor, direction, exit_obj)
    return false
  end
  action("Peek", A.TO_ROOM, "${S.Name} peek${:s} ${D.To}.", actor, direction)
  local target_inst = exit_obj:get_target()
  for _, occupant in ipairs(target_inst:get_contents()) do
    if occupant:get_player() and occupant:get_descriptor() then
      action("Peek", A.TO_VICT, "${S.Name} peek${:s} in from ${I.FromPhrase}.", actor, occupant, direction)
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
