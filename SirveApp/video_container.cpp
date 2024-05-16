#include "video_container.h"

VideoContainer::VideoContainer()
{
	current_idx = -1;
}

VideoContainer::~VideoContainer()
{
}

void VideoContainer::select_state(int idx)
{
	if (idx == -1)
	{
		return;
	}

	current_idx = idx;

	emit update_display_video();
}

void VideoContainer::clear_processing_states()
{
	processing_states.clear();
	current_idx = -1;

	emit states_cleared();
}

void VideoContainer::add_processing_state(processingState new_state)
{
	processing_states.push_back(new_state);
	current_idx = processing_states.size() - 1;

	QString state_name = QString::number(current_idx) + ": " + new_state.get_friendly_description();
	emit state_added(state_name, current_idx);
	emit update_display_video();
}

processingState VideoContainer::copy_current_state()
{
	return processing_states[current_idx];
}

void VideoContainer::undo()
{
	if (processing_states.size() < 2) {
		return;
	}

	ProcessingMethod method = processing_states.back().method;

	processing_states.pop_back();

	emit state_removed(method, processing_states.size());

	if (current_idx == processing_states.size()) {
		current_idx -= 1;
		emit update_display_video();
	}
}

std::vector<processingState> VideoContainer::get_processing_states()
{
	return processing_states;
}