/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

// #version 150 

in  vec4 vPosition;
in vec4 vColor;
in  vec3 vNormal;
in vec2 vTexCoord;

out vec4 color;
out vec4 position;
varying vec2 texCoord;
varying float stripeTexCoord;
varying vec2 latticeCoord;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 PositionAmbient, PositionDiffuse, PositionSpecular;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform vec4 LightPosition2;
uniform float Shininess;

uniform float lighting;
uniform float smoothFlag;
uniform float SpotlightFlag;

uniform float Vertical;
uniform float Slanted;
uniform float Check;

uniform float Eye;
uniform float Obj;

uniform float Upright;
uniform float Tilted;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation
uniform float PosConstAtt;
uniform float PosLinearAtt;
uniform float PosQuadAtt;

void main()
{

    vec3 pos;
	if(lighting > 0.5) {
		pos = (ModelView * vPosition).xyz;
		vec3 L;
		float attenuation;
		L = normalize(LightPosition.xyz);
		attenuation = 1.0;
		vec3 E = normalize( -pos );
		vec3 H = normalize( L + E );

		// Transform vertex normal into eye coordinates
		//vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;
		vec3 N;
		if(smoothFlag < 0.5) {
			N = normalize(Normal_Matrix * vNormal);
		}
		else N = normalize(Normal_Matrix * vPosition.xyz);

	  // Compute terms in the illumination equation
		vec4 ambient = AmbientProduct;

		float d = max( dot(L, N), 0.0 );
		vec4  diffuse = d * DiffuseProduct;

		float s = pow( max(dot(N, H), 0.0), Shininess );
		vec4  specular = s * SpecularProduct;
    
		if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
		} 
		color = attenuation * (diffuse + specular + ambient);

		/* ------- Positional Light ------- */
		L = normalize(LightPosition.xyz - pos);
		float distance2 = length(LightPosition2 - (ModelView * vPosition));
		attenuation = 1/(PosConstAtt + PosLinearAtt*distance2 + PosQuadAtt*distance2*distance2);
		H = normalize( L + E );

		// Transform vertex normal into eye coordinates
		//vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;

	  // Compute terms in the illumination equation
		ambient = PositionAmbient;

		d = max( dot(L, N), 0.0 );
		diffuse = d * PositionDiffuse;

		s = pow( max(dot(N, H), 0.0), Shininess );
		specular = s * PositionSpecular;
    
		if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
		} 

		if(SpotlightFlag > 0.5) {
			vec4 at = {-6.0, 0, -4.5, 1.0};
			vec4 spotDir = normalize(at - LightPosition2);
			float spotDot = dot(L, spotDir.xyz);
			float spotAtten = 0.0;
			if(spotDot < cos(3.1415926/90)) {
				spotAtten = pow(spotDot, 15);
			}
			attenuation *= spotAtten;
		}

		color += attenuation * (diffuse + specular + ambient);
	}
	else {
		color = vColor;
	}
    gl_Position = Projection * ModelView * vPosition;
	position = gl_Position;
	if(Vertical > 0.5 || Slanted > 0.5) {
		if(Vertical > 0.5) {
			if(Obj > 0.5) {
				if(Check > 0.5) texCoord = vec2(0.75*(vPosition.x + 1), 0.75*(vPosition.y + 1));
				stripeTexCoord = vPosition.x * 2.5;
			}
			else if (Check > 0.5) {
				texCoord = vec2(0.75*(pos.x + 1), 0.75*(pos.y + 1));
			}
			else stripeTexCoord = pos.x * 2.5;
		}
		else {
			if(Obj > 0.5) {
				if(Check > 0.5) texCoord = vec2(0.45*(vPosition.x + vPosition.y + vPosition.z), 0.45*(vPosition.x - vPosition.y + vPosition.z));
				stripeTexCoord = 1.5*(vPosition.x + vPosition.y + vPosition.z);
			}
			else if(Check > 0.5) {
				texCoord = vec2(0.45*(pos.x + pos.y + pos.z), 0.45 * (pos.x - pos.y + pos.z));	
			}	
			else stripeTexCoord = 1.5*(pos.x + pos.y + pos.z);
		}
		if(Upright > 0.5) {
			latticeCoord = vec2(0.5 * (vPosition.x + 1), 0.5 * (vPosition.y + 1));
		}
		else if(Tilted > 0.5) {
			latticeCoord = vec2(0.3 * (vPosition.x + vPosition.y + vPosition.z), 0.3 * (vPosition.x - vPosition.y + vPosition.z));
		}
	}
	else {
		texCoord = vTexCoord;
	}


}
