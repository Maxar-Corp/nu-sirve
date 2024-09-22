#include "video_container.h"

VideoContainer::VideoContainer()
{
	current_idx = -1;
}

VideoContainer::~VideoContainer()
{
}

void VideoContainer::SelectState(int idx)
{
	if (idx == -1)
	{
		return;
	}

	current_idx = idx;

	emit updateDisplayVideo();
}

void VideoContainer::ClearProcessingStates()
{
	processing_states.clear();
	current_idx = -1;

	emit statesCleared();
}

void VideoContainer::AddProcessingState(processingState new_state)
{
	if (current_idx<0){
		current_idx = 0;
	}
    processing_states.push_back(new_state); // <--- this takes time

	current_idx = processing_states.size() - 1;
	QString state_name = "State " + QString::number(current_idx) + ": " + new_state.get_friendly_description();
	QString combobox_state_name = QString::number(current_idx) + ": " + new_state.get_combobox_description();
	processing_states[current_idx].state_description = state_name;

	emit stateAdded(state_name, combobox_state_name, current_idx);
	emit updateDisplayVideo();
}

processingState VideoContainer::CopyCurrentState()
{
	return processing_states[current_idx];
}

processingState VideoContainer::CopyCurrentStateIdx(int idx)
{
	return processing_states[idx];
}

void VideoContainer::PopProcessingState()
{
	if (processing_states.size() < 2) {
		return;
	}

	ProcessingMethod method = processing_states.back().method;
	processing_states[current_idx].details.frames_16bit.clear();
	processing_states.erase(processing_states.end()-1);
	// processing_states.pop_back();

	emit stateRemoved(method, processing_states.size());

	if (current_idx == processing_states.size()) {
		current_idx -= 1;
		emit updateDisplayVideo();
	}
}

std::vector<processingState> VideoContainer::get_processing_states()
{
	return processing_states;
}
