#pragma once

namespace kosmos
{
	struct intersection
	{
		float t;
		float x, y;
	};

	inline bool line_intersection(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, intersection *res)
	{
		const float bx = x1 - x0;
		const float by = y1 - y0;
		const float dx = x3 - x2;
		const float dy = y3 - y2;
		const float bd = bx * dy - by * dx;
		if (bd > 0)
		{
			return false;
		}

		const float cx = x2 - x0;
		const float cy = y2 - y0;
		const float t = (cx * dy - cy * dx) / bd;
		if (t < 0 || t > 1)
		{
			return false;
		}

		const float u = (cx * by - cy * bx) / bd;
		if (u < 0 || u > 1)
		{
			return false;
		}

		res->t = t;
		res->x = x0 + t * bx;
		res->y = y0 + t * by;
		return true;
	}
}
