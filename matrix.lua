local matrix={}
matrix.__index=matrix

matrix.__mul=function(b,a)
	local m={}
	for x=1,4 do
		for y=1,4 do
			local v=0
			for i=1,4 do
				v=v+a[i+y*4-4]*b[x+i*4-4]
			end
			m[x+y*4-4]=v
		end
	end
	return setmetatable(m,matrix)
end

matrix.projection=function(self,n,f,fovy,aspect)
	return setmetatable({math.cos(fovy/2)/math.sin(fovy/2)/aspect,0,0,0,
		0,math.cos(fovy/2)/math.sin(fovy/2),0,0,
		0,0,(f+n)/(f-n),-2*f*n/(f-n),
		0,0,1,0},matrix)
end

matrix.scale=function(self,x,y,z)
	return setmetatable({x,0,0,0,0,y,0,0,0,0,z,0,0,0,0,1},matrix)
end

matrix.identity=function(self)
	return setmetatable({1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},matrix)
end

matrix.translate=function(self,x,y,z)
	return setmetatable({1,0,0,x,0,1,0,y,0,0,1,z,0,0,0,1},matrix)
end

return matrix