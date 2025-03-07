#include "video_container.h"

inline void AddUnique(std::vector<unsigned int>& vec, unsigned int val);

void VideoContainer::SelectState(int idx)
{
	if (idx == -1) {
		return;
	}

	current_idx = idx;

	emit updateDisplayVideo();
}

void VideoContainer::PopProcessingState()
{
	pop_back();
}

processingState& VideoContainer::at(size_t idx)
{
	return processing_states.at(idx);
}

const processingState& VideoContainer::at(size_t idx) const
{
	return processing_states.at(idx);
}

processingState& VideoContainer::operator[](size_t idx)
{
	return at(idx);
}
const processingState& VideoContainer::operator[](size_t idx) const
{
	return at(idx);
}

processingState& VideoContainer::front()
{
	return processing_states.front();
}

const processingState& VideoContainer::front() const
{
	return processing_states.front();
}

processingState& VideoContainer::back()
{
	return processing_states.back();
}

const processingState& VideoContainer::back() const
{
	return processing_states.back();
}

size_t VideoContainer::size() const noexcept
{
	return processing_states.size();
}

bool VideoContainer::empty() const noexcept
{
	return processing_states.empty();
}

/**
 * Adds a processing state to the container given the parent state and the method.
 *
 * This method also sets the state_ID and state_description of the processing state,
 * emits the stateAdded and the updateDisplayVideo signals,
 *
 * \param sourceState The parent state, except for the original state.
 * \param method The processing method of the added state.
 * \param emit_state_added Whether to emit the stateAdded signal.
 * \param emit_update_display Whether to emit the updateDisplayVideo signal.
 */
void VideoContainer::push_back(processingState sourceState, ProcessingMethod method, bool emit_state_added, bool emit_update_display)
{
	// Set the method and source_state_ID of the new state before it gets overwritten
	sourceState.method = method;
	sourceState.source_state_ID = sourceState.state_ID;

	processing_states.push_back(std::move(sourceState));
	auto &last = processing_states.back();
	last.state_ID = size() - 1;

	if (last.method == ProcessingMethod::original) {
		// Record ourselves as the only state
		last.state_steps = QLocale().toString(last.state_ID);
		last.process_steps.clear();

	} else {
		// Update the descendants and ancestors
		auto& source = processing_states.at(last.source_state_ID);

		AddUnique(source.descendants, last.state_ID);
		AddUnique(last.ancestors, source.state_ID);

		last.process_steps.push_back(" [" + last.GetStepName() + "] ");
		last.state_steps += " -> " + QLocale().toString(last.state_ID);
	}

	last.UpdateMaxValue();
	last.UpdateDescription();

	// If the parent state was the current state, update the current state
	if (current_idx == last.source_state_ID) {
		current_idx = last.state_ID;
	} else {
		emit_update_display = false;
	}

	if (emit_state_added) {
		emit stateAdded(last.state_description, last.GetComboBoxDescription(), current_idx);
	}
	if (emit_update_display) {
		emit updateDisplayVideo();
	}
}

void VideoContainer::pop_back(bool emit_state_removed, bool emit_update_display)
{
	if (size() < 2) {
		return;
	}

	ProcessingMethod method = processing_states.back().method;
	processing_states.pop_back();

	if (emit_state_removed) {
		emit stateRemoved(method, processing_states.size());
	}
	if (current_idx == processing_states.size() && emit_update_display) {
		current_idx -= 1;
		emit updateDisplayVideo();
	}
}

void VideoContainer::erase(size_t idx, bool emit_update_display)
{
	erase(processing_states.begin() + idx, emit_update_display);
}

void VideoContainer::erase(const const_iterator& it, bool emit_update_display)
{
	processing_states.erase(it);
	if (current_idx >= static_cast<int>(processing_states.size()) && emit_update_display) {
		current_idx = processing_states.size() - 1;
		emit updateDisplayVideo();
	}
}

void VideoContainer::erase(const const_iterator& begin, const const_iterator& end, bool emit_update_display)
{
	processing_states.erase(begin, end);
	if (current_idx >= static_cast<int>(processing_states.size())) {
		current_idx = processing_states.size() - 1;
		emit updateDisplayVideo();
	}
}

void VideoContainer::clear()
{
	processing_states.clear();
	current_idx = -1;

	emit statesCleared();
}

std::vector<processingState>::iterator VideoContainer::begin()
{
	return processing_states.begin();
}

std::vector<processingState>::const_iterator VideoContainer::begin() const
{
	return processing_states.begin();
}

std::vector<processingState>::iterator VideoContainer::end()
{
	return processing_states.end();
}

std::vector<processingState>::const_iterator VideoContainer::end() const
{
	return processing_states.end();
}

processingState& VideoContainer::GetCurrentState()
{
	if (current_idx < 0) {
		throw std::out_of_range("Video container is empty");
	}
	if (current_idx >= static_cast<int>(processing_states.size())) {
		throw std::out_of_range("Current state index is out of range");
	}
	return processing_states.at(current_idx);
}

const processingState& VideoContainer::GetCurrentState() const
{
	return const_cast<VideoContainer*>(this)->GetCurrentState();
}

int VideoContainer::GetCurrentStateIdx() const
{
	return current_idx;
}

inline void AddUnique(std::vector<unsigned int>& vec, unsigned int val)
{
	if (std::find(vec.begin(), vec.end(), val) == vec.end()) {
		vec.push_back(val);
	}
}
