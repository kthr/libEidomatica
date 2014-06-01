/*
 * VectorArray2D.h
 *
 *  Created on: Jul 10, 2012
 *      Author: kthierbach
 */

#ifndef VECTORARRAY2D_H_
#define VECTORARRAY2D_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "vector_2D.hpp"

#define Address(i,j,nx,ny) ((i)+(nx)*(j))
#define VectorArrayMagic 1447379762
#define VectorArrayMagic2 1278906352

class VectorArray2D
{
public:
	int nx, ny;
	double dx, dy;
	double *vx, *vy;

	VectorArray2D(void);
	VectorArray2D(int _nx, int _ny, double _dx = 1.0, double _dy = 1.0);
	VectorArray2D(VectorArray2D &orig);
	~VectorArray2D();

	typedef struct
	{
			double end;
			double error;
			double alpha;
			double vortex_weight;
			double mu;
			double lambda;
			std::string boundary;
			std::string method;
			double actual_error;
			double actual_time;
	} fparameters;

	Vector2D get(int i, int j) const;
	double getx(int i, int j) const;
	double gety(int i, int j) const;
	void set(int i, int j, Vector2D v);
	void set(int i, int j, Vector2D *v);
	void set(int i, int j, double x, double y);
	void setAll(Vector2D*v);
	void incAll(Vector2D*v);
	void setRotation(double scale, double phi, double cx, double cy);
	void copy(VectorArray2D *source);
	double div(int i, int j);
	double dVxdx(int i, int j);
	double dVydx(int i, int j);
	double dVxdy(int i, int j) const;
	double dVydy(int i, int j) const;
	Vector2D divComponents(int i, int j) const;
	double jacobian(int i, int j) const;
	Vector2D d2x(int i, int j) const;
	Vector2D d2y(int i, int j) const;
	Vector2D dxy(int i, int j) const;
	Vector2D laplace(int i, int j);
	bool load(const char *fname);
	bool save(const char *fname);
	bool save(const char *fname, fparameters *param);

private:
	const static int BUFFER_SIZE = 1024;
};


#endif /* VECTORARRAY2D_H_ */
