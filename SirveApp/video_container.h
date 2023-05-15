#pragma once

#include <iostream>
#include <vector>
#include <qobject.h>

#include "logging.h"
#include "video_details.h"

class Video_Container : public QObject
{
	Q_OBJECT
	
	public:
		std::vector<video_details> something;

		Video_Container();
		~Video_Container();

		void reset(video_details input);
		void display_original_data();
		bool display_data(video_details input);
		int find_data_index(video_details input);

	signals:
		void update_display_video(video_details &update);


	private:

};

