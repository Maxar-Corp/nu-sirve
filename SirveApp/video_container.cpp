#include "video_container.h"

Video_Container::Video_Container()
{
	current_idx = -1;
}

Video_Container::~Video_Container()
{
	processing_states.clear();
}

void Video_Container::select_state(int idx)
{
	if (idx == -1)
	{
		return;
	}

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