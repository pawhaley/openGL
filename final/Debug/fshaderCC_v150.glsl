#version 150




in  vec3 fN;
in  vec3 fL1;
in  vec3 fL2;
in  vec3 fL3;
in  vec3 fE;
in vec2 texCoord;
out vec4 fColor; 

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform float Shininess;
uniform sampler2D texture;


void main() 
{ 
    // Normalize the input lighting vectors
    vec3 N = normalize(fN);
    vec3 E = normalize(fE);
    vec3 L = normalize(fL1);
    vec3 H = normalize(  E+L );
    
    fColor = AmbientProduct;



    float Kd = max(dot(L, N), 0.0);
    vec4 diffuse = Kd*DiffuseProduct;
    
    float Ks = pow(max(dot(N, H), 0.0), Shininess);
    vec4 specular = Ks*SpecularProduct;

    // discard the specular highlight if the light's behind the vertex
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    }

	fColor += (diffuse + specular)/2;

	L = normalize(fL2);
    H = normalize(  E+L );
	Kd = max(dot(L, N), 0.0);
    diffuse = Kd*DiffuseProduct;
    
    Ks = pow(max(dot(N, H), 0.0), Shininess);
    specular = Ks*SpecularProduct;

    // discard the specular highlight if the light's behind the vertex
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    }

	fColor += (diffuse + specular)/2;

	L = normalize(fL3);
    H = normalize(  E+L );
	Kd = max(dot(L, N), 0.0);
    diffuse = Kd*DiffuseProduct;
    
    Ks = pow(max(dot(N, H), 0.0), Shininess);
    specular = Ks*SpecularProduct;

    // discard the specular highlight if the light's behind the vertex
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    }






    fColor += (diffuse + specular)/2;
    fColor.a = 1.0;
	fColor = fColor*texture2D(texture,texCoord);
}