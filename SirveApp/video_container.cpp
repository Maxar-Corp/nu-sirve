#include "video_container.h"

Video_Container::Video_Container()
{
	current_idx = -1;
}

Video_Container::~Video_Container()
{
}

void Video_Container::select_state(int idx)
{
	if (idx == -1)
	{
		return;
	}

	current_idx = idx;

	emit update_display_video(processing_states[idx].details);
}

void Video_Container::clear_processing_states()
{
	processing_states.clear();
	current_idx = -1;

	emit states_cleared();
}

void Video_Container::add_processing_state(processing_state new_state)
{
	processing_states.push_back(new_state);
	current_idx = processing_states.size() - 1;

	QString state_name = QString::number(current_idx) + ": " + new_state.get_friendly_description();
	emit state_added(state_name, current_idx);
	emit update_display_video(new_state.details);
}

processing_state Video_Container::copy_current_state()
{
	return processing_states[current_idx];
}

void Video_Container::undo()
{
	if (processing_states.size() < 2) {
		return;
	}

	Processing_Method method = processing_states.back().method;

	processing_states.pop_back();

	emit state_removed(method, processing_states.size());

	if (current_idx == processing_states.size()) {
		current_idx -= 1;
		emit update_display_video(processing_states[current_idx].details);
	}
}

std::vector<processing_state> Video_Container::get_processing_states()
{
	return processing_states;
}