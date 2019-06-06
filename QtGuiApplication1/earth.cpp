/*==============================================================================
* File: Earth.cpp
*
* Description: Provides functions relating to the Earth constants, coordinate
* frames, and celestial body position relative to the Earth.
*
*
*=============================================================================*/

//Required Headers
#include "Earth.h"

namespace earth {

	//Earth measurements in kilometers
	double earthSMA_km = 6378.137;
	double earthSMI_km = 6356.752314;

	//Earth shape
	double earthEccentricity = std::sqrt((std::pow(earthSMA_km, 2) - std::pow(earthSMI_km, 2)) / std::pow(earthSMA_km, 2));
	double earthFlattening = 1 / 298.2572236;

	//Earth rotation rate (rad/s)
	double earthRotation = 0.00007292115;//rad / solar sec

	//Conversions for radians/degrees
	double rad_2_deg = 180 / M_PI;
	double deg_2_rad = M_PI / 180;

	/*==============================================================================
	* Function: earth::MMod
	*
	* Purpose: Performs modulus operation element-by-element for an armadillo vector
	*
	* Inputs: vector -- armadillo type vector of numerators
	*		       n -- divisor
	*
	*
	* Return: Return the remainder when the vector is divided by the divisor
	*
	* Notes: None
	*=============================================================================*/
	arma::vec MMod(arma::vec vector, double n) {

		return vector - arma::floor(vector / n) * n;
	}

	/*==============================================================================
	* Function: earth::GMST_Calculate
	*
	* Purpose: Calculates the Greenwich Mean Sidereal Time (GMST) from the julian date
	*			This gives the angle from ECI-x axis to the ECF x-axis, which is helpful
	*			when converting between ECI and ECF.
	*
	* Inputs: julianDate -- Julian date for calculating the angle
	*
	*
	* Return: Return the rotation angle (about the z-axis) to align the ECI with the ECF
	*			coordinate frame
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	double GMST_Calculate(double julianDate, bool epochJ2000) {

		double Tut1, thetaGMST, theta;

		if (epochJ2000) Tut1 = julianDate / 36525.;
		else Tut1 = (julianDate - 2451545.0) / 36525.;

		thetaGMST = 67310.54841 + (876600 * 60. * 60. + 8640184.812866) * Tut1 + 0.093104 * std::pow(Tut1, 2) - 6.2 * std::pow(10, -6) * std::pow(Tut1, 3);

		theta = std::remainder(thetaGMST, 86400) / 240.;

		return theta;
	}

	double ERA_Calculate(double julianDate, bool epochJ2000) {

		double era, t;

		if (epochJ2000) t = julianDate ;
		else t = (julianDate - 2451545.0);

		era = 2 * M_PI * (0.7790572732640 + 1.00273781191135448 * t ) * rad_2_deg;

		era = std::remainder(era, 360);

		return era;
	}

	/*==============================================================================
	* Function: earth::GMST_Calculate
	*
	* Purpose: Calculates the Greenwich Mean Sidereal Time (GMST) from the julian date
	*			This gives the angle from ECI-x axis to the ECF x-axis, which is helpful
	*			when converting between ECI and ECF.
	*
	* Inputs: julianDate -- Julian date for calculating the angle
	*
	*
	* Return: Return an armadillo vector of rotation angles (about the z-axis) to
	*			align the ECI with the ECF coordinate frame for each julian date provided
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::vec GMST_Calculate(arma::vec julianDate, bool epochJ2000) {
		arma::vec Tut1, thetaGMST, theta;

		if (epochJ2000) Tut1 = julianDate / 36525.;
		else Tut1 = (julianDate - 2451545.0) / 36525.;

		thetaGMST = 67310.54841 + (876600 * 60. * 60. + 8640184.812866) * Tut1 + 0.093104 * arma::pow(Tut1, 2) - 6.2 * std::pow(10, -6) * arma::pow(Tut1, 3);
		theta = MMod(thetaGMST, 86400.) / 240.;

		return theta;
	}

	arma::vec ERA_Calculate(arma::vec julianDate, bool epochJ2000) {

		arma::vec era;

		if (epochJ2000) era = 280.46061837504 + 360.985612288808 * (julianDate)* rad_2_deg;
		else era = 280.46061837504 + 360.985612288808 * (julianDate - 2451545.0)* rad_2_deg;

		era = MMod(era, 360);

		return era;
	}

	/*==============================================================================
	* Function: earth::ECFtoLLA
	*
	* Purpose: Converts the given Earth-Centered Earth-Fixed (ECF) vector to latitude,
	*			longitude, and altitude. Angles are in degrees and distance is in
	*			kilometers
	*
	* Inputs: ecfData -- ECF data to convert, with x-axis in row 0, y-axis in row 1,
	*					 and z-axis in row 2. Assumes distance is in kilometers
	*
	*
	* Return: Returns the latitude (deg), longitude (deg), and alitude
	*			corresponding to the provided ECF vector
	*
	* Notes: None
	*=============================================================================*/
	arma::mat ECFtoLLA(arma::mat ecfData) {

		double a, b, e2, eps;
		int numColumns;
		arma::rowvec x, y, z;

		numColumns = ecfData.n_cols;
		x = ecfData.row(0);
		y = ecfData.row(1);
		z = ecfData.row(2);

		a = earthSMA_km;
		b = earthSMA_km * (1 - earthFlattening);
		e2 = std::pow(earthEccentricity, 2);
		eps = e2 / (1 - e2);

		arma::rowvec p, q;
		p = arma::sqrt(arma::square(ecfData.row(0)) + arma::square(ecfData.row(1)));
		q = arma::atan(ecfData.row(2) * a / (p * b));

		arma::rowvec lat, lon, nu, h;

		lat = arma::atan((z + eps * b * arma::pow(arma::sin(q), 3)) / (p - e2 * a * arma::pow(arma::cos(q), 3)));
		lon = arma::atan2(y, x);
		nu = a / arma::sqrt(1 - e2 * arma::pow(arma::sin(lat), 2));
		h = (p / arma::cos(lat)) - nu;

		arma::mat out(3, numColumns);
		out.row(0) = lat * 180 / M_PI;
		out.row(1) = lon * 180 / M_PI;
		out.row(2) = h;

		return out;
	}

	/*==============================================================================
	* Function: earth::LLAtoECF
	*
	* Purpose: Converts the given Earth-Centered Earth-Fixed (ECF) vector to latitude,
	*			longitude, and altitude. Angles are in degrees and distance is in
	*			kilometers
	*
	* Inputs: lla -- Latitude, longitude, and altitude data to convert, with one
	*				  position per column. Units are deg, deg, and kilometers
	*
	*
	* Return: Returns the ECF vector components in km. One vector per column
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat LLAtoECF(arma::mat lla) {

		int numColumns;
		numColumns = lla.n_cols;

		double earthRadius, e2;
		earthRadius = earthSMA_km;
		e2 = std::pow(earthEccentricity, 2);

		arma::mat C, S;
		C = earthRadius / (arma::sqrt(1 - e2 * arma::pow(arma::sin(lla.row(0) * deg_2_rad), 2)));
		S = (earthRadius * (1 - e2)) / (arma::sqrt(1 - e2 * arma::pow(arma::sin(lla.row(0) * deg_2_rad), 2)));

		arma::mat out(3, numColumns);
		out.row(0) = (C + lla.row(2)) * arma::cos(lla.row(0) * deg_2_rad) * arma::cos(lla.row(1) * deg_2_rad);
		out.row(1) = (C + lla.row(2)) * arma::cos(lla.row(0) * deg_2_rad) * arma::sin(lla.row(1) * deg_2_rad);
		out.row(2) = (S + lla.row(2)) * arma::sin(lla.row(0) * deg_2_rad);

		return out;
	}

	/*==============================================================================
	* Function: earth::Atf_Transformation
	*
	* Purpose: Calculates the given transformation matrix to go from ECF to a
	*			topocentric coordinate system given a latitude and longitude
	*
	* Inputs:  latitude -- latitude of the new origin point (deg)
	*		  longitdue -- longitude of the new origin point (deg)
	*
	*
	* Return: Returns a 3x3 matrix to convert an ECF vector to topocenric coordinate
	*		   frame at the given lat/lon. Multiplacation assumes a format of
	*		   [Rot Matrix] * [vector]
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat Atf_Transformation(double latitude, double longitude) {

		arma::mat rot2, rot3, rotation;

		rot2 = rotate::CoordFrame_Rotation2(90 - latitude);
		rot3 = rotate::CoordFrame_Rotation3(longitude);
		rotation = rot2 * rot3;

		return rotation;
	}

	/*==============================================================================
	* Function: earth::Atf_Transformation
	*
	* Purpose: Calculates the given transformation matrices to go from ECF to a
	*			topocentric coordinate system given a series of latitudes and longitudes
	*
	* Inputs: latitude -- latitude of the new origin point (deg)
	*		 longitdue -- longitude of the new origin point (deg)
	*
	*
	* Return: Returns a 9 x n matrix to convert an ECF vector to topocenric coordinate
			   frame at the given lat/lon. Multiplacation assumes a format of
			   [Rot Matrix] * [vector]
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat Atf_Transformation(arma::vec latitude, arma::vec longitude) {

		arma::vec lat_r, lon_r;

		arma::mat rot2, rot3, rotation;

		rot2 = rotate::CoordFrame_Rotation2(90 - latitude);
		rot3 = rotate::CoordFrame_Rotation3(longitude);
		rotation = rotate::MatrixMulitply(rot2, rot3, false);

		return rotation;
	}

	/*==============================================================================
	* Function: earth::AER
	*
	* Purpose: Calculates the azimuth, elevation, and range of a target from a given
				origin point
	*
	* Inputs:  lla -- Matrix of latitude, longitude, and range for the origin points.
	*			       Each column represents one location with the units of degrees,
	*				   degrees, and kilometers
	*  ecfPosition -- Matrix of ECF vector positions with each column
	*				  representing one target (kilometers)
	*
	*
	* Return: Returns a 3 x n matrix of azimuth, elevation, and altitude data with
			   each column representing one target. Units are deg, deg, km
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat AER(arma::mat lla, arma::mat ecfPosition) {

		int nColumns1 = lla.n_cols;
		int nColumns2 = ecfPosition.n_cols;
		int maxCol = std::max(nColumns1, nColumns2);

		arma::mat gsECF, diff, Atf, sez, xyz;
		gsECF.set_size(ecfPosition.n_rows, ecfPosition.n_cols); gsECF.zeros();

		xyz = LLAtoECF(lla);
		gsECF.row(0) += xyz(0, 0); gsECF.row(1) += xyz(1, 0); gsECF.row(2) += xyz(2, 0);

		diff = ecfPosition - gsECF;
		Atf = Atf_Transformation(lla.row(0), lla.row(1));
		sez = rotate::MatrixMulitply(Atf, diff, false);

		arma::mat baseVector(3, maxCol, arma::fill::zeros), aerData(3, maxCol);
		baseVector.row(0) = sez.row(0);
		baseVector.row(1) = sez.row(1);
		//baseVector.row(2) = 0;

		aerData.row(0) = arma::atan2(sez.row(1), -sez.row(0)) * rad_2_deg;
		aerData.row(2) = rotate::MMagnitude(sez).t();
		aerData.row(1) = arma::asin(sez.row(2) / aerData.row(2)) * rad_2_deg;


		return aerData;
	}

	/*==============================================================================
	* Function: earth::Afi_Transformation
	*
	* Purpose: Calculates the given transformation matrix to go from the Earth
	*			Centered Inertial (ECI) coordinate frame to the to Earth Centered
	*			Earth Fixed (ECF) coordinate frame
	*
	* Inputs: julianDate -- julian date associated with the time that each
	*						 transformation matrix is valid
	*
	*
	* Return: Returns a 3 x 3 matrix to convert an ECI vector to an ECF vector.
	*		  Multiplacation assumes a format of [Rot Matrix] * [vector]
	*
	* Notes: None
	*=============================================================================*/
	arma::mat Afi_Transformation(double julianDate, bool epochJ2000) {

		double GMST = GMST_Calculate(julianDate, epochJ2000);

		return rotate::CoordFrame_Rotation3(GMST);
	}

	/*==============================================================================
	* Function: earth::Afi_Transformation
	*
	* Purpose: Calculates the given transformation matrices to go from the Earth
	*			Centered Inertial (ECI) coordinate frame to the to Earth Centered
	*			Earth Fixed (ECF) coordinate frame
	*
	* Inputs: julianDate -- vector of julian dates associated with the time that each
	*						 transformation matrix is valid
	*
	*
	* Return: Returns a 9 x n matrix to convert an ECI vector to an ECF vector.
	*		  Multiplacation assumes a format of [Rot Matrix] * [vector]
	*
	* Notes: None
	*=============================================================================*/
	arma::mat Afi_Transformation(arma::vec julianDate, bool epochJ2000) {

		arma::vec GMST = GMST_Calculate(julianDate, epochJ2000);

		return rotate::CoordFrame_Rotation3(GMST);
	}

	/*==============================================================================
	* Function: earth::Afi_Velocity
	*
	* Purpose: Function converts an ECI velocity vector to an ECF velocity vector
	*
	* Inputs: julianDate -- julian date associated with each position
	*		    position -- the ECI position vector (kilometers)
	*		    velocity -- the ECI velocity vector (kilometers/second)
	*
	*
	* Return: Returns an ECF velocity vector
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat Afi_Velocity(double julianDate, arma::mat position, arma::mat velocity, bool epochJ2000) {

		arma::mat Afi, omega(3,1);

		Afi = Afi_Transformation(julianDate, epochJ2000);
		omega << 0 << arma::endr << 0 << arma::endr << earthRotation;

		arma::mat comp1, comp2, result;
		comp1 = Afi * velocity;
		comp2 = arma::cross(omega, Afi * position);
		result = comp1 - comp2;

		return result;
	}

	/*==============================================================================
	* Function: earth::Afi_Velocity
	*
	* Purpose: Function converts an ECI velocity vector to an ECF velocity vector
	*
	* Inputs: julianDate -- julian date associated with each position of armadillo vector type
	*		    position -- the ECI position vector (kilometers) of armadillo matrix type
	*		    velocity -- the ECI velocity vector (kilometers/second) of armadillo matrix type
	*
	*
	* Return: Returns an ECF velocity vector of armadillo matrix type
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat Afi_Velocity(arma::vec julianDate, arma::mat position, arma::mat velocity, bool epochJ2000){

		arma::mat Afi, omega;

		Afi = Afi_Transformation(julianDate, epochJ2000);
		omega << 0 << arma::endr << 0 << arma::endr << earthRotation;

		arma::mat comp1, comp2, result;
		comp1 = rotate::MatrixMulitply(Afi, velocity, false);
		comp2 = rotate::MCross(omega, rotate::MatrixMulitply(Afi, position, false));
		result = comp1 - comp2;

		return result;
}

	/*==============================================================================
	* Function: earth::Aif_Velocity
	*
	* Purpose: Function converts an ECF velocity vector to an ECI velocity vector
	*
	* Inputs: julianDate -- julian date associated with each position
	*		    position -- the ECF position vector (kilometers)
	*		    velocity -- the ECF velocity vector (kilometers/second)
	*
	*
	* Return: Returns an ECI velocity vector
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat Aif_Velocity(double julianDate, arma::mat position, arma::mat velocity, bool epochJ2000) {

		//double gmst;
		arma::mat Afi, Aif, omega, comp1, comp2, out;

		Afi = earth::Afi_Transformation(julianDate, epochJ2000);
		Aif = Afi.t();

		omega << 0 << arma::endr << 0 << arma::endr << earthRotation;

		comp1 = arma::cross(omega, position);
		out = Aif * (comp1 + velocity);

		return out;
	}

	/*==============================================================================
	* Function: earth::Aif_Velocity
	*
	* Purpose: Function converts an ECF velocity vector to an ECI velocity vector
	*
	* Inputs: julianDate -- julian date associated with each position of armadillo vector type
	*		    position -- the ECF position vector (kilometers) of armadillo matrix type
	*		    velocity -- the ECF velocity vector (kilometers/second) of armadillo matrix type
	*
	*
	* Return: Returns an ECI velocity vector of armadillo matrix type
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat Aif_Velocity(arma::vec julianDate, arma::mat position, arma::mat velocity, bool epochJ2000) {

		arma::vec gmst;
		arma::mat Aif, omega, comp1, comp2, out;

		gmst = GMST_Calculate(julianDate, epochJ2000);
		Aif = rotate::CoordFrame_Rotation3(-gmst);

		omega << 0 << arma::endr << 0 << arma::endr << earthRotation;

		comp1 = rotate::MCross(omega, position);
		out = rotate::MatrixMulitply(Aif, comp1 + velocity, false);

		return out;

	}

	/*==============================================================================
	* Function: earth::Apparent_Acceleration
	*
	* Purpose: Function provides the acceleration related to orbiting the earth
	*
	* Inputs: position -- the ECF position vector (kilometers) of armadillo matrix type
	*
	*
	* Return: Returns an acceleration vector of armadillo matrix type
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat Apparent_Acceleration(arma::mat position) {

		arma::mat omega, out;
		omega << 0 << arma::endr << 0 << arma::endr << earthRotation;

		out = rotate::MCross(omega, position);
		out = rotate::MCross(omega, out);

		return out;
	}

	/*==============================================================================
	* Function: earth::Coriolis_Acceleration
	*
	* Purpose: Function provides the acceleration related to orbiting the earth
	*
	* Inputs: velocity -- the ECF velocity vector (kilometers/second) of armadillo matrix type
	*
	*
	* Return: Returns an acceleration vector of armadillo matrix type
	*
	* Notes: None
	*=============================================================================*/
	arma::mat Coriolis_Acceleration(arma::mat velocity){

		arma::mat omega, out;
		omega << 0 << arma::endr << 0 << arma::endr << earthRotation;

		out = rotate::MCross(2 * omega, velocity);

		return out;
	}

	/*==============================================================================
	* Function: earth::Sun_RA_Declination
	*
	* Purpose: Function provides the right ascension and declination for the sun
	*
	* Inputs: julianDate -- julian date associated with each position
	*
	*
	* Return: Returns an 2 x n armadillo matrix where each column contains an
	*			right ascension (deg) and declination (deg)
	*
	* Notes: Reference Fundamentals of Astrodynamics and Applications - Vallado
	*=============================================================================*/
	arma::mat Sun_RA_Declination(double julianDate) {

		double Tut1, lambdaM, M, lambdaEcliptic, eps, ra, decl;


		Tut1 = (julianDate - 2451545.) / 36525.;
		lambdaM = 280.460 + 36000.771 * Tut1, 360;
		M = 357.5277233 + 35999.05034 * Tut1;

		lambdaEcliptic = deg_2_rad * (lambdaM + 1.914666471 * std::sin(M * deg_2_rad) + 0.019994643 * std::sin(2 * M * deg_2_rad));
		eps = deg_2_rad * (23.439291 - 0.0130042 * Tut1);

		decl = std::asin(std::sin(eps) * std::sin(lambdaEcliptic));
		ra =  std::atan2(std::cos(eps) * std::sin(lambdaEcliptic) / std::cos(decl), std::cos(lambdaEcliptic) / std::cos(decl));

		//From NASA Models of Earth's Atmosphere (SP-8021)
		double j = 2441683 + (std::round(julianDate) - jtime::JulianDate(1972, 12, 31, 12, 0, 0)) - 2435839;
		double ls = 0.017203 * (j) + 0.0335 * std::sin(0.017203 * (j)) - 1.41;
		double ds = std::asin(std::sin(ls) * std::sin(23.45 * deg_2_rad));
		double ras = std::asin(std::tan(ds) / std::tan(23.45 * deg_2_rad));

		arma::vec out(2);
		out(0) = ra * rad_2_deg;
		out(1) = decl * rad_2_deg;

		return out;
	}

	/*==============================================================================
	* Function: earth::Get_Earth_Rotation_Rate
	*
	* Purpose: Get Earth rotation rate
	*
	* Inputs: None
	*
	*
	* Return: Returns Earth rotation rate (radians/second)
	*
	* Notes: None
	*=============================================================================*/
	double Get_Earth_Rotation_Rate() {
		return earthRotation;
	}

	/*==============================================================================
	* Function: earth::Get_Earth_Radius
	*
	* Purpose: Get Earth radius
	*
	* Inputs: None
	*
	*
	* Return: Returns Earth radius (km)
	*
	* Notes: None
	*=============================================================================*/
	double Get_Earth_Radius() {

		return earthSMA_km;
	}



}
