graphics=require("graphics")
matrix=require("matrix")
quat=require("quat")
vector=require("vector")
loadobj=require("obj")
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
	color=vec4(result,1.0f);
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

	if ld~=nil then
		local cols={}
		local removes={}
		local eq=function(a,b)
			return a[1]==b[1] and a[2]==b[2] and a[3]==b[3]
		end
		--local centers={}
		collectgarbage("collect")
		print(1,collectgarbage("count"))
		for i=0,#pos/3-1 do
			table.insert(cols,{{},{},{}})
			--local x,y,z
			--x=(pos[(i*3+0)+1]+pos[(i*3+1)+1]+pos[(i*3+2)+1])/3
			--y=(pos[(i*3+0)+2]+pos[(i*3+1)+2]+pos[(i*3+2)+2])/3
			--z=(pos[(i*3+0)+3]+pos[(i*3+1)+3]+pos[(i*3+2)+3])/3
			--table.insert(centers,{x,y,z})
		end
		collectgarbage("collect")
		print(2,collectgarbage("count"))
		local mem=0
		for i=1,#pos/3 do
			for j=i+1,#pos/3 do
				--print(#pos,i*3+2-2,j*3+2-2)
				if eq(pos[i*3-2],pos[j*3-2]) then
					table.insert(cols[i][1],{j,0})
					table.insert(cols[j][1],{i,0})
				elseif eq(pos[i*3-2],pos[j*3+1-2]) then
					table.insert(cols[i][1],{j,1})
					table.insert(cols[j][2],{i,0})
				elseif eq(pos[i*3-2],pos[j*3+2-2]) then
					table.insert(cols[i][1],{j,2})
					table.insert(cols[j][3],{i,0})
				end
				if eq(pos[i*3+1-2],pos[j*3-2]) then
					table.insert(cols[i][2],{j,0})
					table.insert(cols[j][1],{i,1})
				elseif eq(pos[i*3+1-2],pos[j*3+1-2]) then
					table.insert(cols[i][2],{j,1})
					table.insert(cols[j][2],{i,1})
				elseif eq(pos[i*3+1-2],pos[j*3+2-2]) then
					table.insert(cols[i][2],{j,2})
					table.insert(cols[j][3],{i,1})
				end
				if eq(pos[i*3+2-2],pos[j*3-2]) then
					table.insert(cols[i][3],{j,0})
					table.insert(cols[j][1],{i,2})
				elseif eq(pos[i*3+2-2],pos[j*3+1-2]) then
					table.insert(cols[i][3],{j,1})
					table.insert(cols[j][2],{i,2})
				elseif eq(pos[i*3+2-2],pos[j*3+2-2]) then
					table.insert(cols[i][3],{j,2})
					table.insert(cols[j][3],{i,2})
				end
			end
			collectgarbage("collect")
			local cc=collectgarbage("count")
			--if mem~=cc then
			--	print(33,tostring(i).."/"..tostring(#pos/3-1),cc)
			--	mem=cc
			--end
		end
		--[[for i=0,#pos/3-1 do
			for j=i+1,#pos/3-1 do
				if eql(i*3,i*3+1,j*3,j*3+1) then
					cols[i+1][1]={j,1}
					break
				elseif eql(i*3,i*3+2,j*3,j*3+2) then
					cols[i+1][1]={j,2}
					break
				elseif eql(i*3,i*3+3,j*3,j*3+3) then
					cols[i+1][1]={j,2}
					break
				end
			end
			for j=i+1,#pos/3-1 do
				if eql(i*3+1,i*3+1,j*3+1,j*3+1) then
					cols[i+1][1]={j,1}
					break
				elseif eql(i*3+1,i*3+2,j*3+1,j*3+2) then
					cols[i+1][1]={j,2}
					break
				elseif eql(i*3+1,i*3+3,j*3+1,j*3+3) then
					cols[i+1][1]={j,2}
					break
				end
			end
			for j=i+1,#pos/3-1 do
				if eql(i*3+2,i*3+1,j*3+2,j*3+1) then
					cols[i+1][1]={j,1}
					break
				elseif eql(i*3+2,i*3+2,j*3+2,j*3+2) then
					cols[i+1][1]={j,2}
					break
				elseif eql(i*3+2,i*3+3,j*3+2,j*3+3) then
					cols[i+1][1]={j,2}
					break
				end
			end
		end]]
		collectgarbage("collect")
		print(3,collectgarbage("count"),"ld",ld)
		for i=1,#pos/3 do
			local a={table.unpack(pos[i*3-2])}
			local b={table.unpack(pos[i*3-1])}
			local c={table.unpack(pos[i*3])}
			--local a,b,c=pos[i*3+1],pos[i*3+2],pos[i*3+3]
			a[1]=math.floor(a[1]/ld)*ld
			a[2]=math.floor(a[2]/ld)*ld
			a[3]=math.floor(a[3]/ld)*ld
			b[1]=math.floor(b[1]/ld)*ld
			b[2]=math.floor(b[2]/ld)*ld
			b[3]=math.floor(b[3]/ld)*ld
			c[1]=math.floor(c[1]/ld)*ld
			c[2]=math.floor(c[2]/ld)*ld
			c[3]=math.floor(c[3]/ld)*ld
			if eq(a,b) or eq(b,c) or eq(c,a) then
				table.insert(removes,i)
			end
		end
		print("removes",#removes)
		collectgarbage("collect")
		print(4,collectgarbage("count"))
		print("poss",#pos)
		for i=1,#removes do
			local v=removes[i]
			local x,y,z,ux,uy,nx,ny,nz
			if not (eq(pos[v*3-2],pos[v*3+1-2]) or eq(pos[v*3+1-2],pos[v*3+2-2]) or eq(pos[v*3+2-2],pos[v*3-2])) then
				--print(pos[(0*3+0)*3+1],pos[(0*3+1)*3+1],pos[(0*3+2)*3+1])
				x=(pos[v*3-2][1]+pos[v*3+1-2][1]+pos[v*3+2-2][1])/3
				y=(pos[v*3-2][2]+pos[v*3+1-2][2]+pos[v*3+2-2][2])/3
				z=(pos[v*3-2][3]+pos[v*3+1-2][3]+pos[v*3+2-2][3])/3
				if bvt then
					ux=(texcoord[v*3-2][1]+pos[v*3+1-2][1])/2
					uy=(texcoord[v*3-2][2]+pos[v*3+1-2][2])/2
				end
				nx=(normal[v*3-2][1]+normal[v*3+1-2][1]+normal[v*3+2-2][1])/3
				ny=(normal[v*3-2][2]+normal[v*3+1-2][2]+normal[v*3+2-2][2])/3
				nz=(normal[v*3-2][3]+normal[v*3+1-2][3]+normal[v*3+2-2][3])/3
				for _,j in pairs(cols[v][1]) do
					--print(#pos,j[1]*3+j[2]-2)
					pos[j[1]*3+j[2]-2][1]=x
					pos[j[1]*3+j[2]-2][2]=y
					pos[j[1]*3+j[2]-2][3]=z
					if bvt then
						texcoord[j[1]*3+j[2]-2][1]=ux
						texcoord[j[1]*3+j[2]-2][2]=uy
					end
					normal[j[1]*3+j[2]-2][1]=nx
					normal[j[1]*3+j[2]-2][2]=ny
					normal[j[1]*3+j[2]-2][3]=nz
					for _,jj in pairs(cols[v][2]) do
						table.insert(cols[j[1]][j[2]+1],jj)
					end
					for _,jj in pairs(cols[v][3]) do
						table.insert(cols[j[1]][j[2]+1],jj)
					end
				end
				for _,j in pairs(cols[v][2]) do
					pos[j[1]*3+j[2]-2][1]=x
					pos[j[1]*3+j[2]-2][2]=y
					pos[j[1]*3+j[2]-2][3]=z
					if bvt then
						texcoord[j[1]*3+j[2]-2][1]=ux
						texcoord[j[1]*3+j[2]-2][2]=uy
					end
					normal[j[1]*3+j[2]-2][1]=nx
					normal[j[1]*3+j[2]-2][2]=ny
					normal[j[1]*3+j[2]-2][3]=nz
					for _,jj in pairs(cols[v][1]) do
						table.insert(cols[j[1]][j[2]+1],jj)
					end
					for _,jj in pairs(cols[v][3]) do
						table.insert(cols[j[1]][j[2]+1],jj)
					end
				end
				for _,j in pairs(cols[v][3]) do
					pos[j[1]*3+j[2]-2][1]=x
					pos[j[1]*3+j[2]-2][2]=y
					pos[j[1]*3+j[2]-2][3]=z
					if bvt then
						texcoord[j[1]*3+j[2]-2][1]=ux
						texcoord[j[1]*3+j[2]-2][2]=uy
					end
					normal[j[1]*3+j[2]-2][1]=nx
					normal[j[1]*3+j[2]-2][2]=ny
					normal[j[1]*3+j[2]-2][3]=nz
					for _,jj in pairs(cols[v][1]) do
						table.insert(cols[j[1]][j[2]+1],jj)
					end
					for _,jj in pairs(cols[v][2]) do
						table.insert(cols[j[1]][j[2]+1],jj)
					end
				end
			end
		end
		for i=#removes,1,-1 do
			local v=removes[i]
			table.remove(pos,v*3+1)
			table.remove(pos,v*3+1)
			table.remove(pos,v*3+1)
			if bvt then
				table.remove(texcoord,v*3+1)
				table.remove(texcoord,v*3+1)
				table.remove(texcoord,v*3+1)
			end
			table.remove(normal,v*3+1)
			table.remove(normal,v*3+1)
			table.remove(normal,v*3+1)
		end
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

w=graphics.window.new(640,480,"title",errfunc)
shader=w:shader(w:vertexshader(vertex_shader_text),w:fragmentshader(fragment_shader_text))
shader2=w:shader(w:vertexshader(vertex_shader_text2),w:fragmentshader(fragment_shader_text2))

--vao4=loadobjdata(w,shader2,"robot.obj",false)
--vao3=loadobjdata(w,shader2,"monster.obj")
--vao22=loadobjdata(w,shader2,"skel.obj")--,nil,1.0)
--vao2=loadobjdata(w,shader2,"skel.obj",false)
vao1=loadobjdata(w,shader2,"terrain.obj",false)

tex1=w:texture(graphics.image.load("Gun.png"))
tex11=w:texture(graphics.image.load("box_spec.png"))
--tex2=w:texture(graphics.image.load("btr8chas.jpg"))
--tex3=w:texture(graphics.image.load("monster.bmp"))

dt=graphics:timedelta()
width,height,t=0,0,1
vpos=vector:new(0,0,0)
vquat=quat:identity()
mouseview=false
speed=3
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
		w:clearcolor(0.2,0.3,0.3,1.0)
		w:cleardepth()
		proj=matrix:projection(0.1,1000,math.pi/2,width/height)
		model=matrix:scale(0.1,0.1,0.1)*matrix:translate(0,0,0)
		vao1:draw({M=model,V=view,P=proj,tex=tex1.tex,viewPos=vpos,tex_spec=tex11.tex,sun=sun})
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