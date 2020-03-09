#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtGuiApplication1.h"

#include "osm_reader.h"
#include "ABIR_Reader.h"
#include "video_class.h"
#include "histogram_plotter.h"
#include "osm_plotter.h"
#include "play_back.h"
#include "color_correction.h"
#include "engineering_data.h"
#include "plot_engineering_data.h"
#include "video_container.h"
#include "process_file.h"
#include "color_correction_plot.h"
#include "non_uniformity_correction.h"
#include "background_subtraction.h"
#include "deinterlace.h"
#include "logging.h"
#include "jtime.h"
#include "annotations.h"

#include <qlabel.h>
#include <qgridlayout.h>
#include <qthread.h>
#include <qobject.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qdialogbuttonbox.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qclipboard.h>
#include <QThread>
#include <QButtonGroup.h>
#include <QRadioButton.h>
#include <qdatetime.h>
#include <qmenu.h>


class QtGuiApplication1 : public QMainWindow
{
	Q_OBJECT

public:

	//Variables
	Process_File file_data;
	
	QWidget *main_widget;
	QGridLayout *video_layout, *engineering_plot_layout, *histogram_layout, *histogram_abs_layout, *color_plot_layout;
	
	QAction *menu_add_banner, *menu_add_primary_data, *menu_sensor_boresight, *menu_osm, *menu_change_color_tracker, *menu_change_color_banner, *menu_annotate;
	QAction *menu_plot_all_data, *menu_plot_primary, *menu_plot_frame_marker, *menu_plot_edit_banner;

	QButtonGroup data_plot_yformat, data_plot_yloglinear;

	QThread thread_video, thread_timer;

	Min_Max_Value color_correction;
	Playback *playback_controller;
	QMenu *menu, *plot_menu;
	
	Video_Container *videos;
	Video *ir_video;
	//HistogramLine_Plot *histogram_plot;
	Engineering_Plots *data_plots;
	Engineering_Data *eng_data;
	ColorPlotter color_correction_plot;
	bool record_video;

	
	QtGuiApplication1(QWidget *parent = Q_NULLPTR);
	~QtGuiApplication1();

	void set_color_correction_slider_labels();
	void toggle_video_playback_options(bool input);
	void enable_engineering_plot_options(bool input);
	bool check_min_max_frame_input(int min_frame, int max_frame);
	void update_epoch_string(QString new_epoch_string);
	void display_original_epoch(QString new_epoch_string);
	QString create_epoch_string(std::vector<double> new_epoch);
	void clear_frame_label();

	signals:
		void change_banner(QString banner_text);
		void change_banner_color(QString color);
		void change_tracker_color(QString color);
		void enhanced_dynamic_range(bool enhance_range);
	

	public slots:
	
	void lift_slider_toggled(int value);
	void gain_slider_toggled(int value);
	void color_correction_toggled(double lift_value, double gain_value);

	void load_osm_data();
	void load_abir_data();

	void start_stop_video_record();

	void update_fps();
	void reset_color_correction();
	
	void plot_full_data();
	void plot_primary_only();
	void plot_current_frame_marker();

	void auto_change_plot_display(int index);
	


	void save_plot();
	void save_frame();
	void set_frame_number_label(int counter);
	void copy_osm_directory();
	void update_enhanced_range(bool input);
	void toggle_relative_histogram(bool input);
	void apply_epoch_time();
	
	void create_non_uniformity_correction();
	void create_background_subtraction_correction();
	void toggle_video_filters();
	void create_deinterlace();

	void toggle_osm_tracks();
	void toggle_primary_track_data();
	void toggle_sensor_track_data();

	void yaxis_log_toggled(bool input);
	void yaxis_linear_toggled(bool input);
	void yaxis_decimal_toggled(bool input);
	void yaxis_scientific_toggled(bool input);


	deinterlace_type find_deinterlace_type(int index);
	Video_Parameters find_deinterlace_video_type(int index);
	void clear_image_processing();

	video_details get_current_filter_state();
	bool check_filter_selection(video_details filter_state);
	void show_available_filter_options();
	
private:
	Ui::QtGuiApplication1Class ui;
	QClipboard *clipboard;
	int max_used_bits;

	void create_menu_actions();
	void edit_banner_text();
	void edit_plot_text();
	void edit_banner_color();
	void edit_tracker_color();
	void plot_change(int index);
	void annotate_video();
	

	int get_integer_from_txt_box(QString input);
	bool check_value_within_range(int input_value, int min_value, int max_value);
	int get_color_index(QList<QString> colors, QColor input_color);
	
	
};
