-- Enum picker menu helpers.

local M = {}

local io = require("io")

-- Digit menu over enum names.
function M.list_enum_names(d, title, names)
  if type(names) == "function" then
    names = names()
  end
  d:clear_menu()
  local m = d:ensure_menu()
  m:set_title(title)
  m:add_choices()
  for i, name in ipairs(names) do
    m:add_item(tostring(i), name)
  end
  m:set_prompt("Enter choice")
  d:print_menu()
end

-- Enum menu receive; update draft field.
function M.parse_enum_name(d, Q, line, state_name, names, get_edit, set_value, cancel_empty, cancel_message)
  if line == "" then
    if cancel_empty then
      io.fail(d, Q, cancel_message or "Entry cancelled.")
    end
    d:pop_state()
    return
  end
  if type(names) == "function" then
    names = names()
  end
  local m = d:get_menu()
  local key = m:match_key(line)
  local name = key and names[tonumber(key)]
  if not name then
    io.show_unknown_choice(d, Q, state_name)
    return
  end
  local edit = get_edit()
  if edit then
    set_value(edit, name)
  end
  d:pop_state()
end

return M
