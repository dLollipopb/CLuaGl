graphics=require("graphics")
matrix=require("matrix")
quat=require("quat")
vector=require("vector")
loadobj=require("obj")
gui=require("gui")
keycode,keyname=table.unpack(require("keys"))

tcc=createtccstate()

qq=[[

typedef struct vec3
{
	float x,y,z;
}vec3;

vec3 vec3(float x,float y,float z)
{
	vec3 v=new vec3;
	v.x=x;
	v.y=y;
	v.z=z;
	return v;
}

vec3 __add(vec3 a,vec3 b)
{
	return vec3(a.x+b.x,a.y+b.y,a.z+b.z);
}

#vec3 2
#__add 1
#v 1
#v2 0

int main()
{
	v=vec3(1.0,1.0,0.5);
	v2=v+vec3(0.1,0.1,0.1);
	return 0;
}

]]

sadd=[[
extern lua_pushinteger(void*,int);
int add(int a,int b)
{
	return a+b;
}
int main(void* L)
{
	lua_pushinteger(L,add(2,3));
	return 1;
}
]]
--addsymbol(tcc,"lua_pushinteger",lua_pushinteger)
--compile(tcc,sadd)
--print(getsymbol(tcc,"main")())

vertex_shader_text=[[
#version 450
layout (location=0) in vec3 pos;
layout (location=1) in vec3 norm;
layout (location=2) in vec2 texcoord;
out vec2 otexcoord;
out vec3 onormal;
out vec3 FragPos;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
void main()
{
	gl_Position=P*V*M*vec4(pos,1);
	FragPos = vec3(M * vec4(pos, 1.0f));
	otexcoord=texcoord;
	onormal=mat3(transpose(inverse(M))) * norm;
}]]

fragment_shader_text=[[
#version 450
uniform sampler2D tex;
uniform sampler2D tex_spec;
in vec2 otexcoord;
in vec3 onormal;
in vec3 FragPos;
out vec4 color;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 viewPos;
void main()
{
	vec3 viewDir = normalize(-viewPos - FragPos);
	vec3 norm = normalize(onormal);
	vec3 lightDir = normalize(vec3(1,1,1));
	vec3 reflectDir = reflect(-lightDir, norm);
	float diff = max(dot(norm, lightDir), 0.0);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
	vec3 lightColor = vec3(1,1,1);
	vec3 specular = 5.0 * spec * lightColor * texture(tex_spec,otexcoord).xyz;
	vec3 diffuse = diff * lightColor;
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;
	vec3 result = ( ambient + diffuse + specular) * texture(tex,otexcoord).xyz;
	color=vec4(result,1.0f);
}]]

vertex_shader_text2=[[
#version 450
layout (location=0) in vec3 pos;
layout (location=1) in vec3 norm;
layout (location=2) in vec2 texcoord;
out vec3 onormal;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
void main()
{
}]]

fragment_shader_text2=[[
#version 450
uniform sampler2D tex;
in vec3 onormal;
out vec4 FragColor;
void main()
{
   	FragColor=vec4(dot(normalize(vec3(-1,1,-1)),normalize(onormal))*vec3(1,1,1),1);
}]]

vertex_shader_text2=[[
#version 450
layout (location=0) in vec3 pos;
layout (location=1) in vec3 norm;
//layout (location=2) in vec2 texcoord;
//out vec2 otexcoord;
out vec3 onormal;
//out vec3 FragPos;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
void main()
{
	gl_Position=P*V*M*vec4(pos,1);
	//FragPos = vec3(M * vec4(pos, 1.0f));
	//otexcoord=texcoord;
	onormal=mat3(transpose(inverse(M))) * norm;
}]]

fragment_shader_text2=[[
#version 450
//uniform sampler2D tex;
in vec2 otexcoord;
in vec3 onormal;
//in vec3 FragPos;
out vec4 color;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 sun;
//uniform vec3 viewPos;
void main()
{
	vec3 norm = normalize(onormal);
	//vec3 lightDir = normalize(vec3(1,1,1));
	float diff = max(dot(norm, sun), 0.0);
	//vec3 result = diff * texture(tex,otexcoord).xyz;
	vec3 result = diff * vec3(1.0,1.0,1.0);
	color=vec4(result,1.0);
}]]

loadobjdata=function(w,shader,file,bvt,ld)
	local data=loadobj(file)
	local pos,texcoord,normal={},{},{}

	if bvt==nil then
		bvt=true
	end

	local gv,gvt,gvn=1,1,1
	for _,i in ipairs(data["f"]) do
		gv=i[1] or gv
		gvt=i[2] or gvt
		gvn=i[3] or gvn
		table.insert(pos,data["v"][gv])
		if bvt then
			table.insert(texcoord,data["vt"][gvt])
		end
		table.insert(normal,data["vn"][gvn])
	end

	print(file,#pos/3,ld)


	local vd=shader:vertex("pos")
	if vd~=nil then
		for _,i in ipairs(pos) do
			vd:add(table.unpack(i))
		end
		vbo=vd:vbo()
	end

	if bvt then
		local texc=shader:vertex("texcoord")
		if texc~=nil then
			for _,i in ipairs(texcoord) do
				texc:add(table.unpack(i))
			end
			vbo2=texc:vbo()
		end
	end

	local norm=shader:vertex("norm")
	if norm~=nil then
		for _,i in ipairs(normal) do
			norm:add(table.unpack(i))
		end
		vbo3=norm:vbo()
	end
	if bvt then
		return w:vao("triangles",vbo,vbo2,vbo3)
	end
	return w:vao("triangles",vbo,vbo3)
end

errfunc=function(str)
	print(str)
end

w=graphics.window.new(1366,768,"title",errfunc)
g=gui.new(w)
shader=w:shader(w:vertexshader(vertex_shader_text),w:fragmentshader(fragment_shader_text))
shader2=w:shader(w:vertexshader(vertex_shader_text2),w:fragmentshader(fragment_shader_text2))

--vao4=loadobjdata(w,shader2,"robot.obj",false)
--vao3=loadobjdata(w,shader2,"monster.obj")
--vao22=loadobjdata(w,shader2,"skel.obj")--,nil,1.0)
--vao2=loadobjdata(w,shader2,"skel.obj",false)
vao1=loadobjdata(w,shader2,"monster.obj",false)

tex1=w:textureload(graphics.image.load("monster.bmp"))
tex11=w:textureload(graphics.image.load("box_spec.png"))
--tex2=w:texture(graphics.image.load("btr8chas.jpg"))
--tex3=w:texture(graphics.image.load("monster.bmp"))

dt=graphics:timedelta()
width,height,t=0,0,1
vpos=vector:new(0,0,0)
vquat=quat:identity()
mouseview=false
speed=0.5
rect=g:rect(0,0,100,100,{0.0,0.0,0.0,0.9},{0.0,0.0},{0.5,0.7})
rect=g:rect(0,0,100,100,{0.0,0.0,0.0,0.9},{-0.8,-1.0},{0.5,0.7})
render=function(...)
	while not w:shouldclose() do
		dt=graphics:timedelta()
		t=t+dt
		sun=vector:new(math.cos(t),math.sin(t),0.7):normal()
		if w.keys["q"] then
			mouseview=false
			w:mouseenable(true)
		end
		if w.keys["e"] then
			mouseview=true
			w:mouseenable(false)
			w:mousesetpos(0,0)
		end
		if mouseview then
			local mx,my=w:mousegetpos()
			w:mousesetpos(0,0)
			vquat=vquat*quat:rotate(vector:new(0,1,0),mx/100)*quat:rotate(vector:new(1,0,0),my/100)
		end
		if w.keys["w"] then
			vpos=vpos-vquat:mulvect(vector:new(0,0,dt*speed))
		end
		if w.keys["s"] then
			vpos=vpos-vquat:mulvect(vector:new(0,0,-dt*speed))
		end
		if w.keys["d"] then
			vpos=vpos-vquat:mulvect(vector:new(dt*speed,0,0))
		end
		if w.keys["a"] then
			vpos=vpos-vquat:mulvect(vector:new(-dt*speed,0,0))
		end
		if w.keys["space"] then
			vpos=vpos-vquat:mulvect(vector:new(0,dt*speed,0))
		end
		if w.keys["left_control"] then
			vpos=vpos-vquat:mulvect(vector:new(0,-dt*speed,0))
		end
		width,height=w:getsize()
		w:viewport(0,0,width,height)
		--model=quat:rotate(vector:new(0.0,1.0,0.0),t):matrix()*matrix:translate(0,0,3)
		view=matrix:translate(table.unpack(vpos))*vquat:matrix()
		w:bind()
		w:cleardepth()
		w:clearcolor(0.2,0.3,0.7,1.0)
		proj=matrix:projection(0.1,1000,math.pi/2,width/height)
		model=matrix:scale(0.1,0.1,0.1)*matrix:translate(0,0,0)
		vao1:draw({M=model,V=view,P=proj,tex=tex1.tex,viewPos=vpos,tex_spec=tex11.tex,sun=sun})
		g:draw()
		--model=matrix:scale(0.1,0.1,0.1)*matrix:translate(3,0,0)
		--vao22:draw({M=model,V=view,P=proj,tex=tex2.tex,viewPos=vpos,tex_spec=tex11.tex})
		--model=matrix:scale(0.1,0.1,0.1)*matrix:translate(0,0,0)
		--vao2:draw({M=model,V=view,P=proj,tex=tex2.tex,viewPos=vpos,tex_spec=tex11.tex})
		--for i=1,10 do
		--	model=matrix:scale(0.1,0.1,0.1)*quat:rotate(vector:new(0.0,1.0,0.0),t):matrix()*matrix:translate(i-3,0,3+i)
		--	vao2:draw({M=model,V=view,P=proj,tex=tex2.tex,viewPos=vpos,tex_spec=tex11.tex})
		--end
		--for i=1,10 do
		--	model=quat:rotate(vector:new(0.0,1.0,0.0),t):matrix()*matrix:translate(i-3,i-7,3+i)
		--	vao3:draw({M=model,V=view,P=proj,tex=tex3.tex,viewPos=vpos,tex_spec=tex11.tex})
		--end
		w:swap()
		graphics.pollevents()
	end
	w:close()
	--closetask(id)
	--closetask(id2)
	collectgarbage()
end

keycb=function(win,key,scancode,action,mods)
	print("key "..tostring(key))
end
--w:keycallback(keycb,errfunc)



qwe=function(tt,...)
	local num=0
	--tt=1.3
	while true do
		if t>tt*num then
			print("num"..tostring(num))
			num=num+1
		end
		--coroutine.yield()
	end
end


--id2=newtask(qwe,errfunc,1.1,"q","w","e","r","t","y")
--id=newtask(qwe,errfunc,1.3)
newtask(render,errfunc,1,2,3,4,5)