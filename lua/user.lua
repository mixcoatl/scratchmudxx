-- User session helpers.

local M = {}

local io = require("io")

-- Require logged-in user; redirect to login.
function M.ensure_user(d, Q)
  local user = d:get_user()
  if not user then
    io.fail(d, Q, "Not logged in.")
    d:set_state("LoginUser")
    return nil
  end
  return user
end

return M
