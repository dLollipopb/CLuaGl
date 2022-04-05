local vector=require("vector")
local matrix=require("matrix")

local quat={}
quat.__index=quat

quat.__mul=function(a,b)
	local q={}
    q[1]=a[4]*b[1]+a[1]*b[4]+a[2]*b[3]-a[3]*b[2]
    q[2]=a[4]*b[2]-a[1]*b[3]+a[2]*b[4]+a[3]*b[1]
    q[3]=a[4]*b[3]+a[1]*b[2]-a[2]*b[1]+a[3]*b[4]
	q[4]=a[4]*b[4]-a[1]*b[1]-a[2]*b[2]-a[3]*b[3]
	return setmetatable(q,quat)
end

quat.identity=function(self,x,y,z,w)
	return setmetatable({0,0,0,1},quat)
end

quat.new=function(self,x,y,z,w)
	return setmetatable({x,y,z,w},quat)
end

quat.rotate=function(self,v,a)
	local vn=v:normal()
	return setmetatable({vn[1]*math.sin(a/2),vn[2]*math.sin(a/2),vn[3]*math.sin(a/2),math.cos(a/2)},quat)
end

quat.normal=function(self)
	return quat:new(vector:new(unpack(self)):normal())
end

quat.invert=function(self)
	return quat:new(-self[1],-self[2],-self[3],self[4])
end

quat.matrix=function(self)
	local xx,xy,xz=self[1]*2*self[1],self[1]*2*self[2],self[1]*2*self[3]
	local yy,yz,zz=self[2]*2*self[2],self[2]*2*self[3],self[3]*2*self[3]
	local wx,wy,wz=self[4]*2*self[1],self[4]*2*self[2],self[4]*2*self[3]
	local m={1-(yy+zz),xy+wz,xz-wy,0,
			xy-wz,1-(xx+zz),yz+wx,0,
			xz+wy,yz-wx,1-(xx+yy),0,
			0,0,0,1}
	return setmetatable(m,matrix)
end

quat.mulvect=function(a,b)
	local q=a*quat:new(b[1],b[2],b[3],0)*a:invert()
	return vector:new(q[1],q[2],q[3])
end

return quat