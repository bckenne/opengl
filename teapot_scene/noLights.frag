varying vec3 ec_vnormal, ec_vposition;
uniform sampler2D texture;

const int TOTAL_LIGHTS = 3;
const double PI = 3.1415926535;
const float diffuseWeight = 0.0;
const float textureWeight = 1.0;

void main(void){

	vec3 P, N, L, V, H, tColor;
	vec3 EyePosition = vec3(4,4,4);
	N = normalize(ec_vnormal);
	P = ec_vposition;

	vec4 diffuse_sum = vec4(0,0,0,0);
	vec4 spec_sum = vec4(0,0,0,0); 

	float shininess = gl_FrontMaterial.shininess;
	tColor = vec3(texture2D(texture, gl_TexCoord[0].st));
	glFragColor = vec4(tColor,1.0);
	
}
