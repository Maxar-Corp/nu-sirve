#include "tracks.h"

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

TrackInformation::TrackInformation(const std::vector<Frame> & osm_file_frames)
    : TrackInformation(static_cast<unsigned int>(osm_file_frames.size()))
{
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
            ptd.irradiance = osm_file_frames[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0];
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

int TrackInformation::get_track_count()
{
    return static_cast<int>(osm_track_ids.size());
}

int TrackInformation::get_frame_count()
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
	std::vector<TrackFrame>::const_iterator first = manual_image_frames.begin() + start_index;
	std::vector<TrackFrame>::const_iterator last = manual_image_frames.begin() + end_index;

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

void TrackInformation::set_manual_frame(int frame_index, int track_id, TrackDetails *centroid) {
    manual_frames[frame_index].tracks[track_id] = *centroid;
}

void TrackInformation::AddManualTracks(std::vector<TrackFrame> new_frames)
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
            manual_plotting_frames[i].tracks[track_id] = GetManualPlottingTrackDetails(i, trackData.second.centroid_x, trackData.second.centroid_y);
            manual_image_frames[i].tracks[track_id].centroid_x = trackData.second.centroid_x;
            manual_image_frames[i].tracks[track_id].centroid_y = trackData.second.centroid_y;
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

void TrackInformation::AddCreatedManualTrack(int track_id, const std::vector<std::optional<TrackDetails>> & new_track_details, QString new_track_file_name)
{
    //Assumption: TrackInformation has been initialized and the size of new_track_details and manual_frames match
    manual_track_ids.insert(track_id);

    QFile file(new_track_file_name);
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    RemoveManualTrackPlotting(track_id);
    RemoveManualTrackImage(track_id);
    for (int i = 0; i < manual_frames.size(); i++)
    {
        if (new_track_details[i].has_value())
        {
            TrackDetails track_details = new_track_details[i].value();
            manual_frames[i].tracks[track_id] = track_details;

            QString csv_line = QString::number(track_id) + "," + QString::number(i+1) + "," + QString::number(track_details.centroid_x) + "," + QString::number(track_details.centroid_y);
            file.write(csv_line.toUtf8());
            file.write("\n");

            manual_plotting_frames[i].tracks[track_id] = GetManualPlottingTrackDetails(i, track_details.centroid_x, track_details.centroid_y);
            manual_image_frames[i].tracks[track_id].centroid_x = track_details.centroid_x;
            manual_image_frames[i].tracks[track_id].centroid_y = track_details.centroid_y;
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

std::vector<std::optional<TrackDetails>> TrackInformation::GetEmptyTrack()
{
    std::vector<std::optional<TrackDetails>> track_frames;
    for (int i = 0; i < manual_frames.size(); i++ )
    {
        track_frames.push_back(std::nullopt);
    }
    return track_frames;
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

        while (!file.atEnd())
        {
            line_num += 1;

            QByteArray line = file.readLine();
            QList<QByteArray> cells = line.split(',');

            int track_id = cells[0].toInt(&ok);
            if (!ok) throw std::runtime_error("Track ID");
            int frame_number = cells[1].toInt(&ok);
            if (!ok) throw std::runtime_error("Frame Number");
            int track_x = cells[2].toInt(&ok);
            if (!ok) throw std::runtime_error("Track X Value");
            int track_y = cells[3].toInt(&ok);
            if (!ok) throw std::runtime_error("Track Y Value");

            if (frame_number < 0 || frame_number > num_frames) throw std::runtime_error("Invalid frame number");

            TrackDetails td;
            td.centroid_x = track_x;
            td.centroid_y = track_y;
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

ManualPlottingTrackDetails TrackInformation::GetManualPlottingTrackDetails(int frame_number, int centroid_x, int centroid_y)
{
    TrackEngineeringData eng_data = track_engineering_data[frame_number];
    struct ManualPlottingTrackDetails details;
	
	bool adjust_frame_ref = true;
	
    std::vector<double> az_el_result = AzElCalculation::calculate(centroid_x, centroid_y, eng_data.boresight_lat, eng_data.boresight_long, eng_data.dcm, eng_data.i_fov_x, eng_data.i_fov_y, adjust_frame_ref);
    details.azimuth = az_el_result[0];
    details.elevation = az_el_result[1];

    details.centroid.centroid_x = centroid_x;
    details.centroid.centroid_y = centroid_y;

    return details;
}
