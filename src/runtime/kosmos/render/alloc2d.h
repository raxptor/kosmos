#ifndef __KOSMOS_ALLOC_2D_H__
#define __KOSMOS_ALLOC_2D_H__

namespace kosmos
{
	// 2d allocator with row/column stacking. it only returns exact matches, wastes no pixels and leaves
	// padding and optimizations for the user.
	namespace alloc2d
	{
		struct data;
		
		data* create(int width, int height);
		void free(data*);

		// output rect in u[0], v[0], u[1], v[1] on succes (return true)
		bool alloc(data *, int width, int height, int *u, int *v);
		void clear(data *);
	}
}

#endif
