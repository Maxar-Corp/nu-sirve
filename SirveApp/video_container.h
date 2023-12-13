#pragma once

#include <vector>
#include <qobject.h>

#include <QString>

#include "logging.h"
#include "video_details.h"
#include "processing_state.h"
#include "deinterlace_type.h"

class Video_Container : public QObject
{
	Q_OBJECT
	
	public:
		std::vector<processing_state> processing_states;
		int current_idx;

		Video_Container();
		~Video_Container();

		void clear_processing_states();
		void add_processing_state(processing_state new_details);

		processing_state copy_current_state();

		void undo();

		std::vector<processing_state> get_processing_states();

	signals:
		void update_display_video(video_details &update);

		void states_cleared();
		void state_added(QString state_name, int index);
		void state_removed(Processing_Method method, int index);

	public slots:
		void select_state(int idx);

	private:
};

