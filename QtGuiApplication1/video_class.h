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


    unsigned int number_of_frames;
	int timer_frequency;
    QLabel  *label;
    QLabel *text;
	Min_Max_Value color_correction;
	HistogramLine_Plot *histogram_plot;

	QString banner_text, boresight_text;
	QColor banner_color, boresight_color;
	QVector<QRgb> colorTable;
	
	void set_frame_data(std::vector<Plotting_Frame_Data> &input_data);
	bool start_recording(double fps);
	void add_new_frame(QImage &img, int format);
	void stop_recording();
	
	bool plot_boresight, display_boresight_txt, display_tgt_pos_txt;
	void save_frame();


public slots:
    void update_display_frame();	
	void update_specific_frame(unsigned int frame_number);
	//void update_color_correction(double lift, double gamma, double gain);
	void update_color_correction(double new_min_value, double new_max_value);
	void update_video_file(std::vector<std::vector<uint16_t>> &video_data, int x_pixels, int y_pixels);
	void receive_video_data(video_details &new_input);
	void update_banner_text(QString input_banner_text, QColor input_banner_color);
	void toggle_osm_tracks();
	void toggle_primary_track_data();
	void toggle_sensor_boresight_data();
	void toggle_relative_histogram();

private:

	int image_x, image_y, number_pixels, max_bit_level;

	std::vector<std::vector<uint16_t>> frame_data;
    unsigned int counter;

	std::vector<Plotting_Frame_Data> display_data;

};


#endif // VIDEO_CLASS_H
