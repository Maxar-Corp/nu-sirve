#include "test_az_el_calculation.h"

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

    // Verify the boresight az/el from frame 0 matches reality
    int boresight_x = 0, boresight_y = 0;
    std::vector<double> results = AzElCalculation::calculate(boresight_x, boresight_y, sensor_lat_frame_0, sensor_long_frame_0, dcm_frame_0, ifovx, ifovy, false);
    double data_export_expected_az = 99.904309, data_export_expected_el = 28.537447;
    QVERIFY(results[0] - delta <= data_export_expected_az && results[0] + delta >= data_export_expected_az);
    QVERIFY(results[1] - delta <= data_export_expected_el && results[1] + delta >= data_export_expected_el);

    /*
    NOTES ON THESE TESTS:

    The "pinpoint" track x/y are the values the pinpoint feature reports as the pixel (coordinate) "identity."
    
    The indeces of the human-user-interface viewport (the 640x480 gui window) start at 1 for the pinpoint feature.
    That is, the "pinpoint" track X/Y is 1-indexed from the left and top.
    
    However, the math in this az/el calculation assumes a 0-indexed offset from the center
    
    Therefore, the "real" track identity must be adjusted by one pixel
    Each coordinate must be shifted by 1 to the "left" (fixing the X direction) and "up" (fixing the Y direction)
    */
    
    // Verify the OSM track az/el from frame 0 matches reality
    int pinpoint_track_x = 624, pinpoint_track_y = 206;
    //NOTE: Due to a weird bug where the OSM centroid double is rounded to an integer, the OSM track for this frame is slightly off the pinpoint pixel.
    //Adjusting this one to the left makes the math work for this frame.
    int adjusted_track_x = pinpoint_track_x - 2;
    int adjusted_track_y = pinpoint_track_y - 1;
    int osm_x = 302;
    int osm_y = -35;

    std::vector<double> pinpoint_results = AzElCalculation::calculate(adjusted_track_x, adjusted_track_y, sensor_lat_frame_0, sensor_long_frame_0, dcm_frame_0, ifovx, ifovy, true);
    std::vector<double> osm_results = AzElCalculation::calculate(osm_x, osm_y, sensor_lat_frame_0, sensor_long_frame_0, dcm_frame_0, ifovx, ifovy, false);

    data_export_expected_az = 100.245425, data_export_expected_el = 28.572941;
    QVERIFY(pinpoint_results[0] - delta <= data_export_expected_az && pinpoint_results[0] + delta >= data_export_expected_az);
    QVERIFY(pinpoint_results[1] - delta <= data_export_expected_el && pinpoint_results[1] + delta >= data_export_expected_el);
    QVERIFY(osm_results[0] - delta <= data_export_expected_az && osm_results[0] + delta >= data_export_expected_az);
    QVERIFY(osm_results[1] - delta <= data_export_expected_el && osm_results[1] + delta >= data_export_expected_el);

    // Verify the OSM track az/el from frame 1 matches reality
    pinpoint_track_x = 624, pinpoint_track_y = 205;
    adjusted_track_x = pinpoint_track_x - 1;
    //NOTE: Due to a weird bug where the OSM centroid double is rounded to an integer, the OSM track for this frame is slightly off the pinpoint pixel.
    //Adjusting this one "down" (canceling out the shift "up") makes the math work for this frame.
    adjusted_track_y = pinpoint_track_y;
    osm_x = 303;
    osm_y = -35;

    pinpoint_results = AzElCalculation::calculate(adjusted_track_x, adjusted_track_y, sensor_lat_frame_1, sensor_long_frame_1, dcm_frame_1, ifovx, ifovy, true);
    osm_results = AzElCalculation::calculate(osm_x, osm_y, sensor_lat_frame_1, sensor_long_frame_1, dcm_frame_1, ifovx, ifovy, false);

    data_export_expected_az = 100.250388, data_export_expected_el = 28.569998;
    QVERIFY(pinpoint_results[0] - delta <= data_export_expected_az && pinpoint_results[0] + delta >= data_export_expected_az);
    QVERIFY(pinpoint_results[1] - delta <= data_export_expected_el && pinpoint_results[1] + delta >= data_export_expected_el);
    QVERIFY(osm_results[0] - delta <= data_export_expected_az && osm_results[0] + delta >= data_export_expected_az);
    QVERIFY(osm_results[1] - delta <= data_export_expected_el && osm_results[1] + delta >= data_export_expected_el);
}