in vec4 vColor;
in vec4 velocity;
in vec4 position;

out vec4 fColor;

void main() {
	if(position.y < 0.1) {
		discard;
	}
	fColor = vColor;
	
}