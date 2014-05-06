function table.contains(t, value, recurse)
	for k, v in pairs(t) do
		if k == value then
			return true
    elseif type(k) == "number" and v == value then
      return true
		elseif recurse and type(v) == "table" and table.contains(v, value, recurse) then
      return true
		end
	end
	return false
end
