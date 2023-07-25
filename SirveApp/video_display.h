#pragma once

#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>

#include <QString>
#include <QTimer>
#include <QLabel>
#include <QColor>
#include <vector>
#include <math.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qfiledialog.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/types_c.h"

#include <armadillo>

#include "color_scheme.h"
#include "abir_reader.h"
#include "color_correction.h"
#include "video_container.h"
#include "video_details.h"
#include "Data_Structures.h"
#include "histogram_plotter.h"
#include "enhanced_label.h"
#include "color_map.h"
#include "calibration_data.h"
#include "annotation_info.h"

struct absolute_zoom_info {
	double x, y, width, height;
};

class VideoDisplay : public QWidget
{
    Q_OBJECT
public:
   
	VideoDisplay(int x_pixels, int y_pixels, int input_bit_level);
	~VideoDisplay();
	QVBoxLayout *video_display_layout;
	void reclaim_label();

	int counter_record, video_frame_number;
	bool record_frame, show_relative_histogram, auto_lift_gain;
	cv::VideoWriter video;
	
	std::vector<annotation_info> annotation_list;

	int image_x, image_y, number_pixels, max_bit_level;

    size_t number_of_frames;
	int timer_frequency;
	QImage frame;
    EnhancedLabel  *label;
	double lift, gain;
	HistogramLine_Plot *histogram_plot;
	Video_Container container;

	QString banner_text, boresight_text;
	QColor banner_color, tracker_primary_color, tracker_color;

	// defines color map parameters for showing color in video frame
	ColorMap video_colors;
	int index_video_color;
	QVector<QRgb> colorTable;

	void highlight_bad_pixels(bool status);

	void set_starting_frame_number(unsigned int frame_number);

	void update_frame_data(std::vector<Plotting_Frame_Data> input_data);
	void set_frame_data(std::vector<Plotting_Frame_Data> input_data, std::vector<ABIR_Frame>& input_frame_header);
	void set_calibration_model(CalibrationData input);
	bool start_recording(double fps);
	void add_new_frame(QImage &img, int format);
	void stop_recording();

	void toggle_osm_tracks(bool input);
	void toggle_primary_track_data(bool input);
	void toggle_sensor_boresight_data(bool input);

	void toggle_action_zoom(bool status);
	void toggle_action_calculate_radiance(bool status);
	
	bool plot_tracks, display_boresight_txt, display_tgt_pos_txt, display_time;
	void save_frame();
	void clear_all_zoom_levels(int x_pixels, int y_pixels);

	void remove_frame();

signals:
	void clear_mouse_buttons();
	void force_new_lift_gain(double lift, double gain);

public slots:
    void update_display_frame();	
	void update_specific_frame(unsigned int frame_number);
	//void update_color_correction(double lift, double gamma, double gain);
	void update_color_correction(double new_min_value, double new_max_value);
	void receive_video_data(video_details &new_input);
	void update_banner_text(QString input_banner_text);
	void update_banner_color(QString input_color);
	void update_tracker_color(QString input_color);
	void update_tracker_primary_color(QString input_color);
	void update_color_map(QString input_map);

	void toggle_relative_histogram();
	
	void pinpoint(QPoint origin);
	void clear_pinpoints();

	void zoom_image(QRect info);
	void unzoom();


private:
	QLabel *lbl_pinpoint;
	QPushButton *btn_pinpoint, *btn_clear_pinpoints;
	QGroupBox *grp_pinpoint;
	QHBoxLayout *pinpoint_layout;
	//int pinpoint_x, pinpoint_y;
	std::vector<absolute_zoom_info> absolute_zoom_list;
	std::vector<unsigned int> pinpoint_indeces;
	
	bool is_zoom_active, is_calculate_active, is_pinpoint_active, should_show_bad_pixels;
	std::vector<QRect> zoom_list;
	int index_current_video;
	QLabel *lbl_frame_number, *lbl_video_time_midnight, *lbl_zulu_time;

	QRect calculation_region;
	
	std::vector<std::vector<uint16_t>> frame_data;
    unsigned int counter, starting_frame_number;

	std::vector<unsigned int> bad_pixels;

	CalibrationData model;
	std::vector<Plotting_Frame_Data> display_data;
	std::vector<ABIR_Frame>frame_headers;
	std::vector<int> get_position_within_zoom(int x0, int y0);
	void setup_labels();
	void setup_pinpoint_display();

	void handle_btn_pinpoint(bool checked);

	QString get_zulu_time_string(double seconds_midnight);
};

#endif // VIDEO_DISPLAY_H