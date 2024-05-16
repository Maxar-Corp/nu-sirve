#pragma once

#include <vector>
#include <qobject.h>

#include <QString>

#include "processing_state.h"
#include "deinterlace_type.h"

class VideoContainer : public QObject
{
	Q_OBJECT
	
	public:
		std::vector<processingState> processing_states;
		int current_idx;

        VideoContainer();
        ~VideoContainer();

		void clear_processing_states();
		void add_processing_state(processingState new_details);

		processingState copy_current_state();

		void undo();

		std::vector<processingState> get_processing_states();

	signals:
		void update_display_video();

		void states_cleared();
		void state_added(QString state_name, int index);
		void state_removed(ProcessingMethod method, int index);

	public slots:
		void select_state(int idx);

	private:
};

