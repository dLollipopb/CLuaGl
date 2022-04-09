vector=require("vector")

local gui={}

local main_gui_vertex_shader=[[
#version 450
layout (location=0) in vec2 pos;
layout (location=1) in vec2 texcoord;
out vec2 otexcoord;
void main()
{
	gl_Position=vec4(pos,0.0,1.0);
	otexcoord=texcoord;
}]]

local main_gui_fragment_shader=[[
#version 450
uniform sampler2D tex;
in vec2 otexcoord;
out vec4 ocolor;
void main()
{
	vec4 color=texture(tex,otexcoord);
	//ocolor=vec4(color.rgb,0.5*color.a);
	ocolor=color;
}]]

local gui_vertex_shader=[[
#version 450
layout (location=0) in vec2 pos;
uniform vec2 rpos;
uniform vec2 rsize;
void main()
{
	gl_Position=vec4(vec2(pos.x*rsize.x,pos.y*rsize.y)+rpos,0.0,1.0);
}]]

local gui_fragment_shader=[[
#version 450
uniform vec4 color;
out vec4 ocolor;
void main()
{
	ocolor=color;
}]]

local gui_shader=nil
local gui_vao=nil
local win=nil

gui.new=function(w)
	local obj={win=w,objs={}}
	obj.gui_shader=w:shader(w:vertexshader(gui_vertex_shader),w:fragmentshader(gui_fragment_shader))
	obj.main_gui_shader=w:shader(w:vertexshader(main_gui_vertex_shader),w:fragmentshader(main_gui_fragment_shader))
	local vd=obj.gui_shader:vertex("pos")
	vd:add(0.0,0.0)
	vd:add(1.0,0.0)
	vd:add(1.0,1.0)
	vd:add(1.0,1.0)
	vd:add(0.0,1.0)
	vd:add(0.0,0.0)
	local main_vd=obj.main_gui_shader:vertex("pos")
	main_vd:add(-1.0,-1.0)
	main_vd:add(1.0,-1.0)
	main_vd:add(1.0,1.0)
	main_vd:add(1.0,1.0)
	main_vd:add(-1.0,1.0)
	main_vd:add(-1.0,-1.0)
	local main_texcoord=obj.main_gui_shader:vertex("texcoord")
	main_texcoord:add(0.0,0.0)
	main_texcoord:add(1.0,0.0)
	main_texcoord:add(1.0,1.0)
	main_texcoord:add(1.0,1.0)
	main_texcoord:add(0.0,1.0)
	main_texcoord:add(0.0,0.0)
	local vbo=vd:vbo()
	local main_vbo=main_vd:vbo()
	local main_vbo2=main_texcoord:vbo()
	obj.gui_vao=w:vao("triangles",vbo)
	obj.main_gui_vao=w:vao("triangles",main_vbo,main_vbo2)
	obj.ftex=w:texture(1366,768)
	obj.fdepth=w:depthtexture(1366,768)
	obj.fstencil=w:stenciltexture(1366,768)
	obj.fb=w:framebuffer(obj.ftex,obj.fdepth,obj.fstencil)
	return setmetatable(obj,{__index=gui})
end

gui.rect=function(self,x,y,w,h,color,pos,size)
	local obj={x=x,y=y,w=w,h=h,color=color,pos=pos,size=size}
	local index={}
	self.objs[index]=obj
	return index
end

gui.draw=function(self)
	self.fb:bind()
	self.win:clearcolor(0.0,0.0,0.0,0.0)
	--self.ftex:clear(1.0,0.0,0.0,0.5)
	for _,v in pairs(self.objs) do
		self.win:cleardepth()
		self.gui_vao:draw({color=vector:new(table.unpack(v.color)),rpos=vector:new(table.unpack(v.pos)),rsize=vector:new(table.unpack(v.size))})
	end
	self.win:bind()
	self.win:cleardepth()
	self.main_gui_vao:draw({tex=self.ftex.tex})
end

return gui