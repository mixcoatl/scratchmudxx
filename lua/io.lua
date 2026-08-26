-- Descriptor message helpers.

local M = {}

local indent = string.rep(" ", #"Usage: ")
local prefix = "Usage: "

-- Failed player message.
function M.fail(d, Q, text)
  d:print_format("%s%s%s\r\n", Q.FAILED, text, Q.NORMAL)
end

-- Field prompt.
function M.prompt(d, Q, text)
  d:print_format("%s%s%s:%s ", Q.PROMPT, text, Q.PUNCTUATION, Q.NORMAL)
end

-- Success player message.
function M.okay(d, Q, text)
  d:print_format("%s%s%s\r\n", Q.OKAY, text, Q.NORMAL)
end

-- Menu bad key; re-enter state.
function M.show_unknown_choice(d, Q, state_name)
  M.fail(d, Q, "Unknown choice.")
  d:set_state(state_name)
end

-- Usage lines with aligned continuations.
function M.usage(d, Q, ...)
  local lines
  local first = select(1, ...)
  if type(first) == "table" then
    lines = first
  else
    lines = {first, select(2, ...)}
  end
  if #lines == 0 then
    return
  end
  d:print_format("%s%s%s%s\r\n", Q.FAILED, prefix, lines[1], Q.NORMAL)
  for i = 2, #lines do
    d:print_format("%s%s%s%s\r\n", Q.FAILED, indent, lines[i], Q.NORMAL)
  end
end

return M
