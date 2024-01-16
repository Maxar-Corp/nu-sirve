#pragma once

#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>
#include <optional>

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
#include <QCheckBox>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/types_c.h"

#include <armadillo>

#include "color_scheme.h"
#include "abir_reader.h"
#include "color_correction.h"
#include "video_container.h"
#include "Data_Structures.h"
#include "histogram_plotter.h"
#include "enhanced_label.h"
#include "color_map.h"
#include "calibration_data.h"
#include "annotation_info.h"
#include "video_display_zoom.h"
#include "tracks.h"

class VideoDisplay : public QWidget
{
    Q_OBJECT
public:
   
	VideoDisplay(int input_bit_level);
	~VideoDisplay();
	QVBoxLayout *video_display_layout;
	void reclaim_label();

	int counter_record, video_frame_number;
	bool record_frame;
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
	QColor tracker_color;
	bool plot_tracks;

	// defines color map parameters for showing color in video frame
	ColorMap video_colors;
	int index_video_color;
	QVector<QRgb> colorTable;

	void highlight_bad_pixels(bool status);

	void set_starting_frame_number(unsigned int frame_number);

	void update_frame_data(std::vector<Plotting_Frame_Data> input_data);
	void set_frame_data(std::vector<Plotting_Frame_Data> input_data, std::vector<ABIR_Frame>& input_frame_header);

	void initialize_track_data(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input);
	void update_manual_track_data(std::vector<TrackFrame> track_frame_input);
	void add_manual_track_id_to_show_later(int id);
	void hide_manual_track_id(int id);
	void show_manual_track_id(int id);

	void set_calibration_model(CalibrationData input);
	bool start_recording(double fps);
	void add_new_frame(QImage &img, int format);
	void stop_recording();

	void toggle_osm_tracks(bool input);

	void toggle_action_zoom(bool status);
	void toggle_action_calculate_radiance(bool status);
	void enter_track_creation_mode(std::vector<std::optional<TrackDetails>> starting_track_details);
	const std::vector<std::optional<TrackDetails>> & get_created_track_details();
	void exit_track_creation_mode();

	void save_frame();

	void remove_frame();
	void receive_video_data(int x, int y, int num_frames);
	void initialize_toggles();

signals:
	void clear_mouse_buttons();
	void force_new_lift_gain(double lift, double gain);
	void add_new_bad_pixels(std::vector<unsigned int> new_pixels);
	void remove_bad_pixels(std::vector<unsigned int> pixels);
	void advance_frame();
	void finish_create_track();

public slots:
	void update_frame_vector();
	void view_frame(unsigned int frame_number);
	void update_color_correction(double new_min_value, double new_max_value);
	void update_banner_text(QString input_banner_text);
	void update_banner_color(QString input_color);
	void update_tracker_color(QString input_color);
	void update_color_map(QString input_map);

	void toggle_sensor_boresight_data();
	void toggle_frame_time(bool checked);
	
	void handle_click(QPoint origin);
	void clear_pinpoints();

	void handle_new_auto_lift_gain_sigma(double lift_sigma, double gain_sigma);
	void end_auto_lift_gain();

	void handle_image_area_selection(QRect area);
	void unzoom();


private:
	VideoDisplayZoomManager *zoom_manager;
	std::vector<double> original_frame_vector;

	QLabel *lbl_pinpoint;
	QPushButton *btn_pinpoint, *btn_pinpoint_bad_pixel, *btn_pinpoint_good_pixel, *btn_clear_pinpoints;
	QGroupBox *grp_pinpoint;
	std::vector<unsigned int> pinpoint_indeces;

	QLabel *lbl_create_track;
	QPushButton *btn_select_track_centroid, *btn_clear_track_centroid;
	QCheckBox  *chk_auto_advance_frame;
	QGroupBox *grp_create_track;
	std::vector<std::optional<TrackDetails>> track_details;
	int track_details_min_frame, track_details_max_frame;
	
	bool is_zoom_active, is_calculate_active, should_show_bad_pixels;
	bool in_track_creation_mode;
	bool auto_lift_gain;
	double auto_lift_sigma, auto_gain_sigma;
	QLabel *lbl_frame_number, *lbl_video_time_midnight, *lbl_zulu_time;

	QRect calculation_region;
	
	std::vector<std::vector<uint16_t>> frame_data;
    unsigned int counter, starting_frame_number;

	std::vector<unsigned int> bad_pixels;

	QColor banner_color;
	bool display_boresight_txt, display_time;

	CalibrationData model;
	std::vector<Plotting_Frame_Data> display_data;
	std::vector<TrackFrame> osm_track_frames;
	std::vector<TrackFrame> manual_track_frames;
	std::set<int> manual_track_ids_to_show;
	std::vector<ABIR_Frame> frame_headers;

	void setup_labels();
	void setup_create_track_controls();
	void setup_pinpoint_display();

	void calibrate(QRect area);

	QRectF get_rectangle_around_pixel(int x_center, int y_center, int box_size, double box_width, double box_height);

	void pinpoint(unsigned int x, unsigned int y);
	void select_track_centroid(unsigned int x, unsigned int y);

	void add_pinpoints_to_bad_pixel_map();
	void remove_pinpoints_from_bad_pixel_map();
	void handle_btn_pinpoint(bool checked);
	void handle_btn_select_track_centroid(bool checked);
	void handle_btn_clear_track_centroid();
	void reset_create_track_min_and_max_frames();
	void update_create_track_label();
    void update_display_frame();

	QString get_zulu_time_string(double seconds_midnight);
};

#endif // VIDEO_DISPLAY_H