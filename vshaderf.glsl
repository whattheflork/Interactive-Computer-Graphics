in vec4 color;
in vec4 velocity;

uniform float time;

out vec4 vColor;
out vec4 position;

void main() {
	float xPos = 0.0 + 0.001 * velocity.x * time;
	float yPos = 0.1 + 0.001 * velocity.y * time + 0.5 * (-4.9 * pow(10.0, -7.0)) * pow(time, 2);
	float zPos = 0.0 + 0.001 * velocity.z * time;

	position = vec4(xPos, yPos, zPos, 1);

	gl_Position = position;
	vColor = color;
}