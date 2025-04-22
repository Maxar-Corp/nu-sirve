#include "tracks.h"
#include "constants.h"
#include "QFile"
#include "support/az_el_calculation.h"

TrackInformation::TrackInformation()
{
    osm_track_ids = std::set<int>();
    osm_frames = std::vector<TrackFrame>();
    osm_plotting_track_frames = std::vector<PlottingTrackFrame>();

    manual_track_ids = std::set<int>();

    manual_frames = std::vector<TrackFrame>();

    track_engineering_data = std::vector<TrackEngineeringData>();

    manual_plotting_frames = std::vector<ManualPlottingTrackFrame>();

    manual_image_frames = std::vector<TrackFrame>();
}

TrackInformation::TrackInformation(unsigned int num_frames) : TrackInformation()
{
    for (unsigned int i = 0; i < num_frames; i++)
    {
        TrackFrame frame;
        frame.tracks = std::map<int, TrackDetails>();
        osm_frames.push_back(frame);

        TrackFrame manual_frame;
        manual_frame.tracks = std::map<int, TrackDetails>();
        manual_frames.push_back(manual_frame);

        PlottingTrackFrame track_frame;
        track_frame.details = std::vector<PlottingTrackDetails>();
        osm_plotting_track_frames.push_back(track_frame);

        track_engineering_data.push_back(TrackEngineeringData());

        ManualPlottingTrackFrame manual_plotting_track_frame;
        manual_plotting_track_frame.tracks = std::map<int,ManualPlottingTrackDetails>();
        manual_plotting_frames.push_back(manual_plotting_track_frame);

        TrackFrame manual_image_track_frame;
        manual_image_track_frame.tracks = std::map<int, TrackDetails>();
        manual_image_frames.push_back(manual_image_track_frame);
    }
}

TrackInformation::TrackInformation(const std::vector<Frame> & osm_file_frames, ABPFileType file_type)
    : TrackInformation(static_cast<unsigned int>(osm_file_frames.size()))
{
    if (file_type == ABPFileType::ABP_D)
    {
        int nRows = 720;
        int nCols = 1280;
    }

    for (unsigned int i = 0; i < osm_file_frames.size(); i++)
    {
        //Here we retain all the track "engineering" data (boresight lat/long, ifov, dcm)
        //This is required to later calculate az/el for manual tracks
        track_engineering_data[i].boresight_lat = osm_file_frames[i].data.lla[0];
        track_engineering_data[i].boresight_long = osm_file_frames[i].data.lla[1];
        track_engineering_data[i].dcm = osm_file_frames[i].data.dcm;
        track_engineering_data[i].i_fov_x = osm_file_frames[i].data.i_fov_x;
        track_engineering_data[i].i_fov_y = osm_file_frames[i].data.i_fov_y;

        int number_tracks = osm_file_frames[i].data.num_tracks;
        
        for (int track_index = 0; track_index < number_tracks; track_index++)
        {
            //This is the "ideal" representation of a track
            //For each frame (TrackFrame), there is information about any tracks in that frame (TrackDetails)
            //The track details are mapped (hash table/lookup) by their track_id
            TrackDetails td;
            td.centroid_x = std::lround(osm_file_frames[i].data.track_data[track_index].centroid_x);
            td.centroid_y = std::lround(osm_file_frames[i].data.track_data[track_index].centroid_y);
            int track_id = osm_file_frames[i].data.track_data[track_index].track_id;
            osm_track_ids.insert(track_id);
            osm_frames[i].tracks[track_id] = td;

            //This is a "combined" track representation that I'm only keeping around because I'm not smart enough to replace it yet
            //Across frames, it treats the first track as track 0, the second as track 1, etc.
            //In the future, we'll want to change this and the plotting code to stop merging tracks
            PlottingTrackDetails ptd;

            //Note: This line assumes that there is only a single ir band, code will need updated if this changes
            ptd.sum_relative_counts = osm_file_frames[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0];
			ptd.azimuth = osm_file_frames[i].data.track_data[track_index].az_el_track[0];
			ptd.elevation = osm_file_frames[i].data.track_data[track_index].az_el_track[1];
            ptd.centroid.centroid_x = td.centroid_x;
            ptd.centroid.centroid_y = td.centroid_y;

            //Needed for the data export.
            //In the future, all track data should probably live in one struct or a more logical format
            ptd.track_id = track_id;

            osm_plotting_track_frames[i].details.push_back(ptd);
        }
    }
}

int TrackInformation::get_track_count() const
{
    return static_cast<int>(osm_track_ids.size());
}

int TrackInformation::get_frame_count() const
{
    return static_cast<int>(osm_frames.size());
}

std::vector<TrackFrame> TrackInformation::get_osm_frames(int start_index, int end_index)
{
	std::vector<TrackFrame>::const_iterator first = osm_frames.begin() + start_index;
	std::vector<TrackFrame>::const_iterator last = osm_frames.begin() + end_index;

    std::vector<TrackFrame> subset(first, last);
    
    return subset;
}

std::vector<TrackFrame> TrackInformation::get_manual_frames(int start_index, int end_index)
{
	std::vector<TrackFrame>::const_iterator first = manual_frames.begin() + start_index;
	std::vector<TrackFrame>::const_iterator last = manual_frames.begin() + end_index;

    std::vector<TrackFrame> subset(first, last);

    return subset;
}

std::vector<PlottingTrackFrame> TrackInformation::get_osm_plotting_track_frames()
{
    return osm_plotting_track_frames;
}

std::vector<ManualPlottingTrackFrame> TrackInformation::get_manual_plotting_frames()
{
    return manual_plotting_frames;
}

std::vector<TrackFrame> TrackInformation::get_manual_image_frames()
{
    return manual_image_frames;
}

std::set<int> TrackInformation::get_manual_track_ids()
{
    return manual_track_ids;
}

std::set<int> TrackInformation::get_OSM_track_ids()
{
    return osm_track_ids;
}


void TrackInformation::AddManualTracks(const std::vector<TrackFrame>& new_frames)
{
    //Assumption: TrackInformation has been initialized and the size of new_frames and manual_frames match
    for (int i = 0; i < manual_frames.size(); i++ )
    {
		for ( const auto &trackData : new_frames[i].tracks )
        {
            int track_id = trackData.first;
            RemoveManualTrackPlotting(track_id);
            RemoveManualTrackImage(track_id);
        }
    }
    for (int i = 0; i < manual_frames.size(); i++ )
    {
		for ( const auto &trackData : new_frames[i].tracks )
        {
            int track_id = trackData.first;
            manual_track_ids.insert(track_id);
            manual_frames[i].tracks[track_id] = trackData.second;
            manual_plotting_frames[i].tracks[track_id] = GetManualPlottingTrackDetails(i, trackData.second.centroid_x-1, trackData.second.centroid_y-1, trackData.second.sum_relative_counts);
            manual_image_frames[i].tracks[track_id].centroid_x = trackData.second.centroid_x-1;
            manual_image_frames[i].tracks[track_id].centroid_y = trackData.second.centroid_y-1;
            manual_plotting_frames[i].tracks[track_id].sum_relative_counts = trackData.second.sum_relative_counts;
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

    QFile file(new_track_file_name);
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    QString csv_line0 = "TrackID, Frame Number, Frame Time, Julian Date, Second Past Midnight, Timeing Offset, Centroid X Boresight, Centroid Y Boresight, Centroid X, Centroid Y, Azimuth, Elevation, Number Pixels, Peak Counts, Mean Counts, Sum Counts, Sum Relative Counts, Peak Irradiance, Mean Irradiance, Sum Irradiance, bbox_x, bbox_y, bbox_width, bbox_height";
    file.write(csv_line0.toUtf8());
    file.write("\n");
    RemoveManualTrackPlotting(track_id);
    RemoveManualTrackImage(track_id);
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
             + QString::number(track_details.centroid_x_boresight+1) + ","
             + QString::number(track_details.centroid_y_boresight+1) + ","
             + QString::number(track_details.centroid_x+1) + ","
             + QString::number(track_details.centroid_y+1) + ","
             + QString::number(track_details.az) + ","
             + QString::number(track_details.el) + ","
             + QString::number(track_details.number_pixels) + ","
             + QString::number(track_details.peak_counts) + ","
             + QString::number(track_details.mean_counts) + ","
             + QString::number(track_details.sum_counts) + ","
             + QString::number(track_details.sum_relative_counts) + ","
             + QString::number(track_details.peak_irradiance) + ","
             + QString::number(track_details.mean_irradiance) + ","
             + QString::number(track_details.sum_irradiance) + ","
             + QString::number(track_details.bbox_x+1) + ","
             + QString::number(track_details.bbox_y+1) + ","
             + QString::number(track_details.bbox_width) + ","
             + QString::number(track_details.bbox_height);

            file.write(csv_line.toUtf8());
            file.write("\n");

            manual_plotting_frames[i].tracks[track_id].sum_relative_counts = track_details.sum_relative_counts;
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

    try
    {
        QFile file(absolute_file_name);
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        QByteArray line = file.readLine();
        while (!file.atEnd())
        {
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
            double timeing_offset = cells[5].toDouble(&ok);
            if (!ok) throw std::runtime_error("Timeing Offset");
            int track_x_boresight = cells[6].toInt(&ok);
            if (!ok) throw std::runtime_error("Track X Boresight Value");
            int track_y_boresight = cells[7].toInt(&ok);
            if (!ok) throw std::runtime_error("Track Y Boresight Value");
            int track_x = cells[8].toInt(&ok);
            if (!ok) throw std::runtime_error("Track X Value");
            int track_y = cells[9].toInt(&ok);
            if (!ok) throw std::runtime_error("Track Y Value");
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
            td.timing_offset = timeing_offset;
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

    std::vector<double> az_el_result = AzElCalculation::calculate(nRows, nCols, centroid_x, centroid_y, eng_data.boresight_lat, eng_data.boresight_long, eng_data.dcm, eng_data.i_fov_x, eng_data.i_fov_y, adjust_frame_ref);
    details.azimuth = az_el_result[0];
    details.elevation = az_el_result[1];

    details.centroid.centroid_x = centroid_x+1;
    details.centroid.centroid_y = centroid_y+1;
    details.sum_relative_counts = sum_relative_counts;

    return details;
}
