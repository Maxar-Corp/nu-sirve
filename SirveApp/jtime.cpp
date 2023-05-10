/*==============================================================================
* File: JTime.h
*
* Description: Provides functions for converting to/from julian time
* to the gregorian calendar
*
*
*=============================================================================*/

#include "JTime.h"

namespace jtime {

	/*==============================================================================
	* Function: jtime::JulianDate
	*
	* Purpose: Compute the julian date based on gregorian calendar and UTC time
	*
	* Inputs:  year -- year in format yyyy
	*		  month -- month
	*		    day -- day
	*		     hr -- hour
	*		 minute -- minute
	*		    sec -- seconds
	*
	* Return: Return the julian date corresponding to UTC date/time provided
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	double JulianDate(int year, int month, int day, int hr, int minute, double sec) {

		double a, b, c, d, e;

		a = 367 * year;
		b = -std::floor(7 * (year + std::floor((month + 9) / 12)) / 4);
		c = -std::floor(3 * (std::floor((year + (month - 9) / 7) / 100) + 1) / 4);
		d = std::floor(275 * month / 9) + day + 1721028.5;
		e = (hr * 3600 + minute * 60 + sec) / 86400;

		return a + b + c + d + e;
	}

	/*==============================================================================
	* Function: jtime::DateTime
	*
	* Purpose: Convert a julian date to the gregorian calendar (UTC)
	*
	* Inputs:  julianDate -- Julian date for conversion
	*
	* Return: Returns an armadillo vector type, with the following format:
			  [year, month, day, hour, minute, second]
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/

	arma::vec DateTime(double julianDate)
	{
		double jd, z, alpha, a, b, c, d, e, f;

		jd = julianDate + 0.5;
		z = std::floor(jd);
		f = jd - z;

		if (z < 2299161) {
			a = z;
		}
		else {
			alpha = std::floor((z - 1867216.25) / 36524.25);
			a = z + 1 + alpha - std::floor(alpha / 4.);
		}
		b = a + 1524;
		c = std::floor((b - 122.1) / 365.25);
		d = std::floor(365.25 * c);
		e = std::floor(((b - d) / 30.6001));

		double month, day, dayD, year, hrD, hr, minD, mn, secD;

		if (e < 13.5) {
			month = e - 1;
		}
		else {
			month = e - 13;
		}
		if (month > 2.5) {
			year = c - 4716;
		}
		else {
			year = c - 4715;
		}

		dayD = b - d - std::floor(30.6001*e) + f;
		day = std::floor(dayD);

		hrD = (dayD - std::floor(dayD)) * 24;
		hr = std::floor(hrD);

		minD = (hrD - hr) * 60.;
		mn = std::floor(minD);

		secD = (minD - mn) * 60.;

		arma::vec out;

		out << year << month << day << hr << mn << secD;

		return out;
	}

}
