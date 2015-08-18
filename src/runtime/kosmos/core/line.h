#pragma once

#include <math.h>

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

	inline float line_point_distance(float x, float y, float x0, float y0, float x1, float y1)
	{
		const float a = x - x0;
		const float b = y - y0;
		const float c = x1 - x0;
		const float d = y1 - y0;

		const float dot = a * c + b * d;
		const float len_sq = c * c + d * d;

		if (len_sq != 0)
		{
			const float param = dot / len_sq;
			float xc, yc;

			if (param < 0)
			{
				xc = x0;
				yc = y0;
			}
			else if (param > 1)
			{
				xc = x1;
				yc = y1;
			}
			else
			{
				xc = x0 + param * c;
				yc = y0 + param * d;
			}

			return sqrtf((xc-x)*(xc-x) + (yc-y)*(yc-y));
		}
		return 0;
	}
}
