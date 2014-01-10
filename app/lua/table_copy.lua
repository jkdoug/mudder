function table.deepcopy(t)
  local lookup_table = {}
  assert(type(t) == "table", "You must specify a table to copy")

  local function _copy(t)
    if type(t) ~= "table" then
      return t
    elseif lookup_table[t] then
      return lookup_table[t]
    end

    local new_table = {}
    lookup_table[t] = new_table

    for index,value in pairs(t) do
      new_table[_copy(index)] = _copy(value)
    end

    return setmetatable(new_table, getmetatable(t))
  end

  return _copy(t)
end

function table.shallowcopy(t)
  assert(type(t) == "table", "You must specify a table to copy")
    
  local result = {}

  for k,v in pairs(t) do
    result[k] = v
  end

  return setmetatable(result, getmetatable(t))
end
