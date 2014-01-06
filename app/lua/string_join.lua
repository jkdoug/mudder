function string.join(delimiter, list)
  local len = #list
  if len == 0 then 
    return "" 
  end
  local s = list[1]
  for i = 2, len do 
    s = s .. delimiter .. list[i] 
  end
  return s
end
