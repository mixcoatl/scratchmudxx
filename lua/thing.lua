-- Thing audit metadata helpers.

local M = {}

-- Resolve audit author from provenance object.
function M.audit_author(source)
  local d
  if source and source.get_descriptor then
    d = source:get_descriptor()
  elseif source and (source.get_user or source.get_character) then
    d = source
  end

  if d and d.get_user then
    local user = d:get_user()
    if user then
      return user:get_name()
    end
  end

  if d and d.get_character then
    local character = d:get_character()
    local player = character and character.get_player and character:get_player()
    if player then
      return player:get_name()
    end
  end

  if source and source.get_player then
    local player = source:get_player()
    if player then
      return player:get_name()
    end
  end

  if source and source.get_name
      and not source.get_descriptor
      and not source.get_user
      and not source.get_character then
    return source:get_name()
  end

  return "system"
end

-- Stamp created and modified metadata on thing.
function M.set_audit_metadata(thing, source, is_new)
  local author = M.audit_author(source)
  local modified = os.time()
  if is_new then
    thing:set_created(modified)
    thing:set_created_by(author)
  end
  thing:set_modified(modified)
  thing:set_modified_by(author)
end

return M
