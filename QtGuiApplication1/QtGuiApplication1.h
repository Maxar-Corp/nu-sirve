#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtGuiApplication1.h"

#include "osm_reader.h"
#include "ABIR_Reader.h"
#include "video_class.h"
#include "histogram_plotter.h"
#include "osm_plotter.h"
#include "play_back.h"
#include "color_correction_lgg.h"
#include "engineering_data.h"
#include "plot_engineering_data.h"
#include "video_container.h"
#include "process_file.h"
#include "color_correction_plot.h"
#include "non_uniformity_correction.h"
#include "background_subtraction.h"
#include "deinterlace.h"
#include "logging.h"

#include <qlabel.h>
#include <qgridlayout.h>
#include <qthread.h>
#include <qobject.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qjsonobject.h>
#include <qjsondocument.h>


class QtGuiApplication1 : public QMainWindow
{
	Q_OBJECT

public:

	//Variables
	Process_File file_data;
	
	QWidget *main_widget;
	QGridLayout *video_layout, *engineering_plot_layout, *histogram_layout, *color_plot_layout;
	QAction *menu_add_banner, *menu_add_primary_data, *menu_sensor_boresight, *menu_osm;

	Lift_Gamma_Gain color_correction;
	Playback *playback_controller;
	QMenu *menu;
	
	Video_Container *videos;
	Video *ir_video;
	HistogramLine_Plot *histogram_plot;
	Engineering_Plots *data_plots;
	Engineering_Data *eng_data;
	ColorPlotter color_correction_plot;

	
	QtGuiApplication1(QWidget *parent = Q_NULLPTR);
	~QtGuiApplication1();

	void set_color_correction_slider_labels();

	signals:
		void change_banner(QString banner_text, QColor banner_color);
	

	public slots:
	void color_correction_toggled(int value);

	void load_osm_data();
	void load_abir_data();

	void update_fps();
	void reset_color_correction();
	void allow_epoch();
	void plot_full_data(int index);
	void plot_primary_only();
	void plot_current_frame_marker();
	void save_plot();
	void save_frame();
	void set_frame_number_label(int counter);
	
	void create_non_uniformity_correction();
	void create_background_subtraction_correction();
	void toggle_video_filters();
	void create_deinterlace();
	void change_deinterlace_options(int index);
	deinterlace_type find_deinterlace_type(int index);
	Video_Parameters find_deinterlace_video_type(int index);
	void clear_image_processing();

	video_details get_current_filter_state();
	bool check_filter_selection(video_details filter_state);
	void show_available_filter_options();
	
private:
	Ui::QtGuiApplication1Class ui;
	int max_used_bits;

	void create_menu_actions();
	void edit_banner_text();
	void plot_change(int index);

	int get_integer_from_txt_box(QString input);
	bool check_value_within_range(int input_value, int min_value, int max_value);
	
	
};
