// ........................phong.frag ...........................
// Phong lighting in eye coordinates.

// These are set by the .vert code, and they're interpolated.
varying vec3 ec_vnormal, ec_vposition;

vec4 diffuse_color[3];
vec4 specular_color[3];
vec3 H[3];
vec3 L[3];

void main() {
	vec3 P, N, V;
	vec4 totdiffuse = (0.0,0.0,0.0,0.0);
	vec4 totspec = (0.0,0.0,0.0,0.0);

	P = ec_vposition;
	N = normalize(ec_vnormal);
	V = normalize(-P);		// usually put "eye_position - P", but eye position is (0,0,0)!

	vec4 ambient_color = gl_FrontMaterial.ambient;
	vec4 emissive_color = gl_FrontMaterial.emissive;
	float shininess = gl_FrontMaterial.shininess;

	int i;
	for(i = 0; i < 3; i++) {
		diffuse_color[i] = gl_FrontMaterial.diffuse;
		specular_color[i] = gl_FrontMaterial.specular;
		L[i] = normalize(gl_LightSource[i].position - P);
		H[i] = normalize(L[i] + V);

		diffuse_color[i] *= max(dot(N,L[i]),0.0);
		specular_color[i] *= pow(max(dot(H[i],N),0.0),shininess); 

		totdiffuse += diffuse_color[i];
		totspec += specular_color[i]; 
	}

	gl_FragColor = emissive_color + ambient_color + totdiffuse + totspec;
}