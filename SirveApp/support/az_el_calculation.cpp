#include "az_el_calculation.h"

#include "euler.h"
#include "support/earth.h"

#include <qmath.h>


std::vector<double> AzElCalculation::calculate(int nRows, int nCols, int x, int y, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y, bool adjust_frame_ref)
{
	if (adjust_frame_ref)
	{
		x -= nCols / 2;
		y -= nRows / 2;
	}
	
    arma::mat a_ecf_to_seu = earth::Atf_Transformation(sensor_lat, sensor_long);
	arma::mat rot_z = rotate::CoordFrame_Rotation3(180);
	arma::mat rot_x = rotate::CoordFrame_Rotation1(180);

	arma::mat a_ecf_to_ned = rot_x * rot_z * a_ecf_to_seu;

	arma::mat cam_to_ecf(dcm);
	cam_to_ecf.reshape(3, 3);

	double dtheta = x * ifov_x;
	double dphi = -y * ifov_y; //positive is up on the FPA in this case, so need a negative in front

	arma::vec los(3);
	los(0) = std::sin(dtheta) * std::cos(dphi);
	los(1) = -std::sin(dphi);
	los(2) = std::cos(dtheta) * std::cos(dphi);

	arma::vec los_ned = a_ecf_to_ned * cam_to_ecf * los;

    double rtd = 180.0 / M_PI;

    double los_az = std::ceil(rtd * std::atan2(los_ned[1], los_ned[0])*1000000.0)/1000000.0;
    double los_el = std::ceil(rtd * std::atan2(-los_ned[2], std::sqrt(los_ned[0] * los_ned[0] + los_ned[1] * los_ned[1]))*1000000.0)/1000000.0;

	if (los_az < 0)
		los_az += 360;

	return std::vector<double> {los_az, los_el};
}

std::vector<int> AzElCalculation::calculateXY(int nRows, int nCols, double az, double el, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y)
{

    arma::mat a_ecf_to_seu = earth::Atf_Transformation(sensor_lat, sensor_long);
	arma::mat rot_z = rotate::CoordFrame_Rotation3(180);
	arma::mat rot_x = rotate::CoordFrame_Rotation1(180);

	arma::mat a_ecf_to_ned = rot_x * rot_z * a_ecf_to_seu;

	arma::mat cam_to_ecf(dcm);
	cam_to_ecf.reshape(3, 3);

	double rtd = 180.0 / M_PI;
	double dtr = M_PI / 180.0;
    double uz = - std::sin(el*dtr);
    double ux = -uz*std::cos(az*dtr)/std::tan(el*dtr);
    double uy = -uz*std::sin(az*dtr)/std::tan(el*dtr);
    arma::vec los_ned(3);
	arma::vec los(3);
	los_ned(0) = ux;
	los_ned(1) = uy;
	los_ned(2) = uz;
    std::vector<double> test_cam_to_ecf = arma::conv_to<std::vector<double>>::from(cam_to_ecf.as_col());
    std::vector<double> test_a_ecf_to_ned = arma::conv_to<std::vector<double>>::from(a_ecf_to_ned.as_col());
    std::vector<double> test_inv_cam_to_ecf = arma::conv_to<std::vector<double>>::from(arma::inv(cam_to_ecf).as_col());
    std::vector<double> test_inv_a_ecf_to_ned = arma::conv_to<std::vector<double>>::from(arma::inv(a_ecf_to_ned).as_col());
    los  = arma::inv(cam_to_ecf)*arma::inv(a_ecf_to_ned)*los_ned;
    std::vector<double> test_los = arma::conv_to<std::vector<double>>::from(los);
    double dphi = -std::asin(los[1]);
    double dth = std::asin(los[0]/std::cos(std::sin(los[1])));
    int x = std::round(dth/ifov_x);
    int y = std::round(-dphi/ifov_y);

	return std::vector<int> {x, y};
}
