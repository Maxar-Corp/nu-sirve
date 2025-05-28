#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <vector>

struct FrameHeader
{
	FrameHeader() = default;
	FrameHeader(const FrameHeader&) = default;
	FrameHeader& operator=(const FrameHeader&) = default;

	FrameHeader(uint64_t authorization, uint32_t classification, uint32_t type, uint32_t priority, uint32_t operIndicator,
		uint32_t infoSource, uint32_t infoDestination, uint32_t transactionId, uint32_t ackReqIndicator, uint32_t ackResponse,
		uint32_t cantProReason, uint32_t messageLength, uint32_t softwareVersion, double timeGeneratedSeconds);

	uint64_t authorization = 0;
	uint32_t classification = 0;
	uint32_t type = 0;
	uint32_t priority = 0;
	uint32_t oper_indicator = 0;
	uint32_t info_source = 0;
	uint32_t info_destination = 0;
	uint32_t transaction_id = 0;
	uint32_t ack_req_indicator = 0;
	uint32_t ack_response = 0;
	uint32_t cant_pro_reason = 0;
	uint32_t message_length = 0;
	uint32_t software_version = 0;

	double time_generated_seconds = 0.0;
};

struct MessageHeader
{
	MessageHeader() = default;
	MessageHeader(const MessageHeader&) = default;
	MessageHeader& operator=(const MessageHeader&) = default;

	MessageHeader(uint64_t size, double seconds);

	uint64_t size = 0;
	double seconds = 0.0;
};

struct SumCountsData
{
	SumCountsData() = default;
	SumCountsData(const SumCountsData&) = default;
	SumCountsData& operator=(const SumCountsData&) = default;

	SumCountsData(uint32_t bandId, uint32_t numMeasurements, std::vector<double> time, std::vector<double> irRadiance,
		std::vector<double> irSigma);

	uint32_t band_id = 0;
	uint32_t num_measurements = 0;

	std::vector<double> time, ir_radiance, ir_sigma;
};

struct TrackData
{
	TrackData() = default;
	TrackData(const TrackData&) = default;
	TrackData& operator=(const TrackData&) = default;

	TrackData(uint32_t trackId, uint32_t sensorType, uint32_t numBands, uint32_t roiBlx, uint32_t roiBly, uint32_t roiUrx, uint32_t roiUry,
		uint32_t numPixels, uint32_t objectType, double semiMajorAxis, double semiMinorAxis, double orientationAngle, double maxX,
		double maxY, double frameStabilizeX, double frameStabilizeY, double stableFrameShiftX, double stableFrameShiftY, double centroidX,
		double centroidY, double centroidVarianceX, double centroidVarianceY, double covariance, std::vector<SumCountsData> irMeasurements,
		std::vector<double> azElTrack);

	uint32_t track_id = 0;
	uint32_t sensor_type = 0;
	uint32_t num_bands = 0;

	uint32_t roiBLX = 0;
	uint32_t roiBLY = 0;
	uint32_t roiURX = 0;
	uint32_t roiURY = 0;
	uint32_t num_pixels = 0;
	uint32_t object_type = 0;

	double semi_major_axis = 0.0;
	double semi_minor_axis = 0.0;
	double orientation_angle = 0.0;
	double maxX = 0.0;
	double maxY = 0.0;
	double frame_stabilize_x = 0.0;
	double frame_stabilize_y = 0.0;
	double stable_frame_shift_x = 0.0;
	double stable_frame_shift_y = 0.0;
	double centroid_x = 0.0;
	double centroid_y = 0.0;
	double centroid_variance_x = 0.0;
	double centroid_variance_y = 0.0;
	double covariance = 0.0;

    std::vector<SumCountsData> ir_measurements;
	std::vector <double> az_el_track;

};

struct FrameData
{
	FrameData() = default;
	FrameData(const FrameData&) = default;
	FrameData& operator=(const FrameData&) = default;

	FrameData(uint32_t taskId, uint32_t numTracks, double frame_time, double iFovX, double iFovY, double julianDate,
		double secondsPastMidnight, std::vector<double> mrp, std::vector<double> mrpCovRand, std::vector<double> mrpCovBias,
		std::vector<double> ecf, std::vector<double> lla, std::vector<double> dcm, std::vector<double> azElBoresight,
		std::vector<TrackData> trackData);

	uint32_t task_id = 0;
	uint32_t num_tracks = 0;
	double frame_time = 0.0;
	double i_fov_x = 0.0;
	double i_fov_y = 0.0;
	double julian_date = 0.0;
	double seconds_past_midnight = 0.0;

	std::vector<double>  mrp, mrp_cov_rand, mrp_cov_bias, ecf, lla, dcm, az_el_boresight;

	std::vector<TrackData> track_data;
};

struct Frame
{
	Frame() = default;
	Frame(const Frame&) = default;

	Frame(FrameHeader frameHeader, MessageHeader msgHeader, FrameData data);

	FrameHeader frame_header;
	MessageHeader msg_header;
	FrameData data;

};

struct VideoInfo
{
	VideoInfo() = default;
	VideoInfo(const VideoInfo&) = default;

	VideoInfo(int xPixels, int yPixels, int minFrame, int maxFrame, int xCorrection, int yCorrection);

	int x_pixels = 0;
	int y_pixels = 0;
	int min_frame = 0;
	int max_frame = 0;
	int x_correction = 0;
	int y_correction = 0;
};

struct PlottingFrameData
{
	PlottingFrameData() = default;
	PlottingFrameData(const PlottingFrameData&) = default;
	PlottingFrameData& operator=(const PlottingFrameData&) = default;

	PlottingFrameData(double azimuthSensor, double elevationSensor, double julianDate, double secondsPastMidnight, double timingOffset,
		double frameTime);

	double azimuth_sensor = 0.0;
	double elevation_sensor = 0.0;
	double julian_date = 0.0;
	double seconds_past_midnight = 0.0;
	double timing_offset = 0.0;
	double frame_time = 0.0;
};

#endif
