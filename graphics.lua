local graphics={}

local _createwindow=createwindow
local _destroywindow=destroywindow
local _windowshouldclose=windowshouldclose
local _vertexshader=vertexshader
local _fragmentshader=fragmentshader
local _deletesubshader=deletesubshader
local _compileshader=compileshader
local _attributeexists=attributeexists
local _createvertexdata=createvertexdata
local _addvertexdata=addvertexdata
local _deletevertexdata=deletevertexdata
local _createvbo=createvbo
local _deleteshader=deleteshader
local _deletevbo=deletevbo
local _createvao=createvao
local _deletevao=deletevao
local _clearcolor=clearcolor
local _cleardepth=cleardepth
local _swapbuffers=swapbuffers
local _drawmesh=drawmesh
local _windowsize=windowsize
local _viewport=viewport
local _loadtexture=loadtexture
local _createtexture=createtexture
local _textureclear=textureclear
local _createdepthtexture=createdepthtexture
local _createstenciltexture=createstenciltexture
local _deletetexture=deletetexture
local _createframebuffer=createframebuffer
local _deleteframebuffer=deleteframebuffer
local _loadimage=loadimage
local _deleteimage=deleteimage
local _keycallback=keycallback
local _mouseenable=mouseenable
local _mousesetpos=mousesetpos
local _mousegetpos=mousegetpos
local _depthtest=depthtest
local _cullface=cullface
local _bindframebuffer=bindframebuffer
graphics.timedelta=timedelta
graphics.pollevents=pollevents

createwindow=nil
destroywindow=nil
windowshouldclose=nil
vertexshader=nil
fragmentshader=nil
deletesubshader=nil
compileshader=nil
deleteshader=nil
attributeexists=nil
createvertexdata=nil
addvertexdata=nil
deletevertexdata=nil
createvbo=nil
deletevbo=nil
createvao=nil
deletevao=nil
clearcolor=nil
cleardepth=nil
swapbuffers=nil
drawmesh=nil
windowsize=nil
viewport=nil
timedelta=nil
pollevents=nil
loadtexture=nil
createtexture=nil
textureclear=nil
createdepthtexture=nil
createstenciltexture=nil
deletetexture=nil
createframebuffer=nil
deleteframebuffer=nil
loadimage=nil
deleteimage=nil
keycallback=nil
mouseenable=nil
mousepsetos=nil
mousepgetos=nil
depthtest=nil
cullface=nil
bindframebuffer=nil

graphics.window={__metatable={}}

local _key={}

graphics.image={}

graphics.image.load=function(file)
	local imgobj={img=_loadimage(file)}
	local index={}
	local access=function(key)
		if key==_key then
			return imgobj
		end
		assert(false,"no access")
	end
	return setmetatable(index,{__gc=
		function(self)
			_deleteimage(imgobj.img)
		end,__index=setmetatable({access=access},{__index=graphics.image})})
end

local keycode,keyname=table.unpack(require("keys"))

graphics.window.new=function(w,h,title,errfunc,flags)
	local obj={win=_createwindow(w,h,title),vert={},frag={},prog={},vertdata={},vbos={},vaos={},texs={},fbo_stack={}}
	local index={keys={}}
	obj.index=index
	flags=flags or {}
	for i in pairs(keycode) do
		index[i]=false
	end
	local keycb=function(win,key,scancode,action,mods)
		if action>0 then
			index.keys[keyname[key]]=true
		else
			index.keys[keyname[key]]=false
		end
	end
	_keycallback(obj.win,keycb,errfunc)
	local access=function(key)
		if key==_key then
			return obj
		end
		assert(false,"no access")
	end
	return setmetatable(index,{__gc=
		function(self)
			self:close()
		end,__index=setmetatable({access=access},{__index=graphics.window})})
end

graphics.window.bind=function(self)
	local obj=self.access(_key)
	obj.fbo_stack={}
	_bindframebuffer(obj.win)
end

graphics.window.bindback=function(self)
	local obj=self.access(_key)
	local fb=table.remove(obj.fbo_stack)
	if fb==nil then
		_bindframebuffer(obj.win)
	else
		_bindframebuffer(obj.win,fb.buf)
	end
end

graphics.window.close=function(self)
	local obj=self.access(_key)
	if obj.win~=nil then
		_destroywindow(obj.win)
	end
	obj.win=nil
	setmetatable(self,{})
end

graphics.window.mouseenable=function(self,enable)
	local obj=self.access(_key)
	_mouseenable(obj.win,enable)
end

graphics.window.mousesetpos=function(self,x,y)
	local obj=self.access(_key)
	_mousesetpos(obj.win,x,y)
end

graphics.window.mousegetpos=function(self)
	local obj=self.access(_key)
	return _mousegetpos(obj.win)
end

--[[graphics.window.keycallback=function(self,func,errfunc)
	local obj=self.access(_key)
	_keycallback(obj.win,func,errfunc)
end]]

graphics.window.shouldclose=function(self)
	local obj=self.access(_key)
	return _windowshouldclose(obj.win)
end

local shader={}

graphics.window.vertexshader=function(self,src)
	local obj=self.access(_key)
	local shadobj={win=obj,shad=_vertexshader(obj.win,src)}
	local index={}
	local access=function(key)
		if key==_key then
			return shadobj
		end
		assert(false,"no access")
	end
	obj.vert[index]=true
	return setmetatable(index,{__gc=
		function(self)
			local obj=self.access(_key).win
			_deletesubshader(shadobj.shad)
			obj.vert[index]=nil
		end,__index=setmetatable({access=access},{__index=shader})})
end

graphics.window.fragmentshader=function(self,src)
	local obj=self.access(_key)
	local shadobj={win=obj,shad=_fragmentshader(obj.win,src)}
	local index={}
	local access=function(key)
		if key==_key then
			return shadobj
		end
		assert(false,"no access")
	end
	obj.frag[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deletesubshader(shadobj.shad)
			obj.frag[index]=nil
		end,__index=setmetatable({access=access},{__index=shader})})
end

local program={}

graphics.window.shader=function(self,...)
	local obj=self.access(_key)
	local argv={}
	for i,shader in ipairs({...}) do
		if obj.vert[shader]~=nil or obj.frag[shader]~=nil then
			table.insert(argv,shader.access(_key).shad)
		else
			assert(false,"argument "..tostring(i).." must be vertexshader or fragmentshader")
		end
	end
	local progobj={win=obj,shaders={...},prog=_compileshader(obj.win,table.unpack(argv))}
	local access=function(key)
		if key==_key then
			return progobj
		end
		assert(false,"no access")
	end
	local index={}
	obj.prog[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deleteshader(progobj.prog)
			obj.prog[index]=nil
		end,__index=setmetatable({access=access},{__index=program})})
end

local vertex={}

program.vertex=function(self,name)
	local progobj=self.access(_key)
	local obj=progobj.win
	if not _attributeexists(obj.win,progobj.prog,name) then
		return nil
	end
	local vertobj={win=obj,name=name,progobj=progobj,vert=_createvertexdata(obj.win,progobj.prog,name)}
	local index={}
	local access=function(key)
		if key==_key then
			return vertobj
		end
		assert(false,"no access")
	end
	obj.vertdata[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deletevertexdata(vertobj.vert)
			obj.vertdata[index]=nil
		end,__index=setmetatable({access=access},{__index=vertex})})
end

local vbo={}

vertex.vbo=function(self)
	local vertobj=self.access(_key)
	local obj=vertobj.win
	local vboobj={win=obj,name=vertobj.name,progobj=vertobj.progobj,vbo=_createvbo(obj.win,vertobj.vert,name)}
	local index={}
	local access=function(key)
		if key==_key then
			return vboobj
		end
		assert(false,"no access")
	end
	obj.vbos[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deletevbo(vboobj.vbo)
			obj.vbos[index]=nil
		end,__index=setmetatable({access=access},{__index=vbo})})
end

vertex.add=function(self,...)
	local vertobj=self.access(_key)
	_addvertexdata(vertobj.vert,...)
end

local vao={}

graphics.window.vao=function(self,type,...)
	local obj=self.access(_key)
	local argv={...}
	if #argv==0 then
		assert(false,"no vbo available")
	end
	local vaoobj={win=obj,vboobjs={},progobj=argv[1].access(_key).progobj}
	local pvbos={}
	for i,v in ipairs(argv) do
		local vboobj=v.access(_key)
		if vboobj.progobj~=vaoobj.progobj then
			assert(false,"vbo shaders do not match")
		end
		table.insert(pvbos,vboobj.vbo)
		table.insert(vaoobj.vboobjs,vboobj)
	end
	vaoobj.vao=_createvao(obj.win,vaoobj.progobj.prog,type,table.unpack(pvbos))
	local access=function(key)
		if key==_key then
			return vaoobj
		end
		assert(false,"no access")
	end
	local index={}
	obj.vaos[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deletevao(vaoobj.vao)
			obj.vaos[index]=nil
		end,__index=setmetatable({access=access},{__index=vao})})
end

vao.draw=function(self,uniforms)
	local vaoobj=self.access(_key)
	local obj=vaoobj.win
	_drawmesh(obj.win,vaoobj.vao,vaoobj.progobj.prog,uniforms)
end

graphics.window.clearcolor=function(self,...)
	local obj=self.access(_key)
	_clearcolor(obj.win,...)
end

graphics.window.cleardepth=function(self,...)
	local obj=self.access(_key)
	_cleardepth(obj.win)
end

graphics.window.swap=function(self)
	local obj=self.access(_key)
	_swapbuffers(obj.win)
end

graphics.window.getsize=function(self)
	local obj=self.access(_key)
	return _windowsize(obj.win)
end

graphics.window.depthtest=function(self,enable)
	local obj=self.access(_key)
	return _depthtest(obj.win,enable)
end

graphics.window.cullface=function(self,enable)
	local obj=self.access(_key)
	return _cullface(obj.win,enable)
end

graphics.window.viewport=function(self,...)
	local obj=self.access(_key)
	_viewport(obj.win,...)
end

local texture={}

texture.clear=function(self,r,g,b,a)
	local obj=self.access(_key)
	_textureclear(obj.win.win,obj.tex,r,g,b,a)
end

graphics.window.textureload=function(self,image)
	local obj=self.access(_key)
	local texobj={win=obj,tex=_loadtexture(obj.win,image.access(_key).img)}
	local access=function(key)
		if key==_key then
			return texobj
		end
		assert(false,"no access")
	end
	local index={tex=texobj.tex}
	local access=function(key)
		if key==_key then
			return texobj
		end
		assert(false,"no access")
	end
	obj.texs[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deletetexture(texobj.tex)
			obj.texs[index]=nil
		end,__index=setmetatable({access=access},{__index=texture})})
end

graphics.window.texture=function(self,w,h)
	local obj=self.access(_key)
	local texobj={win=obj,tex=_createtexture(obj.win,w,h)}
	local access=function(key)
		if key==_key then
			return texobj
		end
		assert(false,"no access")
	end
	local index={tex=texobj.tex}
	local access=function(key)
		if key==_key then
			return texobj
		end
		assert(false,"no access")
	end
	obj.texs[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deletetexture(texobj.tex)
			obj.texs[index]=nil
		end,__index=setmetatable({access=access},{__index=texture})})
end

graphics.window.depthtexture=function(self,w,h)
	local obj=self.access(_key)
	local texobj={win=obj,tex=_createdepthtexture(obj.win,w,h)}
	local access=function(key)
		if key==_key then
			return texobj
		end
		assert(false,"no access")
	end
	local index={tex=texobj.tex}
	local access=function(key)
		if key==_key then
			return texobj
		end
		assert(false,"no access")
	end
	obj.texs[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deletetexture(texobj.tex)
			obj.texs[index]=nil
		end,__index=setmetatable({access=access},{__index=texture})})
end

graphics.window.stenciltexture=function(self,w,h)
	local obj=self.access(_key)
	local texobj={win=obj,tex=_createstenciltexture(obj.win,w,h)}
	local access=function(key)
		if key==_key then
			return texobj
		end
		assert(false,"no access")
	end
	local index={tex=texobj.tex}
	local access=function(key)
		if key==_key then
			return texobj
		end
		assert(false,"no access")
	end
	obj.texs[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deletetexture(texobj.tex)
			obj.texs[index]=nil
		end,__index=setmetatable({access=access},{__index=texture})})
end

local framebuffer={}

graphics.window.framebuffer=function(self,rgb,depth,stencil)
	local obj=self.access(_key)
	local frameobj={win=obj,buf=_createframebuffer(obj.win,rgb.tex,depth.tex,stencil.tex)}
	local access=function(key)
		if key==_key then
			return frameobj
		end
		assert(false,"no access")
	end
	local index={}
	local access=function(key)
		if key==_key then
			return frameobj
		end
		assert(false,"no access")
	end
	obj.texs[index]=true
	return setmetatable(index,{__gc=
		function(self)
			_deleteframebuffer(frameobj.buf)
			obj.texs[index]=nil
		end,__index=setmetatable({access=access},{__index=framebuffer})})
end

framebuffer.bind=function(self)
	local obj=self.access(_key)
	table.insert(obj.win.fbo_stack,obj)
	_bindframebuffer(obj.win.win,obj.buf)
end

return graphics