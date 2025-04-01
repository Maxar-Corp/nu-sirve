#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <QObject>
#include <vector>

#include "processing_state.h"

/**
 * \brief Holds the video processing states.
 *
 * This class wraps a vector of processingState objects, while automatically updating the containers
 * to reflect their place in the processing state list and emitting signals when the current state is changed.
 *
 * It also provides some functionality of a ContiguousContainer requirement and can largely be used as you'd
 * use an std::vector. See https://en.cppreference.com/w/cpp/named_req/ContiguousContainer
 */
class StateManager : public QObject
{
	Q_OBJECT

public:
	StateManager(QObject* parent = nullptr);

	ProcessingState& GetCurrentState();
	const ProcessingState& GetCurrentState() const;
	int GetCurrentStateIdx() const;

	// ContiguousContainer requirements
	using value_type = ProcessingState;
	using reference = ProcessingState&;
	using const_reference = const ProcessingState&;
	using iterator = std::vector<ProcessingState>::iterator;
	using const_iterator = std::vector<ProcessingState>::const_iterator;
	using difference_type = std::vector<ProcessingState>::difference_type;
	using size_type = std::vector<ProcessingState>::size_type;

	ProcessingState& at(size_t idx);
	const ProcessingState& at(size_t idx) const;

	ProcessingState& operator[](size_t idx);
	const ProcessingState& operator[](size_t idx) const;

	ProcessingState& front();
	const ProcessingState& front() const;

	ProcessingState& back();
	const ProcessingState& back() const;

	size_t size() const noexcept;
	bool empty() const noexcept;

	void push_back(ProcessingState sourceState, ProcessingMethod method = ProcessingMethod::original, bool emit_state_added = true, bool emit_update_display = true);

	void erase(size_t idx, bool emit_update_display = false);
	void erase(const const_iterator& it, bool emit_update_display = false);
	void erase(const const_iterator& begin, const const_iterator& end, bool emit_update_display = false);
	void pop_back(bool emit_state_removed = true, bool emit_update_display = true);
	void clear();

	iterator begin();
	const_iterator begin() const;

	iterator end();
	const_iterator end() const;

signals:
	void statesCleared();
	void stateAdded(QString state_name, QString combobox_state_name, int index);
	void stateRemoved(ProcessingMethod method, int index);
	void updateDisplayVideo();

public slots:
	void SelectState(int idx);
	void PopProcessingState();

private:
	std::vector<ProcessingState> processing_states;
	int current_idx = -1;
};

#endif // STATE_MANAGER_H
