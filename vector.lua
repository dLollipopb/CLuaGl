local vector={}
vector.__index=vector

vector.__add=function(a,b)
	local v={}
	local min,max=math.min(#a,#b),math.max(#a,#b)
	for i=1,min do
		v[i]=a[i]+b[i]
	end
	if #a>#b then
		for i=min+1,max do
			v[i]=a[i]
		end
	else
		for i=min+1,max do
			v[i]=b[i]
		end
	end
	return setmetatable(v,vector)
end

vector.__sub=function(a,b)
	local v={}
	local min,max=math.min(#a,#b),math.max(#a,#b)
	for i=1,min do
		v[i]=a[i]-b[i]
	end
	if #a>#b then
		for i=min+1,max do
			v[i]=a[i]
		end
	else
		for i=min+1,max do
			v[i]=-b[i]
		end
	end
	return setmetatable(v,vector)
end

vector.__mul=function(a,b)
	local v={}
	local min,max=math.min(#a,#b),math.max(#a,#b)
	for i=1,min do
		v[i]=a[i]*b[i]
	end
	return setmetatable(v,vector)
end

vector.cross=function(a,b)
	local v={a[2]*b[3]-a[3]*b[2],a[3]*b[1]-a[1]*b[3],a[1]*b[2]-a[2]*b[1]}
	return setmetatable(v,vector)
end

vector.new=function(self,...)
	local v={...}
	return setmetatable(v,vector)
end

vector.scale=function(self,s)
	local v={}
	for i=1,#self do
		v[i]=self[i]*s
	end
	return setmetatable(v,vector)
end

vector.abs=function(self)
	local n=0
	for _,i in ipairs(self) do
		n=n+i*i
	end
	return math.sqrt(n)
end

vector.normal=function(self)
	local v={}
	local n=self:abs()
	for i,j in ipairs(self) do
		v[i]=j/n
	end
	return setmetatable(v,vector)
end

vector.neg=function(self)
	local v={}
	for i in ipairs(self) do
		v[i]=-self[i]
	end
	return setmetatable(v,vector)
end

return vector