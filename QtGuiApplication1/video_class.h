#pragma once

#ifndef VIDEO_CLASS_H
#define VIDEO_CLASS_H

#include <iostream>
#include <string>
#include <math.h>

#include <QString>
#include <QTimer>
#include <QLabel>
#include <vector>
#include <math.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qfiledialog.h>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/types_c.h"

#include <armadillo>

#include "color_scheme.h"
#include "abir_reader.h"
#include "color_correction.h"
#include "video_container.h"
#include "Data_Structures.h"
#include "histogram_plotter.h"
#include "clickable_label.h"
#include "color_map.h"

class Video : public QWidget
{
    Q_OBJECT
public:
   
	Video(int x_pixles, int y_pixels, int input_bit_level);
	//Video(std::vector<std::vector<uint16_t>> &video_data, int x_pixels, int y_pixels);
	~Video();

	int counter_record, video_frame_number;
	bool record_frame, show_relative_histogram;
	cv::VideoWriter video;
	
	std::vector<annotation_info> annotation_list;

	int image_x, image_y, number_pixels, max_bit_level;

    unsigned int number_of_frames;
	int timer_frequency;
	QImage frame;
    EnhancedLabel  *label;
	Min_Max_Value color_correction;
	HistogramLine_Plot *histogram_plot;

	QString banner_text, boresight_text;
	QColor banner_color, tracker_color;

	// defines color map parameters for showing color in video frame
	ColorMap video_colors;
	int index_video_color;
	QVector<QRgb> colorTable;
	
	void set_frame_data(std::vector<Plotting_Frame_Data> &input_data);
	bool start_recording(double fps);
	void add_new_frame(QImage &img, int format);
	void stop_recording();

	void toggle_osm_tracks(bool input);
	void toggle_primary_track_data(bool input);
	void toggle_sensor_boresight_data(bool input);
	
	bool plot_tracks, display_boresight_txt, display_tgt_pos_txt;
	void save_frame();
	void clear_all_zoom_levels(int x_pixels, int y_pixels);

	void remove_frame();


public slots:
    void update_display_frame();	
	void update_specific_frame(unsigned int frame_number);
	//void update_color_correction(double lift, double gamma, double gain);
	void update_color_correction(double new_min_value, double new_max_value);
	void update_video_file(std::vector<std::vector<uint16_t>> &video_data, int x_pixels, int y_pixels);
	void receive_video_data(video_details &new_input);
	void update_banner_text(QString input_banner_text);
	void update_banner_color(QString input_color);
	void update_tracker_color(QString input_color);

	void toggle_relative_histogram();

	void zoom_image(QRect info);
	void unzoom(QPoint origin);

private:
		
	std::vector<QRect> zoom_list;
	
	std::vector<std::vector<uint16_t>> frame_data;
    unsigned int counter;

	std::vector<Plotting_Frame_Data> display_data;
	
};

#endif // VIDEO_CLASS_H
