#pragma once
#ifndef FIXED_NOISE_CORRECTION_EXTERNAL_H
#define FIXED_NOISE_CORRECTION_EXTERNAL_H

#include <qcombobox.h>
#include <qdialog.h>
#include <qinputdialog.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qpushbutton.h>


#include "data_structures.h"
#include "engineering_data.h"
#include "osm_reader.h"
#include "plot_engineering_data.h"
#include "process_file.h"
#include "support/qthelpers.h"
#include "tracks.h"
#include "abir_reader.h"

class ExternalFixedNoiseInformationWidget : public QDialog
{
	Q_OBJECT

public:
	ExternalFixedNoiseInformationWidget(ABPFileType file_type);
	~ExternalFixedNoiseInformationWidget();

    AbpFileMetadata abp_metadata;
    ProcessFile file_processor;
	OSMReader osm_reader;
    std::vector<Frame> osm_frames;
	int start_frame, stop_frame;

public slots:

    void LoadOsmDataAndPlotFrames();

private:

	QString file_path, instructions;
	
	EngineeringData *engineering_data;
    EngineeringPlot* plot_data;
	TrackInformation *track_info;

	QPushButton* btn_load_file, *btn_load_frames, *btn_close;
	QLabel * lbl_data, *lbl_instructions;
	QFrame* frame_plot;

	QGridLayout* mainLayout;
	QVBoxLayout* frame_layout;

    void closeWindow();
    void closeEvent(QCloseEvent* event);
    void getFrames();

    void DisplayError(QString msg);
    void InitializeGui();
    void PlotOsmFrameData();
	ABPFileType file_type_{ABPFileType::ABP_B};
};


#endif
