#pragma once
#pragma once
#ifndef EXTERNAL_NUC_H
#define EXTERNAL_NUC_H

#include "logging.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qlistwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmessagebox.h>

#include <qgridlayout.h>

#include "Data_Structures.h"
#include "video_class.h"
#include "engineering_data.h"
#include "plot_engineering_data.h"
#include "fixed_aspect_ratio_frame.h"
#include "process_file.h"
#include "support/qthelpers.h"


class ExternalNUCInformationWidget : public QDialog
{
	Q_OBJECT

public:
	ExternalNUCInformationWidget();
	~ExternalNUCInformationWidget();
	Process_File file_data;
	AbpFileMetadata abp_metadata;
	int start_frame, stop_frame;

public slots:

	void get_osm_file();

private:

	QString file_path, instructions;
	
	Engineering_Data *engineering_data;
	Engineering_Plots* plot_data;	

	QPushButton* btn_load_file, *btn_load_frames, *btn_close;
	QLabel * lbl_data, *lbl_instructions;
	FixedAspectRatioFrame* frame_plot;

	QGridLayout* mainLayout;
	QVBoxLayout* frame_layout;

	void initialize_gui();
	void display_error(QString msg);
	void plot_osm();
	void get_frames();
	
	void close_window();
	void closeEvent(QCloseEvent* event);
};


#endif