#version 150




in   vec4 vPosition;
in   vec4 vNormal;
in vec2 s_vTexCoord;

// output values that will be interpretated per-fragment
out vec2 texCoord;
out  vec3 fN;
out  vec3 fE;
out  vec3 fL;

//translate to spacial
uniform mat4 ModelWorld;
uniform mat4 ModelView;
uniform vec4 LightPosition;
//translate to screen
uniform mat4 Projection;

void main()
{
	texCoord=s_vTexCoord;
	vec4 tempPos=ModelView*ModelWorld*vPosition;
	vec4 nonNorm;
	if(vNormal.x==0){
		nonNorm=vec4(1,0,0,0);
	}else{
		nonNorm=vNormal+vec4(0,1,0,0);
	}
	//we are garenteed that nonNorm is not normal to the plane
	vec3 tv1=cross(vNormal.xyz,nonNorm.xyz);
	vec4 v1=vec4(tv1.x,tv1.y,tv1.z,0);
	vec3 tv2=cross(vNormal.xyz,v1.xyz);
	vec4 v2=vec4(tv2.x,tv2.y,tv2.z,0);
	//note v1 X v2 = vNormal*c
	v1=ModelView*ModelWorld*(vPosition+v1)-tempPos;
	v2=ModelView*ModelWorld*(vPosition+v2)-tempPos;
	//note that v1 and v2 remain in the plane thrughout the transition
	vec3 TtempNorm=normalize(cross(v1.xyz,v2.xyz));
	vec4 tempNorm=vec4(TtempNorm.x,TtempNorm.y,TtempNorm.z,0);

    fN = tempNorm.xyz;
    fE = -tempPos.xyz;
	vec4 TLightPosition;


	TLightPosition=ModelView*LightPosition;
	fL = TLightPosition.xyz;
	if( LightPosition.w != 0.0 ) {
		fL = TLightPosition.xyz - tempPos.xyz;
	}
	

    gl_Position = Projection*tempPos;
}
