/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

// #version 150 


in  vec4 color;
in vec4 position;
varying vec2 texCoord;
varying float stripeTexCoord;
varying vec2 latticeCoord;
out vec4 fColor;

uniform float fog;
uniform float linear;
uniform float exponent;
uniform float square;
uniform vec4 eye;
uniform float TextureFlag;

uniform float Vertical;
uniform float Slanted;
uniform float Check;
uniform float Lattice;

uniform sampler2D texture_2D;
uniform sampler1D texture_1D;

void main() 
{ 
	
	if(TextureFlag > 0.5) {
		if(Vertical > 0.5 || Slanted > 0.5) {
			if(Check > 0.5) {
				if(texture(texture_2D, texCoord).r == 0) fColor = vec4(0.9, 0.1, 0.1, 1.0) * color;
				else fColor = texture(texture_2D, texCoord) * color;
			}
			else fColor = texture(texture_1D, stripeTexCoord) * color;
		}
		else {
			fColor = texture(texture_2D, texCoord) * color;
		}
	}
	else fColor = color;
	float distanceFromEye = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor;
	vec4 newColor;
	if(fog > 0.5) {
		if(linear > 0.5) {
			fogFactor = (18.0 - distanceFromEye)/(18.0);
		}
		else if(exponent > 0.5) {
			fogFactor = 1/(exp(distanceFromEye * 0.09));
		}	
		else if(square > 0.5) {
			fogFactor = 1/(exp(pow(distanceFromEye * 0.09, 2)));
		}
		clamp(fogFactor, 0.0, 1.0);
		newColor = mix(vec4(0.7, 0.7, 0.7, 0.5), fColor, fogFactor);
		fColor = newColor;
	}
	if(Lattice > 0.5) {
		if(fract(4 * latticeCoord.x) < 0.35 && fract(4 * latticeCoord.y) < 0.35) {
			discard;
		}
	}
	
} 

