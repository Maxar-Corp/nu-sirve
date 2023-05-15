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

		Video_Container();
		~Video_Container();

		void Video_Container::clear_processing_states();
		void Video_Container::add_processing_state(video_details new_details);

		void reset(video_details input);
		void display_original_data();
		bool display_data(video_details input);
		int find_data_index(video_details input);

	signals:
		void update_display_video(video_details &update);

		void processing_states_cleared();
		void processing_state_added(QString name);

	private:

};

