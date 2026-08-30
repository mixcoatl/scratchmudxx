-- Player preference helpers.

local M = {}

local io = require("io")
local thing = require("thing")

-- Toggle player preference; audit and save.
function M.toggle_preference(actor, d, Q, preference, label)
  local player = actor:get_player()
  if not player then
    return
  end
  if player:has_preference(preference) then
    player:erase_preference(preference)
    io.okay(d, Q, label .. " off.")
  else
    player:add_preference(preference)
    io.okay(d, Q, label .. " on.")
  end
  thing.set_audit_metadata(player, actor)
  get_players():save(player:get_name())
end

return M
