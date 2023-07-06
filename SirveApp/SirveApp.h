#pragma once

#include <QtWidgets/QMainWindow>

#include "osm_reader.h"
#include "ABIR_Reader.h"
#include "video_display.h"
#include "histogram_plotter.h"
#include "osm_plotter.h"
#include "playback.h"
#include "color_correction.h"
#include "engineering_data.h"
#include "plot_engineering_data.h"
#include "video_container.h"
#include "video_details.h"
#include "process_file.h"
#include "color_correction_plot.h"
#include "non_uniformity_correction.h"
#include "background_subtraction.h"
#include "deinterlace.h"
#include "deinterlace_type.h"
#include "logging.h"
#include "annotation_info.h"
#include "annotation_list_dialog.h"
#include "custom_input_dialog.h"
#include "fixed_aspect_ratio_frame.h"
#include "calibration_data.h"
#include "non_uniformity_correction_external_file.h"
#include "application_data.h"
#include "config.h"
#include "support/jtime.h"
#include "support/qthelpers.h"
#include "workspace.h"
#include "Data_Structures.h"
#include "popout_dialog.h"

#include <qstackedlayout.h>
#include <qlabel.h>
#include <qgridlayout.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <qthread.h>
#include <qobject.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qdialogbuttonbox.h>
#include <qprogressdialog.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qclipboard.h>
#include <QThread>
#include <QButtonGroup.h>
#include <QRadioButton.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qgroupbox>
#include <qmenu.h>
#include <qmenubar.h>


class SirveApp : public QMainWindow
{
	Q_OBJECT

public:

	//Variables
	Process_File file_processor;
	OSMReader osm_reader;
	std::vector<Frame> osm_frames;
	AbpFileMetadata abp_file_metadata;
	ConfigValues config_values;
	Workspace workspace;
	
	QWidget *main_widget;
	QGridLayout *video_layout, *engineering_plot_layout;
	
	QAction *menu_add_banner, *menu_add_primary_data, *menu_sensor_boresight, *menu_osm, *menu_change_color_tracker, *menu_change_color_banner, *menu_change_color_map, *menu_annotate;
	QAction *menu_plot_all_data, *menu_plot_primary, *menu_plot_frame_marker, *menu_plot_edit_banner;

	QThread thread_video, thread_timer;

	Min_Max_Value color_correction;
	Playback *playback_controller;
	QMenu *menu, *plot_menu;

	/* --------------------------------------------------------------------------------------------
	Qt Elements for user interface
	----------------------------------------------------------------------------------------------- */

	QTabWidget* tab_menu, * tab_plots;
	QDateTimeEdit* dt_epoch;
	QLabel* lbl_file_load, * lbl_file_name, *lbl_lift_value, *lbl_gain_value, *lbl_max_frames, *lbl_fps, *lbl_video_frame, *lbl_video_time_midnight, * lbl_zulu_time, *lbl_current_epoch, *lbl_adaptive_background_suppression, *lbl_fixed_suppression;
	QSlider* slider_lift, * slider_gain, * slider_video;
	
	QLineEdit* txt_start_frame, * txt_end_frame;
	QPushButton* btn_get_frames, * btn_load_osm, * btn_copy_directory, * btn_apply_epoch, * btn_reset_color_correction, * btn_bgs, * btn_create_nuc,
		* btn_calibration_dialog, * btn_deinterlace, * btn_play, * btn_slow_back, * btn_fast_forward, * btn_prev_frame, * btn_next_frame, * btn_video_menu,
		* btn_pause, * btn_reverse, * btn_frame_save, * btn_frame_record, * btn_save_plot, * btn_plot_menu, * btn_zoom, *btn_calculate_radiance,
		* btn_workspace_load, * btn_workspace_save, * btn_undo_step, * btn_popout_video, * btn_popout_histogram, * btn_bad_pixel_identification;
	
	QCheckBox * chk_relative_histogram, * chk_plot_primary_data, * chk_plot_show_line, * chk_plot_full_data;
	QComboBox* cmb_deinterlace_options, * cmb_plot_yaxis, * cmb_plot_xaxis, *cmb_color_maps, * cmb_workspace_name, * cmb_processing_states;
	QFrame* frame_video_player, * frame_video, *frame_histogram, *frame_histogram_abs, *frame_histogram_abs_full;
	FixedAspectRatioFrame* frame_plots;
	QRadioButton* rad_decimal, * rad_linear, * rad_scientific, * rad_log;
	QStackedLayout* stacked_layout_histograms;
	QButtonGroup *data_plot_yformat, *data_plot_yloglinear;

	QCheckBox* chk_show_tracks, *chk_primary_track_data, *chk_sensor_track_data, *chk_show_time, *chk_smooth_bad_pixels;
	QComboBox* cmb_text_color, *cmb_tracker_color, *cmb_primary_tracker_color;
	QPushButton* btn_change_banner_text, * btn_add_annotations;

	/* --------------------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------- */


	VideoDisplay *video_display;
	Engineering_Plots *data_plots;
	Engineering_Data *eng_data;
	ColorPlotter color_correction_plot;
	bool record_video;
	
	SirveApp(QWidget *parent = Q_NULLPTR);
	~SirveApp();

	void setup_ui();
	QWidget* setup_file_import_tab();
	QWidget* setup_color_correction_tab();
	QWidget* setup_filter_tab();
	QWidget* setup_workspace_tab();
	void setup_video_frame();
	void setup_plot_frame();
	void setup_connections();

	void set_color_correction_slider_labels();
	void toggle_video_playback_options(bool input);
	bool verify_frame_selection(int min_frame, int max_frame);
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
	
		void histogram_clicked(double x0, double x1);
		void lift_slider_toggled(int value);
		void gain_slider_toggled(int value);
		void color_correction_toggled(double lift_value, double gain_value);

		void save_workspace();
		void load_workspace();

		void ui_choose_abp_file();
		bool validate_abp_files(QString path_to_image_file);
		void ui_load_abir_data();
		void ui_execute_background_subtraction();
		void ui_execute_deinterlace();
		void ui_execute_non_uniformity_correction_selection_option();

		void start_stop_video_record();
		void toggle_zoom_on_video();
		void toggle_calculation_on_video();

		void update_fps();
		void reset_color_correction();
	
		void toggle_plot_full_data();
		void toggle_plot_primary_only();
		void toggle_plot_current_frame_marker();

		void auto_change_plot_display(int index);	

		void show_calibration_dialog();
		
		void set_data_timing_offset();
		void close_window();

		void save_plot();
		void save_frame();
		void set_frame_number_label(unsigned int current_frame_number);
		void set_zulu_label();
		void copy_osm_directory();
		void update_enhanced_range(bool input);
		void toggle_relative_histogram(bool input);
		void apply_epoch_time();

		void create_non_uniformity_correction_from_external_file();

		void toggle_osm_tracks();
		void toggle_primary_track_data();
		void toggle_sensor_track_data();
		void toggle_frame_time();
		void handle_chk_smooth_bad_pixels(bool checked);

		void handle_new_processing_state(QString state_name, int index);
		
		void popout_video_closed();
		void popout_histogram_closed();

private:
	QVBoxLayout *histogram_abs_layout_full;
	QClipboard *clipboard;

	PopoutDialog *popout_video;
	PopoutDialog *popout_histogram;

	void create_menu_actions();
	void edit_color_map();
	void edit_banner_text();
	void edit_plot_text();
	void export_plot_data();
	void edit_banner_color();
	void edit_tracker_color();
	void edit_primary_tracker_color();
	void plot_change();
	void annotate_video();

	void handle_popout_video_btn(bool checked);
	void open_popout_video_display();
	
	void handle_popout_histogram_btn(bool checked);
	void open_popout_histogram_plot();

	void resize_ui();

	QMenu *menu_file, *menu_settings;
	QAction *action_close, *action_set_timing_offset;

	int get_integer_from_txt_box(QString input);
	int get_color_index(QVector<QString> colors, QColor input_color);
	CalibrationData calibration_model;
	
	void load_osm_data();
	void load_abir_data(int start_frame, int end_frame);

	void create_background_subtraction_correction(int relative_start_frame, int num_frames);
	void create_deinterlace(deinterlace_type deinterlace_method_type);
	void create_non_uniformity_correction(QString file_path, unsigned int min_frame, unsigned int max_frame);
	void identify_bad_pixels();

	void enable_engineering_plot_options();
};
