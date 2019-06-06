/*==============================================================================
* File: Earth.h
*
* Description: Provides functions relating to the Earth constants, coordinate
* frames, and celestial body position relative to the Earth.
*
*
*=============================================================================*/

#ifndef EARTH_H
#define EARTH_H
#define _USE_MATH_DEFINES

//Required Headers
#include <armadillo>
#include <iostream>
#include <fstream>
#include <cmath>
#include <math.h>
#include <random>
#include <stdexcept>

#include "Euler.h"
#include "JTime.h"

namespace earth {

	arma::vec MMod(arma::vec vector, double n);
	double Get_Earth_Rotation_Rate();
	double Get_Earth_Radius();

	double GMST_Calculate(double julianDate, bool epochJ2000 = false);
	double ERA_Calculate(double julianDate, bool epochJ2000 = false);
	arma::vec GMST_Calculate(arma::vec julianDate, bool epochJ2000 = false);
	arma::vec ERA_Calculate(arma::vec julianDate, bool epochJ2000 = false);

	arma::mat ECFtoLLA(arma::mat ecfData);
	arma::mat LLAtoECF(arma::mat lla);
	arma::mat Atf_Transformation(double latitude, double longitude);
	arma::mat Atf_Transformation(arma::vec latitude, arma::vec longitude);
	arma::mat AER(arma::mat lla, arma::mat ecfPosition);
	arma::mat Afi_Transformation(double julianDate, bool epochJ2000 = false);
	arma::mat Afi_Transformation(arma::vec julianDate, bool epochJ2000 = false);
	arma::mat Afi_Velocity(double julianDate, arma::mat position, arma::mat velocity, bool epochJ2000 = false);
	arma::mat Afi_Velocity(arma::vec julianDate, arma::mat position, arma::mat velocity, bool epochJ2000 = false);
	arma::mat Aif_Velocity(double julianDate, arma::mat position, arma::mat velocity, bool epochJ2000 = false);
	arma::mat Aif_Velocity(arma::vec julianDate, arma::mat position, arma::mat velocity, bool epochJ2000 = false);
	arma::mat Apparent_Acceleration(arma::mat position);
	arma::mat Coriolis_Acceleration(arma::mat velocity);
	arma::mat Sun_RA_Declination(double julianDate);

}
#endif
