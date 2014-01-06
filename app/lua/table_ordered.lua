 -- (phpTable) abuse to: http://richard.warburton.it
function table.ordered(...)
  local newTable,keys,values={},{},{}
  local arg = {n=select('#',...),...}
  setmetatable(newTable,{
    __newindex=function(self,key,value)
      if not self[key] then table.insert(keys,key)
      elseif value==nil then -- Handle item delete
        local count=1
        while keys[count]~=key do count = count + 1 end
        table.remove(keys,count)
      end
      values[key]=value -- replace/create
    end,
    __index=function(self,key) return values[key] end,
    __pairs=function(self)
      local count=0
      return function()
        count=count+1
        return keys[count],values[keys[count]]
      end
    end
  })
  for x=1,#arg do
    for k,v in pairs(arg[x]) do newTable[k]=v end
  end
  return newTable
end