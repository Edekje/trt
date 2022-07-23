#ifndef TRT_COORDINATE_H
#define TRT_COORDINATE_H

namespace trt {
	class Coordinate {
		public:
			double t_lab;
	};

	class Coordinate1D :  public Coordinate {
		public: 
			Coordinate1D(double T, double R);
			double r;
	};
}

#endif
