#include "osm_reader.h"

OSMReader::OSMReader()
{
	location_from_file = false;
}

OSMReader::~OSMReader()
{
}

std::vector<Frame> OSMReader::ReadOsmFileData(QString path)
{
	QByteArray array = path.toLocal8Bit();
	const char* buffer = array.constData();
	
    return LoadFrameVectors(buffer, false);
}

std::vector<Frame> OSMReader::LoadFrameVectors(const char *file_path, bool input_combine_tracks)
{
	std::vector<Frame> data = std::vector<Frame>();

    errno_t err = fopen_s(&fp, file_path, "rb");

	if (err != 0) {
		return data;
	}

	frame_time.clear();

	uint32_t num_messages = 0;
	num_messages = FindMessageNumber();
	data = LoadData(num_messages, input_combine_tracks);
    fclose(fp);

	if (data.size() < num_messages) {
		return std::vector<Frame>();
	}

	location_from_file = false;

    return data;
}

uint32_t OSMReader::FindMessageNumber()
{
	uint32_t num_messages = 0;
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

        number_iterations++;
    }

	return num_messages;
}

std::vector<Frame> OSMReader::LoadData(uint32_t num_messages, bool combine_tracks)
{
	std::vector<Frame> data = std::vector<Frame>();

    if (combine_tracks) {
		arma::vec frame_time_vec(frame_time);
		arma::vec diff = arma::diff(frame_time_vec);
		arma::uvec index = arma::find(diff != 0);
		data.reserve(index.n_elem);
    }
    else {
		data.reserve(num_messages);
    }

    fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < num_messages; i++)
    {
        bool valid_step = i == 0 || frame_time[i] - frame_time[i - 1] != 0 || !combine_tracks;
        if (valid_step) {

            Frame current_frame;

            current_frame.msg_header = ReadMessageHeader();
			if (current_frame.msg_header.size < 0)
				return std::vector<Frame>();


            current_frame.frame_header = ReadFrameHeader();
            current_frame.data = ReadFrameData();

			if (current_frame.data.ecf.size() == 0)
				return std::vector<Frame>();
			
            data.push_back(current_frame);
        }
        else
        {
			AddTrackToLastFrame(data);
        }
    }

	return data;
}

void OSMReader::AddTrackToLastFrame(std::vector<Frame> &data)
{
    //TODO: Verify that adding a track to last frame works correctly.

    Frame last_frame = data.back();
    uint32_t start_track_index = last_frame.data.num_tracks;

    last_frame.data.num_tracks += ReadValue<uint32_t>(true);

    for (uint32_t j = start_track_index; j < last_frame.data.num_tracks; j++)
    {
		TrackData current_track = GetTrackData(last_frame.data);
        last_frame.data.track_data.push_back(current_track);
    }
}

std::vector<double> OSMReader::CalculateLatLonAltVector(std::vector<double> ecf)
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

    qDebug() << "Message Header seconds = " << seconds;

    uint64_t nano_seconds = ReadValue<uint64_t>();
    qDebug() << "Message nano_seconds = " << nano_seconds;

    uint64_t tsize = ReadValue<uint64_t>();
    qDebug() << "Message tsize = " << tsize;

	MessageHeader current_message;
    if (tsize < kSMALL_NUMBER)
	{
		current_message.size = -1;
		return current_message;
    }
   
    current_message.seconds = seconds + nano_seconds * 1e-9;
    current_message.size = tsize;

    return current_message;
}

FrameHeader OSMReader::ReadFrameHeader()
{
    FrameHeader fh;
    fh.authorization = ReadValue<uint64_t>(true);

    qDebug() << "fh.authorization" << fh.authorization;

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

    qDebug() << "fh.software_version" << fh.software_version;

    return fh;
}

FrameData OSMReader::ReadFrameData()
{
    FrameData data;

    data.task_id = ReadValue<uint32_t>(true);
    uint32_t osm_seconds = ReadValue<uint32_t>(true);
    qDebug() << "osm_seconds = " << osm_seconds;

    uint32_t osm_micro_seconds = ReadValue<uint32_t>(true);

    data.frametime = osm_seconds + osm_micro_seconds * 1e-6; // GPS Time since Jan 6, 1990
    qDebug() << "data.frametime = " << data.frametime;

    data.julian_date = CalculateGpsUtcJulianDate(data.frametime);
    qDebug() << "data.julian_date = " << data.julian_date;

	double modified_julian_date = data.julian_date + 0.5;
	int midnight_julian = std::floor(modified_julian_date);
	data.seconds_past_midnight = (modified_julian_date - midnight_julian) * 86400.;

    qDebug() << "data.seconds_past_midnight = " << data.seconds_past_midnight;

    data.mrp = ReadMultipleDoubleValues(3, true);

    qDebug() << "data.mrp = " << data.mrp;

    data.mrp_cov_rand = ReadMultipleDoubleValues(6, true);
    data.mrp_cov_bias = ReadMultipleDoubleValues(6, true);

	//-----------------------------------------------------------------------------------------------------------------
	data.ecf = ReadMultipleDoubleValues(6, true);

    double sum = data.ecf[0] + data.ecf[1] + data.ecf[2] + data.ecf[3] + data.ecf[4] + data.ecf[5];

	if (sum < kSMALL_NUMBER)
	{
		if (location_from_file)
		{
			data.ecf = file_ecef_vector;
		}
		else
		{
			LocationInput get_location_file;
			auto response = get_location_file.exec();
			location_from_file = true;
			
			if (response && get_location_file.path_set)
			{
				file_ecef_vector = get_location_file.GetECEFVector();
			}
			else
			{
				FrameData bad_input;
				location_from_file = false;
				return bad_input;
			}
		}
	}
	//-----------------------------------------------------------------------------------------------------------------

    data.lla = CalculateLatLonAltVector(data.ecf);

    data.dcm = CalculateDirectionCosineMatrix(data.mrp);

    data.i_fov_x = ReadValue<double>(true);
    data.i_fov_y = ReadValue<double>(true);
    data.num_tracks = ReadValue<uint32_t>(true);
	
    std::vector<double> az_el_boresight = CalculateAzimuthElevation(0, 0, data);
	data.az_el_boresight = az_el_boresight;

    for (uint32_t j = 0; j < data.num_tracks; j++)
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
        IrradianceData ir_data;

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

    std::vector<double> az_el_track = CalculateAzimuthElevation(current_track.centroid_x, current_track.centroid_y, input);
	current_track.az_el_track = az_el_track;

    current_track.covariance = ReadValue<double>(true);

    current_track.num_pixels = ReadValue<uint32_t>(true);
    current_track.object_type = ReadValue<uint32_t>(true);

    return current_track;
}

std::vector<double> OSMReader::CalculateDirectionCosineMatrix(std::vector<double> input)
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

	arma::vec temp = arma::conv_to<arma::vec>::from(arma::vectorise(dcos));
	std::vector<double> out = arma::conv_to<std::vector<double>>::from(temp);

	return out;
}

std::vector<double> OSMReader::CalculateAzimuthElevation(int x_pixel, int y_pixel, FrameData & input)
{
	std::vector<double> results = AzElCalculation::calculate(x_pixel, y_pixel, input.lla[0], input.lla[1], input.dcm, input.i_fov_x, input.i_fov_y, false);

    return results;
}

double OSMReader::CalculateGpsUtcJulianDate(double offset_gps_seconds)
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
	double base = jtime::JulianDate(1980, 1, 6, 0, 0, 0);
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
