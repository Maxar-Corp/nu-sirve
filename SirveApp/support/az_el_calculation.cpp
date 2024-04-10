#include "az_el_calculation.h"

std::vector<double> AzElCalculation::calculate(int x, int y, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y, bool adjust_frame_ref, bool adjust_base, bool adjust_rounding_err)
{
	
	int offset_x = adjust_frame_ref ? FOCAL_PLANE_ARRAY_WIDTH  / 2 : 0;
	int offset_y = adjust_frame_ref ? FOCAL_PLANE_ARRAY_HEIGHT / 2 : 0;
	
	x -= offset_x;
    y -= offset_y;
	
	offset_x = adjust_base ? 1 : 0;
	offset_y = adjust_base ? 1 : 0;
	
	x -= offset_x;
	y -= offset_y;
	
	offset_x = adjust_rounding_err ? 1 : 0;
	
	x -= offset_x;
	
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

	double rtd = 180 / std::_Pi;

	double los_az = rtd * std::atan2(los_ned[1], los_ned[0]);
	double los_el = rtd * std::atan2(-los_ned[2], std::sqrt(los_ned[0] * los_ned[0] + los_ned[1] * los_ned[1]));

	if (los_az < 0)
		los_az += 360;

	return std::vector<double> {los_az, los_el};
}