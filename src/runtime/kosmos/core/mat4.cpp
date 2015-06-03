namespace kosmos
{
	void mat4_persp(float *dst, float w, float h, float zn, float zf)
	{
	    for (int i=0;i<16;i++)
	        dst[i] = 0.0f;
	    dst[0] = 1.0f / w;
	    dst[5] = 1.0f / h;
	    dst[10] = 2.0f / (zf - zn);
	    dst[11] = (zf + zn) / (zf - zn);
	    dst[15] = 1.0f;
	}

	void mat4_id(float *dst, float w, float h, float zn, float zf)
	{
	    for (int i=0;i<16;i++)
	        dst[i] = 0.0f;
	    dst[0] = 1.0f;
	    dst[5] = 1.0f;
	    dst[10] = 1.0f;
	    dst[15] = 1.0f;
	}
}