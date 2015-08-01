#include <math.h>

namespace kosmos
{
	#define MTX_SET_COL(c, dst, x, y, z, w) \
		dst[4*c] = x; \
		dst[4*c+1] = y; \
		dst[4*c+2] = z; \
		dst[4*c+3] = w; \

	#define MTX_SET_ROW(r, dst, x, y, z, w) \
		dst[r] = x; \
		dst[r+4] = y; \
		dst[r+8] = z; \
		dst[r+12] = w; \

	// Matrices are column major storage. Which means that the element
	// [0,1,2,3] in a matrix forms the first column of the matrix.
	//
	// [ 1 0 0 x ]
	// [ 0 1 0 y ]
	// [ 0 0 1 z ]
	// [ 0 0 0 1 ]
	//
	// is thus stored as
	//
	// [ 1 0 0 0 0 1 0 0 0 0 1 0 x y z 1 ]

	void mat4_persp(float *dst, float w, float h, float zn, float zf)
	{
		for (int i=0;i<16;i++)
		{
			dst[i] = 0.0f;
		}
		dst[0] = 1.0f / w;
		dst[5] = 1.0f / h;
		dst[10] = (zf + zn) / (zn - zf);
		dst[11] = -1;
		dst[14] = 2.0f * (zf * zn) / (zn - zf);
	}

	void mat4_zero(float *dst)
	{
		MTX_SET_COL(0, dst, 0, 0, 0, 0);
		MTX_SET_COL(1, dst, 0, 0, 0, 0);
		MTX_SET_COL(2, dst, 0, 0, 0, 0);
		MTX_SET_COL(3, dst, 0, 0, 0, 0);
	}

	void mat4_id(float *dst)
	{
		MTX_SET_COL(0, dst, 1, 0, 0, 0);
		MTX_SET_COL(1, dst, 0, 1, 0, 0);
		MTX_SET_COL(2, dst, 0, 0, 1, 0);
		MTX_SET_COL(3, dst, 0, 0, 0, 1);
	}

	void mat4_scale(float *dst, float x, float y, float z)
	{
		MTX_SET_COL(0, dst, x, 0, 0, 0);
		MTX_SET_COL(1, dst, 0, y, 0, 0);
		MTX_SET_COL(2, dst, 0, 0, z, 0);
		MTX_SET_COL(3, dst, 0, 0, 0, 1);
	}
	
	void mat4_trans(float *dst, float x, float y, float z)
	{
		MTX_SET_COL(0, dst, 1, 0, 0, 0);
		MTX_SET_COL(1, dst, 0, 1, 0, 0);
		MTX_SET_COL(2, dst, 0, 0, 1, 0);
		MTX_SET_COL(3, dst, x, y, z, 1);
	}

	void mat4_rot_z_sc(float *dst, float sin, float cos)
	{
		MTX_SET_COL(0, dst, cos,  sin, 0, 0);
		MTX_SET_COL(1, dst, -sin, cos, 0, 0);
		MTX_SET_COL(2, dst, 0, 0, 1, 0);
		MTX_SET_COL(3, dst, 0, 0, 0, 1);
	}

	void mat4_rot_z(float *dst, float a)
	{
		mat4_rot_z_sc(dst, sinf(a), cosf(a));
	}

	void mat4_rot_x_sc(float *dst, float sin, float cos)
	{
		MTX_SET_COL(0, dst, 1, 0, 0, 0);
		MTX_SET_COL(1, dst, 0, cos, sin, 0);
		MTX_SET_COL(2, dst, 0, -sin, cos, 0);
		MTX_SET_COL(3, dst, 0, 0, 0, 1);
	}

	void mat4_rot_x(float *dst, float a)
	{
		mat4_rot_x_sc(dst, sinf(a), cosf(a));
	}

	void mul_mat4_vec4(float *dst_vec, const float *src_mtx, const float *src_vec)
	{
		const float x = src_vec[0];
		const float y = src_vec[1];
		const float z = src_vec[2];
		const float w = src_vec[3];
		dst_vec[0] = src_mtx[0]*x + src_mtx[4]*y + src_mtx[8]*z + src_mtx[12]*w;
		dst_vec[1] = src_mtx[1]*x + src_mtx[5]*y + src_mtx[9]*z + src_mtx[13]*w;
		dst_vec[2] = src_mtx[2]*x + src_mtx[6]*y + src_mtx[10]*z + src_mtx[14]*w;
		dst_vec[3] = src_mtx[3]*x + src_mtx[7]*y + src_mtx[11]*z + src_mtx[15]*w;
	}

	void mul_mat4(float *dst, const float *left, const float *right)
	{
		for (int i=0;i<4;i++)
		{
			mul_mat4_vec4(dst + 4*i, left, right + 4*i);
		}
	}

	void mul_mat4(float *dst, const float *m0, const float *m1, const float *m2)
	{
		float t[16];
		mul_mat4(t, m1, m2);
		mul_mat4(dst, m0, t);
	}

}