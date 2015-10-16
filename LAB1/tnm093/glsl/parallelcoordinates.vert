#version 400
layout(location = 0) in vec2 in_position;

// These values need to be synchronized with tnm_parallelcoordinate.h
#define AxisHandlePositionTop 0
#define AxisHandlePositionBottom 1

uniform int _location;

void main() {
    vec2 pos = in_position;

    if (_location == AxisHandlePositionBottom) {
        const mat2 rotation = mat2(
            0.0, -1.0,
            1.0, 0.0
        );

        pos = rotation * pos;
    }

    gl_Position = vec4(pos, 0.0, 1.0);
}
