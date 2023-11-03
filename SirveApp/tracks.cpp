#include "tracks.h"

TrackInformation::TrackInformation(const std::vector<Frame> & osm_frames)
{
    frames = std::vector<TrackFrame>();

    for (unsigned int i = 0; i < osm_frames.size(); i++)
    {
        TrackFrame frame;
        frame.tracks = std::map<int, TrackDetails>();

        int number_tracks = osm_frames[i].data.num_tracks;

        for (unsigned int track_index = 0; track_index < number_tracks; track_index++)
        {
            TrackDetails td;
            td.centroid_x = std::round(osm_frames[i].data.track_data[track_index].centroid_x);
            td.centroid_y = std::round(osm_frames[i].data.track_data[track_index].centroid_y);
            int track_id = osm_frames[i].data.track_data[track_index].track_id;
            frame.tracks[track_id] = td;
        }

        frames.push_back(frame);
    }
}

std::vector<TrackFrame> TrackInformation::get_frames(int start_index, int end_index)
{
	std::vector<TrackFrame>::const_iterator first = frames.begin() + start_index;
	std::vector<TrackFrame>::const_iterator last = frames.begin() + end_index;

    std::vector<TrackFrame> subset(first, last);
    return subset;
}