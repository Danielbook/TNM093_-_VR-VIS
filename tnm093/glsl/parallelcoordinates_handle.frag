uniform vec3 _color;

void main() {
	gl_FragData[0] = vec4(_color, 1.0);
}
