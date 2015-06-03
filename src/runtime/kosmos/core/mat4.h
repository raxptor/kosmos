#pragma once

namespace kosmos
{
	// all these are for 16 floats
	void mat4_identity(float *dest);
	void mat4_translate(float *dst, float x, float y, float z);
	void mat4_scale(float *dst, float x, float y, float z);
	void mat4_rot_x(float *dst, float r);
	void mat4_rot_y(float *dst, float r);
	void mat4_rot_z(float *dst, float r);
	void mat4_mul(float *dst, const float *left, const float *right);
	void mat4_persp(float *dst, float w, float h, float zn, float zf);
	void mat4_vec4_mul(float *dst_vec, const float *src_mat, const float *src_vec);
}
