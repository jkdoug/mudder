function string.explode(div, str)
  if div == "" then
    return false
  end

  local pos = 0
  local arr = {}
  for st,sp in function() return string.find(str, div, pos, true) end do
    table.insert(arr, string.sub(str, pos, st - 1))
    pos = sp + 1
  end
  table.insert(arr, string.sub(str, pos))
  return arr
end
