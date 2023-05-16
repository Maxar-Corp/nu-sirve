#pragma once

#include <iostream>
#include <vector>
#include <qobject.h>

#include <QString>

#include "logging.h"
#include "video_details.h"

class Video_Container : public QObject
{
	Q_OBJECT
	
	public:
		std::vector<video_details> something;
		std::vector<video_details> processing_states;
		int current_idx;

		Video_Container();
		~Video_Container();

		void clear_processing_states();
		void add_processing_state(video_details new_details);

		void reset(video_details input);
		void display_original_data();
		bool display_data(video_details input);
		int find_data_index(video_details input);

		video_details copy_current_state();

		void undo();

	signals:
		void update_display_video(video_details &update);

		void states_cleared();
		void state_added(QString state_name, int index);
		void state_removed(int index);

	public slots:
		void select_state(int idx);

	private:
};

