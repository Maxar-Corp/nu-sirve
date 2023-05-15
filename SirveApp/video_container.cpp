#include "video_container.h"

Video_Container::Video_Container()
{

}

Video_Container::~Video_Container()
{
	something.clear();
}

void Video_Container::reset(video_details input)
{
	something.clear();
	something.push_back(input);
	emit update_display_video(input);
}

void Video_Container::display_original_data()
{
	emit update_display_video(something[0]);
}

bool Video_Container::display_data(video_details input)
{	
	for (int i = 0; i < something.size(); i++)
	{
		if (input == something[i]) {
			emit update_display_video(something[i]);
			DEBUG << "Video Container: Updating display video to index of " << i;
			return true;
		}
	}

	return false;
}

int Video_Container::find_data_index(video_details input)
{
	int number_videos = something.size();

	for (int i = 0; i < number_videos; i++)
	{
		if (input == something[i]) {
			DEBUG << "Video Container: Match for video found at index " << i;
			return i;
		}
	}

	return -1;
}
