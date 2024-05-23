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
#include "video_container.h"
#include "Data_Structures.h"
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
   
	QString dark_green_button_styleSheet = "color: white; background-color: #1a3533; font-weight: bold;";

	QString olive_green_button_styleSheet = "color: white; background-color: #555121; font-weight: bold;";

	QString dark_blue_button_styleSheet = "color: white; background-color: #0b2139; font-weight: bold;";

	QString dark_orange_button_styleSheet = "color: white; background-color: #743203; font-weight: bold;";

	QString track_button_styleSheet = "color: white; background-color: #002147; font-weight: bold;";

	QString dark_red_stop_styleSheet = "color: white; background-color: #331a1a; font-weight: bold;";
	
	QString orange_styleSheet = "color: black; background-color: #fbb31a; font-weight: bold;";

	QString bright_green_styleSheet = "color: black; background-color: #73ee53; font-weight: bold;";

	VideoDisplay(QVector<QRgb> starting_color_table);
	~VideoDisplay();
	QVBoxLayout *video_display_layout;
	void reclaim_label();

	unsigned int counter;

	int counter_record, video_frame_number;
	bool record_frame;
	cv::VideoWriter video;
	
	std::vector<annotation_info> annotation_list;

	int image_x, image_y, number_pixels;

	QImage frame;
    EnhancedLabel  *label;
	Video_Container container;

	QString banner_text, boresight_text;
	QColor tracker_color, bad_pixel_color;
	bool plot_tracks;

	void highlight_bad_pixels(bool status);
	void highlight_bad_pixels_colors(QString input_color);


	void update_frame_data(std::vector<Plotting_Frame_Data> input_data);

	void initialize_track_data(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input);
	void initialize_frame_data(unsigned int frame_number, std::vector<Plotting_Frame_Data> input_data, std::vector<ABIR_Frame>& input_frame_header);
	void update_manual_track_data(std::vector<TrackFrame> track_frame_input);
	void add_manual_track_id_to_show_later(int id);
	void hide_manual_track_id(int id);
	void delete_manual_track(int id);
	void show_manual_track_id(int id);
	void recolor_manual_track(int id, QColor color);

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
	void receive_video_data(int x, int y);
	void initialize_toggles();

	void view_frame(unsigned int frame_number);

signals:
	void clear_mouse_buttons();
	void add_new_bad_pixels(std::vector<unsigned int> new_pixels);
	void remove_bad_pixels(std::vector<unsigned int> pixels);
	void advance_frame();
	void finish_create_track();

public slots:
	void update_frame_vector(std::vector<double> original, std::vector<uint8_t> converted);
	void update_banner_text(QString input_banner_text);
	void update_banner_color(QString input_color);
	void update_tracker_color(QString input_color);
	void update_color_map(QVector<QRgb> color_table);

	void toggle_sensor_boresight_data();
	void toggle_frame_time(bool checked);
	
	void handle_click(QPoint origin);
	void clear_pinpoints();

	void handle_annotation_changes();

	void handle_image_area_selection(QRect area);
	void unzoom();


private:
	VideoDisplayZoomManager *zoom_manager;
	std::vector<double> original_frame_vector;
	std::vector<uint8_t> display_ready_converted_values;

	QVector<QRgb> colorTable;

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
	QLabel *lbl_frame_number, *lbl_video_time_midnight, *lbl_zulu_time;

	QRect calculation_region;
	
	std::vector<std::vector<uint16_t>> frame_data;
    unsigned int starting_frame_number;

	std::vector<unsigned int> bad_pixels;

	QColor banner_color;
	bool display_boresight_txt, display_time;

	CalibrationData model;
	std::vector<Plotting_Frame_Data> display_data;
	std::vector<TrackFrame> osm_track_frames;
	std::vector<TrackFrame> manual_track_frames;
	std::set<int> manual_track_ids_to_show;
	std::map<int, QColor> manual_track_colors;
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