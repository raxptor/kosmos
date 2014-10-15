#include "alloc2d.h"

namespace kosmos
{
	namespace alloc2d
	{
		struct row_t
		{
			int height;
			int used;
		};

		enum
		{
			MAX_ROWS = 256
		};

		struct data
		{
			int width, height;
			int row_count;
			int height_used;
			row_t row[MAX_ROWS];
		};

		data* create(int width, int height)
		{
			data *d = new data();
			d->width = width;
			d->height = height;
			d->row_count = 0;
			d->height_used = 0;
			return d;
		}

		void free(data*d)
		{
			delete d;
		}

		// output rect in u[0], v[0], u[1], v[1] on succes (return true)
		bool alloc(data *d, int width, int height, int *u, int *v)
		{
			int v0 = 0;

			// scan for exact match
			for (int i=0;i!=d->row_count;i++)
			{
				row_t *r = &d->row[i];
				if (r->height == height && (r->used + width) < d->width)
				{
					u[0] = r->used;
					u[1] = r->used + width;
					v[0] = v0;
					v[1] = v0 + height;
					r->used += width;
					return true;
				}
				v0 += r->height;
			}

			// can add another row?
			if (d->row_count == MAX_ROWS || v0 + height > d->height)
				return false;
			if (width > d->width)
				return false;

			row_t *r = &d->row[d->row_count++];
			r->height = height;
			r->used = width;

			u[0] = 0;
			u[1] = width;
			v[0] = v0;
			v[1] = v0 + height;
			return true;
		}

		void clear(data *)
		{

		}

	}
}