#include "osm_reader.h"

OSMReader::OSMReader()
{
    num_messages = 0;
    contains_data = false;

}

OSMReader::~OSMReader()
{
}

int OSMReader::LoadFile(char *file_path, bool input_combine_tracks)
{

    contains_data = false;

    combine_tracks = input_combine_tracks;

    errno_t err = fopen_s(&fp, file_path, "rb");

    if (err != 0) return err;

    FindMessageNumber();
    InitializeVariables();
    LoadData();

    fclose(fp);

    contains_data = true;

    return err;
}

void OSMReader::FindMessageNumber()
{

    int number_iterations = 0;
    long int seek_position, current_p, current_p1, current_p2;
    size_t status_code;

    while (true && number_iterations < kMAX_NUMBER_ITERATIONS)
    {
        const int num_header_values = 3;
        uint64_t header[num_header_values];
        status_code = ReadMultipleValues(header);

        if (status_code == num_header_values && header[2]) {
            num_messages++;

			current_p = ftell(fp);
            seek_position = 92 - 24;
            status_code = fseek(fp, seek_position, SEEK_CUR);

			current_p1 = ftell(fp);
            uint32_t data[2];
            status_code = ReadMultipleValues(data, true);
			
			current_p2 = ftell(fp);
            double value = data[0] + data[1] * 1e-6;
            frame_time.push_back(value);

            seek_position = header[2] - 76;
            status_code = fseek(fp, seek_position, SEEK_CUR);
        }
        else
        {
            break;
        }

        if (number_iterations == 6240)
            std::cout << number_iterations;

        number_iterations++;
    }


}

void OSMReader::InitializeVariables()
{

    int size_allocation = 1;

    //TODO if combine_tracks is false, do math to find new pre-allocation value
    /*if (combine_tracks) {

    }
    else {
        size_allocation = num_messages;
    }
    */

    data.reserve(num_messages);
}

void OSMReader::LoadData()
{
    int return_code;
    return_code = fseek(fp, 0, SEEK_SET);

    int num_iterations = -1;
    for (int i = 0; i < num_messages; i++)
    {

        bool valid_step = i == 0 || frame_time[i] - frame_time[i - 1] != 0 || !combine_tracks;
        if (valid_step) {

            Frame current_frame;

            current_frame.msg_header = ReadMessageHeader();
            current_frame.frame_header = ReadFrameHeader();
            current_frame.data = ReadFrameData();

            data.push_back(current_frame);
        }
        else
        {
            AddTrackToLastFrame();
        }

    }
}

void OSMReader::AddTrackToLastFrame()
{
    //TODO Verify that this adding track to last frame works correctly.....
    std::cout << "Adding track data to last frame...double check that this works" << std::endl;

    int return_code;
    Frame last_frame = data.back();

    uint32_t start_track_index = last_frame.data.num_tracks;
    return_code = fseek(fp, 284, SEEK_SET);
    last_frame.data.num_tracks += ReadValue<uint32_t>(true);

    for (uint32_t j = start_track_index; j < last_frame.data.num_tracks; j++)
    {
        TrackData current_track = GetTrackData(last_frame.data);
        last_frame.data.track_data.push_back(current_track);
    }
}

std::vector<double> OSMReader::get_lat_lon_alt(std::vector<double> ecf)
{
	
	arma::vec ecf_vector(ecf);
	ecf_vector = ecf_vector / 1000;

	arma::vec lla = earth::ECFtoLLA(ecf_vector.subvec(0, 2));

	std::vector<double> out = arma::conv_to<std::vector<double>>::from(lla);
	out[2] *= 1000;
	return out;
}

MessageHeader OSMReader::ReadMessageHeader()
{
    uint64_t seconds = ReadValue<uint64_t>();
    uint64_t nano_seconds = ReadValue<uint64_t>();
    uint64_t tsize = ReadValue<uint64_t>();

    //TODO matlab also has a condition testing whether tsize is empty. Implement conditional test
    if (tsize == 0) {
        throw std::invalid_argument("received invalid size value in message header");
        //return;
    }

    MessageHeader current_message;
    current_message.seconds = seconds + nano_seconds * 1e-9;
    current_message.size = tsize;

    return current_message;
}

FrameHeader OSMReader::ReadFrameHeader() {

    FrameHeader fh;
    fh.authorization = ReadValue<uint64_t>(true);
    fh.classification = ReadValue<uint32_t>(true);
    fh.type = ReadValue<uint32_t>(true);
    fh.priority = ReadValue<uint32_t>(true);
    fh.oper_indicator = ReadValue<uint32_t>(true);
    fh.info_source = ReadValue<uint32_t>(true);

    fh.info_destination = ReadValue<uint32_t>(true);
    // TODO Matlab has commented out input for sensor ID. Check that this is correct
    uint32_t frame_seconds = ReadValue<uint32_t>(true);
    uint32_t frame_micro_seconds = ReadValue<uint32_t>(true);
    fh.time_generated_seconds = frame_seconds + frame_micro_seconds * 1e-6;

    fh.transaction_id = ReadValue<uint32_t>(true);
    fh.ack_req_indicator = ReadValue<uint32_t>(true);
    fh.ack_response = ReadValue<uint32_t>(true);
    fh.cant_pro_reason = ReadValue<uint32_t>(true);
    fh.message_length = ReadValue<uint32_t>(true);
    fh.software_version = ReadValue<uint32_t>(true);

    return fh;
}

FrameData OSMReader::ReadFrameData() {

    FrameData data;

    data.task_id = ReadValue<uint32_t>(true);
    uint32_t osm_seconds = ReadValue<uint32_t>(true);
    uint32_t osm_micro_seconds = ReadValue<uint32_t>(true);
    data.frametime = osm_seconds + osm_micro_seconds * 1e-6; // GPS Time since Jan 6, 1990

    data.mrp = ReadMultipleDoubleValues(3, true);
    data.mrp_cov_rand = ReadMultipleDoubleValues(6, true);
    data.mrp_cov_bias = ReadMultipleDoubleValues(6, true);

	data.ecf = ReadMultipleDoubleValues(6, true);
	data.lla = get_lat_lon_alt(data.ecf);

	data.dcm = mr2dcos(data.mrp);
	
	data.julian_date = get_gps_time(data.frametime);
    //TODO Write code to process ecf for when ecf proided by file
    //If ~osm_unit.ecf then convert lla to ecf data and push onto frame data

    data.i_fov_x = ReadValue<double>(true);
    data.i_fov_y = ReadValue<double>(true);
    data.num_tracks = ReadValue<uint32_t>(true);

	std::vector<double> az_el_boresight = calculation_azimuth_elevation(0, 0, data);
	data.az_el_boresight = az_el_boresight;

    uint32_t start_track_index = 0;

    for (uint32_t j = start_track_index; j < data.num_tracks; j++)
    {
        TrackData current_track = GetTrackData(data);
        data.track_data.push_back(current_track);
    }

    return data;
}

TrackData OSMReader::GetTrackData(FrameData & input)
{
    TrackData current_track;
    current_track.track_id = ReadValue<uint32_t>(true);
    current_track.sensor_type = ReadValue<uint32_t>(true);

    uint32_t num_bands = ReadValue<uint32_t>(true);
    current_track.num_bands = num_bands;

    for (uint32_t k = 0; k < num_bands; k++)
    {
        IR_Data ir_data;

        ir_data.band_id = ReadValue<uint32_t>(true);
        ir_data.num_measurements = ReadValue<uint32_t>(true);

        std::vector<double> irrad, irr_sigma, irr_time;
        for (uint32_t m = 0; m < ir_data.num_measurements; m++)
        {

            ir_data.ir_radiance.push_back(ReadValue<double>(true));
            ir_data.ir_sigma.push_back(ReadValue<double>(true));

            uint32_t osm_seconds = ReadValue<uint32_t>(true);
            uint32_t osm_micro_seconds = ReadValue<uint32_t>(true);
            double measurement_time = osm_seconds + osm_micro_seconds * 1e-6; // GPS Time since Jan 6, 1990
            ir_data.time.push_back(measurement_time);
        }

        current_track.ir_measurements.push_back(ir_data);
    }

    current_track.roiBLX = ReadValue<uint32_t>(true);
    current_track.roiBLY = ReadValue<uint32_t>(true);
    current_track.roiURX = ReadValue<uint32_t>(true);
    current_track.roiURY = ReadValue<uint32_t>(true);

    current_track.semi_major_axis = ReadValue<double>(true);
    current_track.semi_minor_axis = ReadValue<double>(true);

    current_track.orientation_angle = ReadValue<double>(true);

    current_track.maxX = ReadValue<double>(true);
    current_track.maxY = ReadValue<double>(true);

    current_track.frame_stabilize_x = ReadValue<double>(true);
    current_track.frame_stabilize_y = ReadValue<double>(true);

    current_track.stable_frame_shift_x = ReadValue<double>(true);
    current_track.stable_frame_shift_y = ReadValue<double>(true);

    current_track.centroid_x = ReadValue<double>(true);
    current_track.centroid_y = ReadValue<double>(true);
    current_track.centroid_variance_x = ReadValue<double>(true);
    current_track.centroid_variance_y = ReadValue<double>(true);

	std::vector<double> az_el_track = calculation_azimuth_elevation(current_track.centroid_x, current_track.centroid_y, input);
	current_track.az_el_track = az_el_track;

    current_track.covariance = ReadValue<double>(true);

    current_track.num_pixels = ReadValue<uint32_t>(true);
    current_track.object_type = ReadValue<uint32_t>(true);

    return current_track;
}

std::vector<double> OSMReader::mr2dcos(std::vector<double> input)
{
	
	arma::vec mr(input);

	double sig = 1. - arma::accu(arma::square(mr));
	double norm = 1. / std::pow(1 - sig, 2);
	double sig2 = std::pow(sig, 2);

	double mr02, mr12, mr22;
	mr02 = mr(0) * mr(0);
	mr12 = mr(1) * mr(1);
	mr22 = mr(2) * mr(2);
	
	
	arma::mat dcos(3, 3);
	dcos(0, 0) = 4 * (mr02 - mr12 - mr22) + sig2;
	dcos(0, 1) = 8 * mr(0)*mr(1) + 4 * mr(2)*sig;
	dcos(0, 2) = 8 * mr(0)*mr(2) - 4 * mr(1)*sig;

	dcos(1, 0) = 8 * mr(0)*mr(1) - 4 * mr(2)*sig;
	dcos(1, 1) = 4 * (-mr02 + mr12 - mr22) + sig2;
	dcos(1, 2) = 8 * mr(1)*mr(2) + 4 * mr(0)*sig;

	dcos(2, 0) = 8 * mr(0)*mr(2) + 4 * mr(1)*sig;
	dcos(2, 1) = 8 * mr(1)*mr(2) - 4 * mr(0)*sig;
	dcos(2, 2) = 4 * (-mr02 - mr12 + mr22) + sig2;

	dcos = norm * dcos;

	//sig = 1 - mr'*mr;
	//norm = 1 / (1 + mr'*mr)^2;
	//dcos = norm * [4 * (mr(1) ^ 2 - mr(2) ^ 2 - mr(3) ^ 2) + sig ^ 2   8 * mr(1)*mr(2) + 4 * mr(3)*sig     8 * mr(1)*mr(3) - 4 * mr(2)*sig
	//	8 * mr(1)*mr(2) - 4 * mr(3)*sig         4 * (-mr(1) ^ 2 + mr(2) ^ 2 - mr(3) ^ 2) + sig ^ 2   8 * mr(2)*mr(3) + 4 * mr(1)*sig
	//	8 * mr(1)*mr(3) + 4 * mr(2)*sig         8 * mr(2)*mr(3) - 4 * mr(1)*sig            4 * (-mr(1) ^ 2 - mr(2) ^ 2 + mr(3) ^ 2) + sig ^ 2];
	
	arma::vec temp = arma::conv_to<arma::vec>::from(arma::vectorise(dcos));
	std::vector<double> out = arma::conv_to<std::vector<double>>::from(temp);

	return out;
}

std::vector<double> OSMReader::calculation_azimuth_elevation(int x_pixel, int y_pixel, FrameData & input)
{
	
	arma::mat a_ecf_to_seu = earth::Atf_Transformation(input.lla[0], input.lla[1]);
	arma::mat rot_z = rotate::CoordFrame_Rotation3(180);
	arma::mat rot_x = rotate::CoordFrame_Rotation1(180);

	arma::mat a_ecf_to_ned = rot_x * rot_z * a_ecf_to_seu;

	arma::mat cam_to_ecf(input.dcm);
	cam_to_ecf.reshape(3, 3);

	double dtheta = x_pixel * input.i_fov_x;
	double dphi = -y_pixel * input.i_fov_y; //positive is up on the FPA in this case, so need a negative in front

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

	//ecf2ned = rotations.ecf2ned;
	//ifovX = rotations.ifovX;
	//ifovY = rotations.ifovY;
	//dth = x_pix * ifovX;
	//dphi = -y_pix * ifovY; % positive is up on the FPA in this case, so need a negative in front

	//LOSIC = [sin(dth)*cos(dphi); ...
	//		- sin(dphi); ...
	//		cos(dth)*cos(dphi)];

	//LOSned = ecf2ned * cam2ecf*LOSIC;
	
	//%% Extract LOS Az and El from LOS vector
	//rtd = 180 / pi;
	//LOSned(abs(LOSned) < 1e-14) = 0;
	//ux = LOSned(1, :);
	//uy = LOSned(2, :);
	//uz = LOSned(3, :);
	//losaz = rtd * atan2(uy, ux);
	//losel = rtd * atan2(-uz, sqrt(ux.*ux + uy.*uy));
	//if (losaz < 0) losaz = losaz + 360; end
	
	
	//return std::vector<double>();
}

double OSMReader::get_gps_time(double offset_gps_seconds)
{
	
	//Load leap seconds file organized as year, month, day
	arma::mat leap_seconds_matrix;
	leap_seconds_matrix.load("config/year_month_leap_seconds.txt", arma::raw_ascii);

	//Store leap seconds as julian date
	int num_entries = leap_seconds_matrix.n_rows;
	arma::vec julian_date_leap_seconds(num_entries);

	//Convert dates that leap seconds occurred to julian dates
	for(int i = 0;  i < num_entries; i++)
	{
		double julian_date = jtime::JulianDate(leap_seconds_matrix(i, 0), leap_seconds_matrix(i, 1), leap_seconds_matrix(i, 2), 0, 0, 0);
		julian_date_leap_seconds(i) = julian_date;
	}

	// The last date that UTC and GPS time were aligned
	double base = jtime::JulianDate(1980, 1, 5, 0, 0, 0);
	arma::vec modified_julian_date_leap_seconds = julian_date_leap_seconds - base;

	// Find the number of leap seconds for a given date
	arma::uvec index_number_leap_seconds = arma::find(offset_gps_seconds / 86400. >= modified_julian_date_leap_seconds);
	unsigned int number_leap_seconds = index_number_leap_seconds.n_rows;

	// Adjust gps time
	double day_offset = (offset_gps_seconds - number_leap_seconds) / 86400.;

	// Establish utc julian date
	double utc_julian_date = base + day_offset;

	//arma::vec check = jtime::DateTime(utc_julian_date);
	
	return utc_julian_date;
}

std::vector<Track_Irradiance> OSMReader::Get_Irradiance_Data(){
    /*
     *
     *
     */
    std::vector<Track_Irradiance> out;

    for (unsigned int i = 0; i < num_messages; i++) {

        uint32_t number_tracks = data[i].data.num_tracks;
        if (number_tracks > 0){

            for (unsigned int track_index = 0;track_index < number_tracks; track_index++) {

                unsigned int num_track_ids = out.size();
                bool found_track_id = false;

                //TODO Assumes that there is only a single ir band. Function and struct will need to be updated if multiple bands are being tracked
                //uint32_t current_track_id = data[i].data.track_data[track_index].track_id;
                //uint32_t current_band_id = data[i].data.track_data[track_index].ir_measurements[0].band_id;
                //unsigned int out_track_index = 0;

                //for (out_track_index = 0; out_track_index < num_track_ids; ++out_track_index) {
                //    if(current_track_id == out[out_track_index].track_id){
                //        found_track_id = true;
                //        break;
                //    }
                //}

                //if(found_track_id){
				if(track_index < num_track_ids){
                    out[track_index].frame_number.push_back(i);
                    out[track_index].irradiance.push_back(data[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0]);
                }
                else{
                    Track_Irradiance temp;
                    //temp.track_id = current_track_id;
                    //temp.band_id = current_band_id;
                    temp.frame_number.push_back(i);
                    temp.irradiance.push_back(data[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0]);

                    out.push_back(temp);
                }


            }


        }
     }

    return out;
}

Plotting_Data OSMReader::Get_Boresight_Azimuth_Elevation()
{
	Plotting_Data out;

	for (unsigned int i = 0; i < num_messages; i++) {

		double azimuth = data[i].data.az_el_boresight[0];
		double elevation = data[i].data.az_el_boresight[1];


		out.azimuth.push_back(azimuth);
		out.elevation.push_back(elevation);
		out.frame_time.push_back(data[i].data.julian_date);
		out.frame_number.push_back(i + 1);

	}

	return out;
}

