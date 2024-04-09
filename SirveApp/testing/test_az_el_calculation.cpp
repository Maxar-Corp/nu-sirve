#include "test_az_el_calculation.h"
#include <QDebug>

TestAzElCalculation::TestAzElCalculation()
{}

void TestAzElCalculation::test_azimuth_elevation_calculation()
{
    double delta = 0.000005; //Reasonable room for deviation given precision differences from the unit test to the real file

    //All values taken the star cal example dataset
    double ifovx = .00001732099918;
    double ifovy = .00001732099918;
    double sensor_lat_frame_0 = 41.0022335;
    double sensor_long_frame_0 = -72.83490371;
    double sensor_lat_frame_1 = 41.00223360;
    double sensor_long_frame_1 = -72.83490311;
    std::vector<double> dcm_frame_0 = {1.10336495, -29.22956721, -32.26855777, 11.75914352, 31.27938875, -27.93147359, 41.92075815, -8.004815666, 8.684342038};
    std::vector<double> dcm_frame_1 = {1.101399250, -29.22489568, -32.26757275, 11.75588568, 31.27845844, -27.92778274, 41.91765655, -8.004209470, 8.680239582};

	bool adjust_frame_ref = false;
	bool adjust_base = false;
	bool adjust_rounding_err = false;

    // Verify the boresight az/el from frame 0 matches reality
    int boresight_x = 0, boresight_y = 0;
    std::vector<double> results = AzElCalculation::calculate(boresight_x, boresight_y, sensor_lat_frame_0, sensor_long_frame_0, dcm_frame_0, ifovx, ifovy, adjust_frame_ref, adjust_base, adjust_rounding_err);
    double data_export_expected_az = 99.904309, data_export_expected_el = 28.537447;
	
	qDebug() << "The value of results[0] -> " << results[0];
	qDebug() << "The value of results[1] -> " << results[1];
	
    QVERIFY(results[0] - delta <= data_export_expected_az && results[0] + delta >= data_export_expected_az);
    QVERIFY(results[1] - delta <= data_export_expected_el && results[1] + delta >= data_export_expected_el);

    /*
    NOTES ON THESE TESTS:

    The "pinpoint" track x/y are the values the pinpoint feature reports as the pixel (coordinate) "identity."
	
	The indices of the human-user-interface viewport (the 640x480 gui window) are base-1.
	
	That is, the "pinpoint" track X/Y is 1-indexed from the left and top
	
    However, the math in this az/el calculation assumes a 0-indexed offset from the center
    Therefore, the "real" track identity must be adjusted twice:
        - Each dimension must be shifted left and down by 1 pixel (??)
		- X/Y must be shifted left/down by 1 pixel.
        - We must subtract half the height/width for our X/Y, so 320 and 240 respectively
    */
	
	adjust_frame_ref = true;
	adjust_base = true;
	adjust_rounding_err = true;
    
    // Verify the OSM track az/el from frame 0 matches reality
    int pinpoint_track_x = 624, pinpoint_track_y = 206;
	
    results = AzElCalculation::calculate(pinpoint_track_x, pinpoint_track_y, sensor_lat_frame_0, sensor_long_frame_0, dcm_frame_0, ifovx, ifovy,  adjust_frame_ref, adjust_base, adjust_rounding_err);
    data_export_expected_az = 100.245425, data_export_expected_el = 28.572941;
	
	qDebug() << "The value of results[0] -> " << results[0];
	qDebug() << "The value of results[1] -> " << results[1];
	
    QVERIFY(results[0] - delta <= data_export_expected_az && results[0] + delta >= data_export_expected_az);
    QVERIFY(results[1] - delta <= data_export_expected_el && results[1] + delta >= data_export_expected_el);

	adjust_frame_ref = true;
	adjust_base = true;
	adjust_rounding_err = false;

    // Verify the OSM track az/el from frame 1 matches reality
    pinpoint_track_x = 624, pinpoint_track_y = 205;

	results = AzElCalculation::calculate(pinpoint_track_x, pinpoint_track_y, sensor_lat_frame_1, sensor_long_frame_1, dcm_frame_1, ifovx, ifovy,  adjust_frame_ref, adjust_base, adjust_rounding_err);
    data_export_expected_az = 100.250388, data_export_expected_el = 28.569998;
    QVERIFY(results[0] - delta <= data_export_expected_az && results[0] + delta >= data_export_expected_az);
    QVERIFY(results[1] - delta <= data_export_expected_el && results[1] + delta >= data_export_expected_el);
}