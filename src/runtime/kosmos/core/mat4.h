#pragma once

namespace kosmos
{
	typedef float mat4f[16];
	typedef float vec4f[4];
	typedef float vec3f[3];

	// all these are for 16 floats
	void mat4_id(float *dest);
	void mat4_zero(float *dest);
	void mat4_trans(float *dst, float x, float y, float z);
	void mat4_scale(float *dst, float x, float y, float z);
	void mat4_rot_x(float *dst, float r);
	void mat4_rot_y(float *dst, float r);
	void mat4_rot_z(float *dst, float r);
	void mat4_persp(float *dst, float w, float h, float zn, float zf);

	void mul_mat4(float *dst, const float *left, const float *right);
	void mul_mat4(float *dst, const float *m0, const float *m1, const float *m2);

	void mul_mat4_vec4(float *dst_vec, const float *src_mat, const float *src_vec);
}
