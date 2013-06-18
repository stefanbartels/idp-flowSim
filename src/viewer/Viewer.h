#ifndef VIEWER_H
#define VIEWER_H

class Viewer
{
	public:
		Viewer();

		virtual void renderFrame (
				double** U,
				double** V,
				double** P,
				int nx,
				int ny,
				int it
			) = 0;
};

#endif // VIEWER_H
