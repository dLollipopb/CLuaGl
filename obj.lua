local split=function(inputstr, sep)
	sep=sep or '%s'
	local t={} 
	for field,s in string.gmatch(inputstr, "([^"..sep.."]*)("..sep.."?)") do
		table.insert(t,field) 
		if s=="" then
			return t
		end
	end
end

local loadobj=function(file)
	local f=io.open(file,"r")
	local s=""
	local data={}
	while true do
		s=f:read()
		if s==nil then
			break
		end
		local groups=split(s,"%s")
		if data[groups[1]]==nil then
			data[groups[1]]={}
		end
		local groups2={}
		for i=2,#groups do
			if groups[1]=="f" then
				table.insert(groups2,groups[i])
			else
				table.insert(groups2,tonumber(groups[i]))
			end
		end
		table.insert(data[groups[1]],groups2)
	end
	f:close()
	local f=data["f"]
	data["f"]={}
	for _,vv in ipairs(f) do
		for j=1,#vv-2 do
			for i=3,1,-1 do
				if i==1 then
					v=vv[i]
				else
					v=vv[i+j-1]
				end
				local qq=split(v,"/")
				local ww={}
				for _,j in ipairs(qq) do
					table.insert(ww,tonumber(j))
				end
				table.insert(data["f"],ww)
			end
		end
	end
	return data
end

return loadobj