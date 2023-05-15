#include "video_container.h"

Video_Container::Video_Container()
{
	current_idx = -1;
}

Video_Container::~Video_Container()
{
	something.clear();
	processing_states.clear();
}

void Video_Container::select_state(int idx)
{
	current_idx = idx;

	emit update_display_video(processing_states[idx]);
}

void Video_Container::clear_processing_states()
{
	processing_states.clear();
	current_idx = -1;

	emit states_cleared();
}

void Video_Container::add_processing_state(video_details new_details)
{
	processing_states.push_back(new_details);
	current_idx = processing_states.size() - 1;

	emit state_added(QString::number(current_idx), current_idx);
	emit update_display_video(new_details);
}

video_details Video_Container::copy_current_state()
{
	return processing_states[current_idx];
}

void Video_Container::undo()
{
	if (processing_states.size() < 2) {
		return;
	}

	processing_states.pop_back();
	emit state_removed(processing_states.size());

	if (current_idx == processing_states.size()) {
		current_idx -= 1;
		emit update_display_video(processing_states[current_idx]);
	}
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