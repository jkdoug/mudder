function table.size(t)
	local i = 0
	for k, v in pairs(t or {}) do
		i = i + 1
	end
	return i
end
