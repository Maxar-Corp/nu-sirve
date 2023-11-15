#include "tracks.h"

TrackInformation::TrackInformation()
{
    osm_track_ids = std::set<int>();
    osm_frames = std::vector<TrackFrame>();
    osm_plotting_track_frames = std::vector<PlottingTrackFrame>();
}

TrackInformation::TrackInformation(unsigned int num_frames) : TrackInformation()
{
    for (unsigned int i = 0; i < num_frames; i++)
    {
        TrackFrame frame;
        frame.tracks = std::map<int, TrackDetails>();
        osm_frames.push_back(frame);

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

std::vector<TrackFrame> TrackInformation::get_frames(int start_index, int end_index)
{
	std::vector<TrackFrame>::const_iterator first = osm_frames.begin() + start_index;
	std::vector<TrackFrame>::const_iterator last = osm_frames.begin() + end_index;

    std::vector<TrackFrame> subset(first, last);
    return subset;
}

std::vector<PlottingTrackFrame> TrackInformation::get_plotting_tracks()
{
    return osm_plotting_track_frames;
}