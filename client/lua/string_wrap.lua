function string.wrap(line, length)
  local lines = {}
  while #line > length do
    local col = string.find (line:sub (1, length), "[%s,][^%s,]*$")
    if not col or col < 3 then
      col = length
    end

    table.insert(lines, line:sub (1, col))
    line = line:sub(col + 1)
  end
  table.insert(lines, line)
  return lines
end
