#include "tracks.h"

TrackInformation::TrackInformation()
{
    osm_track_ids = std::set<int>();
    osm_frames = std::vector<TrackFrame>();
    osm_plotting_track_frames = std::vector<PlottingTrackFrame>();

    manual_track_ids = std::set<int>();
    manual_frames = std::vector<TrackFrame>();
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
    }
}

TrackInformation::TrackInformation(const std::vector<Frame> & osm_file_frames)
    : TrackInformation(static_cast<unsigned int>(osm_file_frames.size()))
{
    for (unsigned int i = 0; i < osm_file_frames.size(); i++)
    {
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

            //Needed for the data export.
            //In the future, all track data should probably live in one struct or a more logical format
            ptd.track_id = track_id;

            osm_plotting_track_frames[i].details.push_back(ptd);
        }
    }
}

int TrackInformation::get_count_of_tracks()
{
    return static_cast<int>(osm_track_ids.size());
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

std::vector<PlottingTrackFrame> TrackInformation::get_plotting_tracks()
{
    return osm_plotting_track_frames;
}

std::set<int> TrackInformation::get_manual_track_ids()
{
    return manual_track_ids;
}

void TrackInformation::add_manual_tracks(std::vector<TrackFrame> new_frames)
{
    //Assumption: TrackInformation has been initialized and the size of new_frames and manual_frames match
    for (int i = 0; i < manual_frames.size(); i++ )
    {
		for ( const auto &trackData : new_frames[i].tracks )
        {
            int track_id = trackData.first;

            manual_track_ids.insert(track_id);
            manual_frames[i].tracks[track_id] = trackData.second;
        }
    }
}

TrackFileReadResult TrackInformation::read_tracks_from_file(QString absolute_file_name) const
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