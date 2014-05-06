function io.exists(filename)
  local file = io.open(filename)
  if file then
    io.close(file)
    return true
  end
  return false
end
