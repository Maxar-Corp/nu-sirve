#include "non_uniformity_correction_external_file.h"

ExternalNUCInformationWidget::ExternalNUCInformationWidget()
{

    InitializeGui();
	engineering_data = NULL;
    
    this->show();
}

ExternalNUCInformationWidget::~ExternalNUCInformationWidget()
{
}

void ExternalNUCInformationWidget::InitializeGui()
{
    mainLayout = new QGridLayout();

    instructions = "Open the desired *.abpimage file corresponding to the frames to use with the NUC. When frames have been identified, 'Load Frames' for the NUC.";
   
    lbl_instructions = new QLabel(instructions);
    lbl_instructions->setWordWrap(true);

    btn_load_file = new QPushButton("Load OSM File");
    btn_load_frames = new QPushButton("Load Frames");
    btn_load_frames->setEnabled(false);
    btn_close = new QPushButton("Cancel");

    frame_plot = new QFrame();

    frame_layout = new QVBoxLayout();
    frame_plot->setLayout(frame_layout);

    lbl_instructions->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    btn_load_file->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);   
    btn_load_frames->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        
    mainLayout->addWidget(lbl_instructions, 0, 0);
    mainLayout->addWidget(btn_load_file, 1, 0);
    mainLayout->addWidget(frame_plot, 2, 0);
    mainLayout->addWidget(btn_load_frames, 3, 0);

    this->setLayout(mainLayout);

    connect(btn_load_file, &QPushButton::clicked, this, &ExternalNUCInformationWidget::LoadOsmDataAndPlotFrames);
    connect(btn_load_frames, &QPushButton::clicked, this, &ExternalNUCInformationWidget::getFrames);
    connect(btn_close, &QPushButton::clicked, this, &ExternalNUCInformationWidget::close);
}

void ExternalNUCInformationWidget::LoadOsmDataAndPlotFrames()
{
    QString file_selection = QFileDialog::getOpenFileName(this, ("Open File"), "", ("Image File(*.abpimage)"));
    abp_metadata = file_processor.LocateAbpFiles(file_selection);

    // check that osm and image files are present
    if (!abp_metadata.error_msg.isEmpty())
    {
        QtHelpers::LaunchMessageBox(QString("Issue Loading File"), abp_metadata.error_msg);
        return;
    }

    osm_frames = osm_reader.ReadOsmFileData(abp_metadata.osm_path);
    if (osm_frames.size() == 0)
	{
        QtHelpers::LaunchMessageBox(QString("Error loading OSM file"), QString("Error reading OSM file. Close program and open logs for details."));
		return;
	}

    PlotOsmFrameData();

}

void ExternalNUCInformationWidget::PlotOsmFrameData()
{

    if (engineering_data != NULL) {

        // delete objects with existing data within them
        frame_layout->removeWidget(plot_data->chart_view);
        delete engineering_data;
    }

    int width, height;
    width = this->width();
    height = this->height();

    if (height < 500)
        this->resize(500, 500);

    engineering_data = new EngineeringData(osm_frames);
    plot_data = new EngineeringPlots(osm_frames);

    track_info = new TrackInformation(osm_frames);
    plot_data->set_plotting_track_frames(track_info->get_osm_plotting_track_frames(), track_info->get_track_count());

    plot_data->past_midnight = engineering_data->get_seconds_from_midnight();
    plot_data->past_epoch = engineering_data->get_seconds_from_epoch();
    plot_data->SetPlotTitle("");

    frame_layout->addWidget(plot_data->chart_view);

    plot_data->toggle_yaxis_log(true);
    plot_data->SetYAxisChartId(2);
    plot_data->PlotChart();

    btn_load_frames->setEnabled(true);    
    
}

void ExternalNUCInformationWidget::getFrames()
{
    
    // get total number of frames
    int num_messages = osm_frames.size();

    QString prompt1 = "Start Frame (";
    prompt1.append(QString::number(num_messages));
    prompt1.append(" frames total)");

    QString prompt2 = "Stop Frame (";
    prompt2.append(QString::number(num_messages));
    prompt2.append(" frames total)");

    bool ok;

    // get min frame for nuc while limiting input between 1 and total messages
    start_frame = QInputDialog::getInt(this, "Exernal File NUC Correction", prompt1, 1, 1, num_messages, 1, &ok);
    if (!ok)
        return;

    // get max frame for nuc while limiting input between min and total messages
    stop_frame = QInputDialog::getInt(this, "Exernal File NUC Correction", prompt2, start_frame, start_frame, num_messages, 1, &ok);
    if (!ok)
        return;

    done(QDialog::Accepted);
}

void ExternalNUCInformationWidget::closeWindow()
{
    done(QDialog::Rejected);
}

void ExternalNUCInformationWidget::closeEvent(QCloseEvent* event)
{
    closeWindow();
}
