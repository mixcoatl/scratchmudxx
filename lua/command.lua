-- Command listing helpers.

local M = {}

-- List cell metacolor.
local function cell_color(Q, command)
  if command:get_trust() ~= "None" then
    return Q.RESTRICTED
  end
  if command:get_social() ~= nil then
    return Q.SOCIAL
  end
  return Q.TEXT
end

-- Trust-filtered command listing.
function M.list_commands(d, actor, Q, social_only)
  local player = actor:get_player()
  if not player then
    return
  end
  local cells = {}
  for _, id in ipairs(get_commands():get_ids()) do
    local command = get_commands():get(id)
    if command
        and (not social_only or command:get_social() ~= nil)
        and trust_allows(player:get_trust(), command:get_trust()) then
      cells[#cells + 1] = cell_color(Q, command)
        .. string.lower(command:get_name()) .. Q.NORMAL
    end
  end
  d:print_columns(cells)
end

return M
