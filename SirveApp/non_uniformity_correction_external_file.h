#pragma once
#pragma once
#ifndef EXTERNAL_NUC_H
#define EXTERNAL_NUC_H

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
#include "tracks.h"
#include "engineeringdata.h"
#include "plot_engineering_data.h"
#include "process_file.h"
#include "osm_reader.h"
#include "support/qthelpers.h"


class ExternalNUCInformationWidget : public QDialog
{
	Q_OBJECT

public:
	ExternalNUCInformationWidget();
	~ExternalNUCInformationWidget();
    ProcessFile file_processor;
	OSMReader osm_reader;
	std::vector<Frame> osm_frames;
	AbpFileMetadata abp_metadata;
	int start_frame, stop_frame;

public slots:

	void get_osm_file();

private:

	QString file_path, instructions;
	
	EngineeringData *engineering_data;
    EngineeringPlots* plot_data;
	TrackInformation *track_info;

	QPushButton* btn_load_file, *btn_load_frames, *btn_close;
	QLabel * lbl_data, *lbl_instructions;
	QFrame* frame_plot;

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
