
/*==============================================================================
* File: Euler.h
*
* Description: Provides functions for:
	1) Developing Euler rotation matrices for coordinate frames
	2) Support functions for the vectorization of coordinate frames transforation
		and vectors. This allows for multiplying multiple coordinate transformation
		matrices together at the same time insted of one by one. This works the
		same for multiplying coordinate transformations by vectors as well
	3) Support functions to do cross product, dot product, magnitude, unit vectors,
		and angle between calculations for vectors organized in a 3 x n format
*
*
*=============================================================================*/

#ifndef EULER_H
#define EULER_H
//#define _USE_MATH_DEFINES

#include <armadillo>
#include <iostream>
#include <fstream>
#include <cmath>
#include <math.h>
#include <random>
#include <stdexcept>

namespace rotate {

	arma::mat CoordFrame_Rotation1(double angle);
	arma::mat CoordFrame_Rotation1(arma::vec angle);
	arma::mat CoordFrame_Rotation2(double angle);
	arma::mat CoordFrame_Rotation2(arma::vec angle);
	arma::mat CoordFrame_Rotation3(double angle);
	arma::mat CoordFrame_Rotation3(arma::vec angle);
	arma::mat MatrixMulitply(arma::mat array1, arma::mat array2, bool transpose= false);
	arma::mat MCross(arma::mat array1, arma::mat array2);
	//arma::vec MCross(arma::vec array1, arma::vec array2);
	arma::mat MDot(arma::mat array1, arma::mat array2);
	//double MDot(arma::vec array1, arma::vec array2);
	arma::mat MMagnitude(arma::mat array1);
	//double MMagnitude(arma::vec array1);
	arma::mat MUnit(arma::mat array1);
	//arma::vec MUnit(arma::vec array1);
	arma::mat MAngleBetween(arma::mat array1, arma::mat array2);
	//double MAngleBetween(arma::vec array1, arma::vec array2);

}

#endif
