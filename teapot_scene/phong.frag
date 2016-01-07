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

	for(int i = 0; i < TOTAL_LIGHTS; i++){

		L = normalize(gl_LightSource[i].position.xyz - P);
		//This calculates the view position if the eye is at (0,0,0).  Need to figure out
		V = normalize(EyePosition - P);
		H = normalize(L + V);

		vec4 diffuse_color = (gl_FrontLightProduct[i].diffuse*diffuseWeight + vec4(tColor*textureWeight, 0.0)) * max(dot(N,L), 0.0);

		//standard Blinn-Phong specular model
		//vec4 specular_color = gl_FrontLightProduct[i].specular *pow(max(dot(H,N),0.0), shininess);
		
		//Energy conservation specular model
		vec4 specular_color = gl_FrontLightProduct[i].specular * ((shininess + 2)/(8*PI)*pow(max(dot(H,N),0.0), shininess));
		diffuse_color = clamp(diffuse_color, 0.0, 1.0);
		specular_color = clamp(specular_color, 0.0, 1.0);

		diffuse_sum += diffuse_color;
		spec_sum += specular_color;
	}
	diffuse_sum.w = 0.0;
	spec_sum.w= 0.6;
	gl_FragColor = diffuse_sum + spec_sum;
	//gl_FragColor = vec4(0, 1, 0 , 0);
}
