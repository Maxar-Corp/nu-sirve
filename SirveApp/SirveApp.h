#pragma once

#include <QtWidgets/QMainWindow>

#include "osm_reader.h"
#include "ABIR_Reader.h"
#include "video_display.h"
#include "histogram_plotter.h"
#include "playback.h"
#include "engineering_data.h"
#include "plot_engineering_data.h"
#include "video_container.h"
#include "video_details.h"
#include "process_file.h"
#include "non_uniformity_correction.h"
#include "noise_suppression.h"
#include "deinterlace.h"
#include "deinterlace_type.h"
#include "annotation_info.h"
#include "annotation_list_dialog.h"
#include "custom_input_dialog.h"
#include "calibration_data.h"
#include "non_uniformity_correction_external_file.h"
#include "config.h"
#include "support/jtime.h"
#include "support/qthelpers.h"
#include "workspace.h"
#include "Data_Structures.h"
#include "popout_dialog.h"
#include "bad_pixels.h"
#include "tracks.h"
#include "track_management_widget.h"
#include "data_export.h"
#include "color_correction.h"
#include "windows.h"

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
#include <QStringList>

class SirveApp : public QMainWindow
{
	Q_OBJECT

public:

	//Variables
	Process_File file_processor;
	OSMReader osm_reader;
	std::vector<Frame> osm_frames;
	AbpFileMetadata abp_file_metadata;
	Workspace workspace;

	QWidget *main_widget;
	QGridLayout *engineering_plot_layout;

	QAction *menu_add_banner, *menu_add_primary_data, *menu_sensor_boresight, *menu_osm, *menu_change_color_tracker, *menu_change_color_banner, *menu_change_color_map, *menu_annotate;
	QAction *menu_plot_all_data, *menu_plot_primary, *menu_plot_frame_marker, *menu_plot_edit_banner;

	QThread thread_video, thread_timer;

	Playback *playback_controller;
	QMenu *menu, *plot_menu;

	/* --------------------------------------------------------------------------------------------
	Qt Elements for user interface
	----------------------------------------------------------------------------------------------- */

	QTabWidget* tab_menu, * tab_plots;
	QDateTimeEdit* dt_epoch;
	QLabel * lbl_file_name, *lbl_lift_value, *lbl_gain_value, *lbl_max_frames, *lbl_fps, *lbl_current_epoch, *lbl_adaptive_noise_suppression, *label_adaptive_noise_suppression_status, *lbl_fixed_suppression, *lbl_bad_pixel_count, * lbl_create_track_message;
	QLineEdit* txt_lift_sigma, * txt_gain_sigma;
	QSlider* slider_lift, * slider_gain, * slider_video;

	QLineEdit* txt_start_frame, * txt_end_frame;
	QPushButton* btn_get_frames, * btn_load_osm, * btn_copy_directory, * btn_apply_epoch, * btn_reset_color_correction, * btn_bgs, * btn_create_nuc,
		* btn_calibration_dialog, * btn_deinterlace, * btn_play, * btn_slow_back, * btn_fast_forward, * btn_prev_frame, * btn_next_frame, * btn_video_menu,
		* btn_pause, * btn_reverse, * btn_frame_save, * btn_frame_record, * btn_save_plot, * btn_plot_menu, * btn_zoom, *btn_calculate_radiance,
		* btn_workspace_load, * btn_workspace_save, * btn_undo_step, * btn_popout_video, * btn_popout_histogram, * btn_popout_engineering, * btn_bad_pixel_identification,
		* btn_import_tracks, * btn_create_track, * btn_finish_create_track;

	QCheckBox * chk_auto_lift_gain, * chk_relative_histogram, * chk_plot_primary_data, * chk_plot_show_line, * chk_plot_full_data;
	QGroupBox * grpbox_auto_lift_gain;
	QComboBox* cmb_deinterlace_options, * cmb_plot_yaxis, * cmb_plot_xaxis, *cmb_color_maps, * cmb_workspace_name, * cmb_processing_states;
	QFrame* frame_video_player, *frame_histogram_rel, *frame_histogram_abs;
	QFrame* frame_plots;
	QRadioButton* rad_decimal, * rad_linear, * rad_scientific, * rad_log;
	QButtonGroup *data_plot_yformat, *data_plot_yloglinear;

	QCheckBox* chk_show_tracks, *chk_sensor_track_data, *chk_show_time, *chk_highlight_bad_pixels;
	QComboBox* cmb_text_color, *cmb_tracker_color, *cmb_primary_tracker_color;
	QPushButton* btn_change_banner_text, * btn_add_annotations;

	/* --------------------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------- */

	VideoDisplay *video_display;
	Engineering_Plots *data_plots;
	Engineering_Data *eng_data;
	TrackInformation *track_info;
	TrackManagementWidget *tm_widget;
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

	void toggle_video_playback_options(bool input);
	bool verify_frame_selection(int min_frame, int max_frame);
	void update_epoch_string(QString new_epoch_string);
	void display_original_epoch(QString new_epoch_string);
	QString create_epoch_string(std::vector<double> new_epoch);

	signals:
		void change_banner(QString banner_text);
		void change_banner_color(QString color);
		void change_tracker_color(QString color);

	public slots:

		void histogram_clicked(double x0, double x1);
		void handle_chk_auto_lift_gain(int state);
		void set_lift_and_gain(double lift, double gain);
		void lift_slider_toggled();
		void gain_slider_toggled();

		void save_workspace();
		void load_workspace();
		void import_tracks();
		void handle_removal_of_track(int track_id);
		void handle_manual_track_recoloring(int track_id, QColor color);

		void ui_choose_abp_file();
		bool validate_abp_files(QString path_to_image_file);
		void ui_load_abir_data();
		void ui_execute_noise_suppression();
		void ui_execute_deinterlace();
		void ui_execute_non_uniformity_correction_selection_option();

		void start_stop_video_record();
		void toggle_zoom_on_video();
		void toggle_calculation_on_video();
		void clear_zoom_and_calculation_buttons();

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
		void copy_osm_directory();
		void toggle_relative_histogram(bool input);
		void apply_epoch_time();

		void fixed_noise_suppression_from_external_file();

		void toggle_osm_tracks();

		void receive_new_bad_pixels(std::vector<unsigned int> new_pixels);
		void receive_new_good_pixels(std::vector<unsigned int> pixels);

		void handle_new_processing_state(QString state_name, int index);
		void handle_processing_state_removal(Processing_Method method, int index);
		void SirveApp::handle_cleared_processing_states();

		void popout_video_closed();
		void popout_histogram_closed();
		void popout_engineering_closed();

		void handle_frame_change();

private:
	ColorMap video_colors;
	ColorMapDisplay *color_map_display;

	QVBoxLayout *histogram_abs_layout, *vlayout_tab_histogram;
	QClipboard *clipboard;

	PopoutDialog *popout_video;
	PopoutDialog *popout_histogram;
	PopoutDialog *popout_engineering;

	HistogramLine_Plot *histogram_plot;

	ConfigValues config_values;

	int currently_editing_or_creating_track_id;

	void create_menu_actions();
	void edit_color_map();
	void edit_banner_text();
	void edit_plot_text();
	void export_plot_data();
	void edit_banner_color();
	void edit_tracker_color();
	void plot_change();
	void annotate_video();

	void handle_popout_video_btn(bool checked);
	void open_popout_video_display();

	void handle_popout_histogram_btn(bool checked);
	void open_popout_histogram_plot();

	void handle_popout_engineering_btn(bool checked);
	void open_popout_engineering_plot();

	void resize_ui();

	QMenu *menu_file, *menu_settings;
	QAction *action_close, *action_set_timing_offset;

	int get_integer_from_txt_box(QString input);
	int get_color_index(QVector<QString> colors, QColor input_color);
	CalibrationData calibration_model;

	void load_osm_data();
	void load_abir_data(int start_frame, int end_frame);

	void ui_replace_bad_pixels();
	void replace_bad_pixels(std::vector<unsigned int> & pixels_to_replace);

	void create_fixed_noise_correction(int start_frame, int num_frames, QString hide_shadow_choice);
	void create_adaptive_noise_correction(int relative_start_frame, int num_frames, QString hide_shadow_choice);
	void create_deinterlace(deinterlace_type deinterlace_method_type);
	void fixed_noise_suppression(QString image_path, QString file_path, unsigned int min_frame, unsigned int max_frame);

	void enable_engineering_plot_options();

	void handle_btn_create_track();
	void prepare_for_track_creation(int track_id);
	void exit_track_creation_mode();
	void handle_btn_finish_create_track();

	void handle_frame_number_change(unsigned int new_frame_number);

	void update_global_frame_vector();
};
