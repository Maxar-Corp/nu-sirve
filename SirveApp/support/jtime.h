
/*==============================================================================
* File: JTime.h
*
* Description: Provides functions for converting to/from julian time
* to the gregorian calendar
*
*
*=============================================================================*/
#ifndef JTIME_H
#define JTIME_H


//Required Headers
#include <armadillo>
#include <iostream>
#include <fstream>
#include <cmath>
#include <math.h>
#include <random>
#include <stdexcept>

namespace jtime {

	double JulianDate(int year, int month, int day, int hr, int minute, double sec);
	arma::vec DateTime(double julianDate);
}

#endif
