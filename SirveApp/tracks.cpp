#include "tracks.h"
#include "support/az_el_calculation.h"
#include <opencv2/core/fast_math.hpp>

#include <QFile>

TrackDetails::TrackDetails(double frameTime, double julianDate, double secondPastMidnight, double timingOffset,
                           int centroidXBoresight, int centroidYBoresight, int centroidX, int centroidY, double az, double el, int peakCounts,
                           int sumCounts, int meanCounts, int numberPixels, double sumRelativeCounts, double peakIrradiance,
                           double meanIrradiance, double sumIrradiance, double sumRelativeIrradiance, int bboxX, int bboxY, int bboxWidth,
                           int bboxHeight) :
        frame_time(frameTime),
        julian_date(julianDate),
        second_past_midnight(secondPastMidnight),
        timing_offset(timingOffset),
        centroid_x_boresight(centroidXBoresight),
        centroid_y_boresight(centroidYBoresight),
        centroid_x(centroidX),
        centroid_y(centroidY),
        az(az),
        el(el),
        peak_counts(peakCounts),
        sum_counts(sumCounts),
        mean_counts(meanCounts),
        number_pixels(numberPixels),
        sum_relative_counts(sumRelativeCounts),
        peak_irradiance(peakIrradiance),
        mean_irradiance(meanIrradiance),
        sum_irradiance(sumIrradiance),
        sum_relative_irradiance(sumRelativeIrradiance),
        bbox_x(bboxX),
        bbox_y(bboxY),
        bbox_width(bboxWidth),
        bbox_height(bboxHeight) {}

TrackDetails::TrackDetails(double centroid_x, double centroid_y) :
    centroid_x(cvRound(centroid_x)), centroid_y(cvRound(centroid_y)) {}

TrackDetails::TrackDetails(const TrackData& track_data) :
    centroid_x(cvRound(track_data.centroid_x)),
    centroid_y(cvRound(track_data.centroid_y)),
    number_pixels((int)track_data.num_pixels),
    bbox_x(std::abs((int)track_data.roiURX - (int)track_data.roiBLX)),
    bbox_y(std::min(track_data.roiBLY, track_data.roiURY)),
    bbox_width(std::abs((int)track_data.roiURX - (int)track_data.roiBLX)),
    bbox_height(std::abs((int)track_data.roiURY - (int)track_data.roiBLY))
    {
        if (track_data.az_el_track.size() > 1) {
            az = track_data.az_el_track[0];
            el = track_data.az_el_track[1];
        }

        if (!track_data.ir_measurements.empty() && !track_data.ir_measurements[0].ir_radiance.empty()) {
            sum_relative_counts = track_data.ir_measurements[0].ir_radiance[0];
        }
    }

TrackDetails& TrackDetails::operator=(const TrackData& track_data)
{
    *this = TrackDetails(track_data);
    return *this;
}

TrackDetails::TrackDetails(const FrameData& frame_data, const TrackData& track_data) :
    TrackDetails(track_data)
{
    frame_time = frame_data.frame_time;
    julian_date = frame_data.julian_date;
    second_past_midnight = frame_data.seconds_past_midnight;
    assert(frame_data.num_tracks == frame_data.track_data.size());
}

PlottingTrackDetails::PlottingTrackDetails(int trackId, TrackDetails centroid, double sumRelativeCounts, double azimuth,
                                           double elevation, double irradiance):
        track_id(trackId),
        centroid(std::move(centroid)),
        sum_relative_counts(sumRelativeCounts),
        azimuth(azimuth),
        elevation(elevation),
        irradiance(irradiance) {}

PlottingTrackDetails::PlottingTrackDetails(const TrackData& track_data) :
    track_id(track_data.track_id), centroid(track_data.centroid_x, track_data.centroid_y)
{
    if (!track_data.ir_measurements.empty() && !track_data.ir_measurements[0].ir_radiance.empty()) {
        sum_relative_counts = track_data.ir_measurements[0].ir_radiance[0];
    }

    if (track_data.az_el_track.size() > 1) {
        azimuth = track_data.az_el_track[0];
        elevation = track_data.az_el_track[1];
    }
}

PlottingTrackDetails& PlottingTrackDetails::operator=(const TrackData& track_data)
{
    *this = PlottingTrackDetails(track_data);
    return *this;
}

TrackEngineeringData::TrackEngineeringData(double iFovX, double iFovY, double boresightLat, double boresightLong,
                                           std::vector<double> dcm):
    i_fov_x(iFovX),
    i_fov_y(iFovY),
    boresight_lat(boresightLat),
    boresight_long(boresightLong),
    dcm(std::move(dcm)) {}

TrackEngineeringData::TrackEngineeringData(const FrameData& frame_data) :
    TrackEngineeringData(frame_data.i_fov_x, frame_data.i_fov_y, 0.0, 0.0, frame_data.dcm)
{
    if (frame_data.lla.size() > 1)
    {
        boresight_lat = frame_data.lla[0];
        boresight_long = frame_data.lla[1];
    }
}

TrackEngineeringData& TrackEngineeringData::operator=(const FrameData& frame_data)
{
    *this = TrackEngineeringData(frame_data);
    return *this;
}

TrackInformation::TrackInformation(size_t num_frames) :
    osm_plotting_track_frames(num_frames),
    osm_frames(num_frames),
    track_engineering_data(num_frames),
    manual_frames(num_frames),
    manual_plotting_frames(num_frames),
    manual_image_frames(num_frames) {}

TrackInformation::TrackInformation(const std::vector<Frame> & osm_file_frames, ABPFileType file_type)
    : TrackInformation(osm_file_frames.size())
{
    if (file_type == ABPFileType::ABP_D)
    {
        nRows = 720;
        nCols = 1280;
    }

    for (unsigned int frame_index = 0; frame_index < osm_file_frames.size(); frame_index++)
    {
        //Here we retain all the track "engineering" data (boresight lat/long, ifov, dcm)
        //This is required to later calculate az/el for manual tracks
        const auto& frame_data = osm_file_frames[frame_index].data;
        track_engineering_data[frame_index] = frame_data;

        for (const auto& track_data : frame_data.track_data) {
            // This is the "ideal" representation of a track
            // For each frame (TrackFrame), there is information about any tracks in that frame (TrackDetails)
            // The track details are mapped (hash table/lookup) by their track_id
            osm_track_ids.insert(track_data.track_id);
            osm_frames[frame_index].tracks.emplace(track_data.track_id, TrackDetails { frame_data, track_data });

            // This is a "combined" track representation that I'm only keeping around because I'm not smart enough
            // to replace it yet
            // Across frames, it treats the first track as track 0, the second as track 1, etc.
            // In the future, we'll want to change this and the plotting code to stop merging tracks
            osm_plotting_track_frames[frame_index].details.emplace_back(track_data);
        }
    }
}

size_t TrackInformation::GetTrackCount() const
{
    return osm_track_ids.size();
}

size_t TrackInformation::GetFrameCount() const
{
    return osm_frames.size();
}

std::vector<TrackFrame> TrackInformation::GetOsmFrames(size_t start_index, size_t end_index) const
{
    if (start_index > osm_frames.size() || end_index > osm_frames.size() || start_index > end_index) {
        return {};
    }
    return { osm_frames.begin() + start_index, osm_frames.begin() + end_index };
}

std::vector<TrackFrame> TrackInformation::GetManualFrames(int start_index, int end_index) const
{
    if (start_index > manual_frames.size() || end_index > manual_frames.size() || start_index > end_index) {
        return {};
    }
    return { manual_frames.begin() + start_index, manual_frames.begin() + end_index };
}

std::vector<PlottingTrackFrame>& TrackInformation::GetOsmPlottingTrackFrames()
{
    return osm_plotting_track_frames;
}

const std::vector<ManualPlottingTrackFrame>& TrackInformation::GetManualPlottingFrames()
{
    return manual_plotting_frames;
}

const std::vector<TrackFrame>& TrackInformation::GetManualImageFrames()
{
    return manual_image_frames;
}

const std::set<int>& TrackInformation::GetManualTrackIds()
{
    return manual_track_ids;
}

const std::set<int>& TrackInformation::GetOsmTrackIds()
{
    return osm_track_ids;
}

void TrackInformation::AddManualTracks(const std::vector<TrackFrame>& new_frames)
{
    //Assumption: TrackInformation has been initialized and the size of new_frames and manual_frames match
    for (int i = 0; i < manual_frames.size(); i++)
    {
        for (const auto &trackData : new_frames[i].tracks)
        {
            int track_id = trackData.first;
            RemoveManualTrackPlotting(track_id);
            RemoveManualTrackImage(track_id);
        }
    }
    for (int i = 0; i < manual_frames.size(); i++)
    {
        TrackEngineeringData eng_data = track_engineering_data[i];
        for (const auto& track_data : new_frames[i].tracks)
        {
            int track_id = track_data.first;
            manual_track_ids.insert(track_id);
            std::pair<const int, TrackDetails> temp_track_data = track_data;
            if(track_data.second.centroid_x>nCols || track_data.second.centroid_x_boresight>nCols ||
            track_data.second.centroid_y>nRows || track_data.second.centroid_y_boresight>nRows){
                std::vector<int> x_y_result = AzElCalculation::calculateXY(nRows, nCols, temp_track_data.second.az,temp_track_data.second.el, eng_data.boresight_lat, eng_data.boresight_long, eng_data.dcm, eng_data.i_fov_x, eng_data.i_fov_y);
                temp_track_data.second.centroid_x_boresight = x_y_result[0];
                temp_track_data.second.centroid_y_boresight = x_y_result[1];
                temp_track_data.second.centroid_x = x_y_result[0] + nCols/2;
                temp_track_data.second.centroid_y = x_y_result[1] + nRows/2;
                manual_image_frames[i].tracks[track_id].centroid_x = temp_track_data.second.centroid_x;
                manual_image_frames[i].tracks[track_id].centroid_y = temp_track_data.second.centroid_y;
            }
            else{
                manual_image_frames[i].tracks[track_id].centroid_x = track_data.second.centroid_x;
                manual_image_frames[i].tracks[track_id].centroid_y = track_data.second.centroid_y;
            }
            manual_plotting_frames[i].tracks[track_id] = GetManualPlottingTrackDetails(i, temp_track_data.second.centroid_x, temp_track_data.second.centroid_y, temp_track_data.second.sum_relative_counts);
            manual_frames[i].tracks[track_id] = temp_track_data.second;
            manual_plotting_frames[i].tracks[track_id].sum_relative_counts = temp_track_data.second.sum_relative_counts;
        }
    }
}

void TrackInformation::RemoveManualTrack(int track_id)
{
    manual_track_ids.erase(track_id);
    for (int i = 0; i < manual_frames.size(); i++ )
    {
        manual_frames[i].tracks.erase(track_id);
    }
}

void TrackInformation::RemoveManualTrackPlotting(int track_id)
{
    // manual_track_ids.erase(track_id);
    for (int i = 0; i < manual_frames.size(); i++ )
    {
        manual_plotting_frames[i].tracks.erase(track_id);
    }
}


void TrackInformation::RemoveManualTrackImage(int track_id)
{
    // manual_track_ids.erase(track_id);
    for (int i = 0; i < manual_frames.size(); i++ )
    {
        manual_image_frames[i].tracks.erase(track_id);
    }
}

void TrackInformation::AddCreatedManualTrack(const std::vector<PlottingFrameData>& frame_data, int track_id, const std::vector<std::optional<TrackDetails>> & new_track_details, const QString& new_track_file_name)
{
    //Assumption: TrackInformation has been initialized and the size of new_track_details and manual_frames match
    manual_track_ids.insert(track_id);

    RemoveManualTrackPlotting(track_id);
    RemoveManualTrackImage(track_id);
    WriteManualTrackToFile(frame_data,track_id, new_track_details,new_track_file_name);
}

void TrackInformation::WriteManualTrackToFile(const std::vector<PlottingFrameData>& frame_data, int track_id, const std::vector<std::optional<TrackDetails>> & new_track_details, const QString& new_track_file_name)
{
    QFile file(new_track_file_name);
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    QString csv_line0 = "TrackID, Frame Number, Frame Time, Julian Date, Second Past Midnight, Timing Offset, Centroid X Boresight, Centroid Y Boresight, Centroid X, Centroid Y, Azimuth, Elevation, Number Pixels, Peak Counts, Mean Counts, Sum Counts, Sum Relative Counts, Peak Irradiance, Mean Irradiance, Sum Irradiance, bbox_x, bbox_y, bbox_width, bbox_height";
    file.write(csv_line0.toUtf8());
    file.write("\n");
    for (int i = 0; i < manual_frames.size(); i++)
    {
        if (new_track_details[i].has_value())
        {
            TrackDetails track_details = new_track_details[i].value();
            manual_frames[i].tracks[track_id] = track_details;
            manual_plotting_frames[i].tracks[track_id] = GetManualPlottingTrackDetails(i, track_details.centroid_x, track_details.centroid_y, track_details.sum_relative_counts);
            track_details.frame_time = frame_data[i].frame_time;
            track_details.julian_date = frame_data[i].julian_date;
            track_details.second_past_midnight = frame_data[i].seconds_past_midnight;
            track_details.timing_offset = osm_frames[i].tracks[0].timing_offset;
            track_details.az = manual_plotting_frames[i].tracks[track_id].azimuth;
            track_details.el = manual_plotting_frames[i].tracks[track_id].elevation;

            QString csv_line =
             QString::number(track_id) + ","
             + QString::number(i+1) + ","
             + QString::number(track_details.frame_time,'f',9) + ","
             + QString::number(track_details.julian_date,'f',9) + ","
             + QString::number(track_details.second_past_midnight,'f',9) + ","
             + QString::number(track_details.timing_offset) + ","
             + QString::number(track_details.centroid_x_boresight + 1) + ","
             + QString::number(track_details.centroid_y_boresight + 1) + ","
             + QString::number(track_details.centroid_x + 1) + ","
             + QString::number(track_details.centroid_y + 1) + ","
             + QString::number(track_details.az,'f',6) + ","
             + QString::number(track_details.el,'f',6) + ","
             + QString::number(track_details.number_pixels) + ","
             + QString::number(static_cast<uint32_t>(track_details.peak_counts)) + ","
             + QString::number(static_cast<uint32_t>(track_details.mean_counts)) + ","
             + QString::number(static_cast<uint32_t>(track_details.sum_counts)) + ","
             + QString::number(static_cast<uint32_t>(track_details.sum_relative_counts)) + ","
             + QString::number(track_details.peak_irradiance) + ","
             + QString::number(track_details.mean_irradiance) + ","
             + QString::number(track_details.sum_irradiance) + ","
             + QString::number(track_details.bbox_x + 1) + ","
             + QString::number(track_details.bbox_y + 1) + ","
             + QString::number(track_details.bbox_width) + ","
             + QString::number(track_details.bbox_height);

            file.write(csv_line.toUtf8());
            file.write("\n");

            manual_plotting_frames[i].tracks[track_id].sum_relative_counts = track_details.sum_relative_counts;
            manual_plotting_frames[i].tracks[track_id].peak_irradiance = track_details.peak_irradiance;
            manual_plotting_frames[i].tracks[track_id].mean_irradiance = track_details.mean_irradiance;
            manual_plotting_frames[i].tracks[track_id].sum_irradiance = track_details.sum_irradiance;
            manual_image_frames[i].tracks[track_id].centroid_x = track_details.centroid_x;
            manual_image_frames[i].tracks[track_id].centroid_y = track_details.centroid_y;
            manual_image_frames[i].tracks[track_id].sum_relative_counts = track_details.sum_relative_counts;
        }
    }

    file.close();
}


std::vector<std::optional<TrackDetails>> TrackInformation::CopyManualTrack(int track_id)
{
    std::vector<std::optional<TrackDetails>> single_track_frames;
    for (int i = 0; i < manual_frames.size(); i++ )
    {
        if (auto search = manual_frames[i].tracks.find(track_id); search != manual_frames[i].tracks.end())
        {
            single_track_frames.push_back(manual_frames[i].tracks[track_id]);
        }
        else
        {
            single_track_frames.push_back(std::nullopt);
        }
    }
    return single_track_frames;
}

std::vector<std::optional<TrackDetails>> TrackInformation::GetEmptyTrack() const
{
    return { manual_frames.size(), std::nullopt };
}

TrackFileReadResult TrackInformation::ReadTracksFromFile(QString absolute_file_name) const
{
    size_t num_frames = manual_frames.size();

    std::vector<TrackFrame> track_frames_from_file;
    std::set<int> track_ids_in_file;
    QString error_string = "";

    for (int i = 0; i < num_frames; i++)
    {
        TrackFrame frame_from_file;
        frame_from_file.tracks = std::map<int, TrackDetails>();
        track_frames_from_file.push_back(frame_from_file);
    }

    int line_num = 0;
    bool ok;
    bool computeXY;
    try
    {
        QFile file(absolute_file_name);
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        QByteArray line = file.readLine();
        while (!file.atEnd())
        {
            computeXY = false;
            line_num += 1;

            line = file.readLine();
            QList<QByteArray> cells = line.split(',');

            int track_id = cells[0].toInt(&ok);
            if (!ok) throw std::runtime_error("Track ID");
            int frame_number = cells[1].toInt(&ok);
            if (!ok) throw std::runtime_error("Frame Number");
            double frame_time = cells[2].toDouble(&ok);
            if (!ok) throw std::runtime_error("Frame Time");
            double julian_date = cells[3].toDouble(&ok);
            if (!ok) throw std::runtime_error("Julian Date");
            double seconds_past_midnight = cells[4].toDouble(&ok);
            if (!ok) throw std::runtime_error("Seconds Past Midnight");
            double timing_offset = cells[5].toDouble(&ok);
            if (!ok) throw std::runtime_error("Timing Offset");
            int track_x_boresight = cells[6].toInt(&ok) - 1;
            if (!ok){
                computeXY = true;
            }
            int track_y_boresight = cells[7].toInt(&ok) -1;
            if (!ok || computeXY){
                computeXY = true;
            }
            int track_x = cells[8].toInt(&ok) - 1;
            if (!ok || computeXY){
                computeXY = true;
            }
            int track_y = cells[9].toInt(&ok) - 1;
            if (!ok || computeXY){
                computeXY = true;
            }
            if (computeXY){
                track_x_boresight = 32768;
                track_y_boresight = 32768;
                track_x = 32768;
                track_y = 32768;
            }
            double track_az = cells[10].toDouble(&ok);
            if (!ok) throw std::runtime_error("Track Az Value");
            double track_el = cells[11].toDouble(&ok);
            if (!ok) throw std::runtime_error("Track El Value");
            int number_pixels = cells[12].toInt(&ok);
            if (!ok) throw std::runtime_error("Track Number Pixels");
            int peak_counts = cells[13].toInt(&ok);
            if (!ok) throw std::runtime_error("Track Peak Counts");
            int mean_counts = cells[14].toInt(&ok);
            if (!ok) throw std::runtime_error("Track Mean Counts");
            int sum_counts = cells[15].toInt(&ok);
            if (!ok) throw std::runtime_error("Track Sum Counts");
            int sum_relative_counts = cells[16].toInt(&ok);
            if (!ok) throw std::runtime_error("Track Sum Relative Counts");
            int peak_irradiance = cells[17].toDouble(&ok);
            if (!ok) throw std::runtime_error("Track Peak Irradiace");
            int mean_irradiance = cells[18].toDouble(&ok);
            if (!ok) throw std::runtime_error("Track Mean Irradiace");
            int sum_irradiance = cells[19].toDouble(&ok);
            if (!ok) throw std::runtime_error("Track Sum Irradiace");
            int bbox_x = cells[20].toInt(&ok);
            if (!ok) throw std::runtime_error("BBOX X");
            int bbox_y = cells[21].toInt(&ok);
            if (!ok) throw std::runtime_error("BBOX Y");
            int bbox_width = cells[22].toInt(&ok);
            if (!ok) throw std::runtime_error("BBOX Width");
            int bbox_height = cells[23].toInt(&ok);
            if (!ok) throw std::runtime_error("BBOX Height");
            if (frame_number < 0 || frame_number > num_frames) throw std::runtime_error("Invalid frame number");

            TrackDetails td;
            td.frame_time = frame_time;
            td.julian_date = julian_date;
            td.second_past_midnight = seconds_past_midnight;
            td.timing_offset = timing_offset;
            td.centroid_x_boresight = track_x_boresight;
            td.centroid_y_boresight = track_y_boresight;
            td.centroid_x = track_x;
            td.centroid_y = track_y;
            td.az = track_az;
            td.el = track_el;
            td.number_pixels = number_pixels;
            td.peak_counts = peak_counts;
            td.sum_counts = sum_counts;
            td.mean_counts = mean_counts;
            td.sum_relative_counts = sum_relative_counts;
            td.peak_irradiance = peak_irradiance;
            td.mean_irradiance = mean_irradiance;
            td.sum_irradiance = sum_irradiance;
            td.bbox_x = bbox_x;
            td.bbox_y = bbox_y;
            td.bbox_width = bbox_width;
            td.bbox_height = bbox_height;
            track_ids_in_file.insert(track_id);
            track_frames_from_file[frame_number - 1].tracks[track_id] = td;
        }
    }
    catch (const std::runtime_error& e)
    {
        error_string = "Issue reading track data at line " + QString::number(line_num) + ": " + e.what();
        return TrackFileReadResult {std::vector<TrackFrame>(), std::set<int>(), error_string};
    }
    catch (...)
    {
        return TrackFileReadResult {std::vector<TrackFrame>(), std::set<int>(), "Unexpected error opening or reading track data file"};
    }

    return TrackFileReadResult {track_frames_from_file, track_ids_in_file, ""};
}

ManualPlottingTrackDetails TrackInformation::GetManualPlottingTrackDetails(int frame_number, int centroid_x, int centroid_y, double sum_relative_counts) const
{
    TrackEngineeringData eng_data = track_engineering_data[frame_number];

    ManualPlottingTrackDetails details;

    bool adjust_frame_ref = true;

    if (centroid_x < 0)
    {
        centroid_x = centroid_x + nCols;
    }
    if (centroid_y < 0)
    {
        centroid_y = centroid_y + nRows;
    }
    if (centroid_x > nCols)
    {
        centroid_x = centroid_x - nCols;
    }
    if (centroid_y > nRows)
    {
        centroid_y = centroid_y - nRows;
    }

    std::vector<double> az_el_result = AzElCalculation::calculateAzEl(nRows, nCols, centroid_x, centroid_y, eng_data.boresight_lat, eng_data.boresight_long, eng_data.dcm, eng_data.i_fov_x, eng_data.i_fov_y, adjust_frame_ref);
    details.azimuth = az_el_result[0];
    details.elevation = az_el_result[1];

    details.centroid.centroid_x = centroid_x;
    details.centroid.centroid_y = centroid_y;
    details.sum_relative_counts = sum_relative_counts;

    return details;
}
