#include "fixed_noise_correction_external_file.h"

ExternalFixedNoiseInformationWidget::ExternalFixedNoiseInformationWidget(ABPFileType file_type)
{
    osmTrackDataLoaded = true;

    file_type_ = file_type;
    if (file_type == ABPFileType::ABP_D)
    {
        int nRows = 720;
        int nRows2 = nRows/2;
        int nCols = 1280;
    }
    InitializeGui();
	engineering_data = NULL;
    
    this->show();
}

ExternalFixedNoiseInformationWidget::~ExternalFixedNoiseInformationWidget()
{
}

void ExternalFixedNoiseInformationWidget::InitializeGui()
{
    mainLayout = new QGridLayout();

    instructions = "Open the desired *.abpimage file corresponding to the frames to use. When frames have been identified, 'Load Frames'.";
   
    lbl_instructions = new QLabel(instructions);
    lbl_instructions->setWordWrap(true);

    btn_load_file = new QPushButton("Load OSM File");
    btn_load_file->setFixedWidth(150);
    btn_load_frames = new QPushButton("Load Frames");
    btn_load_frames->setEnabled(false);
    btn_load_frames->setFixedWidth(150);
    btn_close = new QPushButton("Cancel");
    btn_close->setFixedWidth(100);

    frame_plot = new QFrame();

    frame_layout = new QVBoxLayout();
    frame_plot->setLayout(frame_layout);
    frame_plot->setFixedWidth(200);

    lbl_instructions->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    btn_load_file->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);   
    btn_load_frames->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        
    mainLayout->addWidget(lbl_instructions, 0, 0);
    mainLayout->addWidget(btn_load_file, 1, 0);
    mainLayout->addWidget(frame_plot, 2, 0);
    mainLayout->addWidget(btn_load_frames, 3, 0);

    this->setLayout(mainLayout);

    connect(btn_load_file, &QPushButton::clicked, this, &ExternalFixedNoiseInformationWidget::LoadOsmDataAndPlotFrames);
    connect(btn_load_frames, &QPushButton::clicked, this, &ExternalFixedNoiseInformationWidget::getFrames);
    connect(btn_close, &QPushButton::clicked, this, &ExternalFixedNoiseInformationWidget::close);
}

void ExternalFixedNoiseInformationWidget::LoadOsmDataAndPlotFrames()
{
    QString file_selection = QFileDialog::getOpenFileName(this, ("Open File"), "", ("Image File(*.abpimage)"));
    abp_metadata = file_processor.LocateAbpFiles(file_selection);

    // check that osm and image files are present
    if (!abp_metadata.error_msg.isEmpty())
    {
        QtHelpers::LaunchMessageBox(QString("Issue Loading File"), abp_metadata.error_msg);
        return;
    }

    if (!osm_reader.Open(abp_metadata.osm_path))
    {
        QtHelpers::LaunchMessageBox(QString("Error loading OSM file"), QString("Error reading OSM file. Close program and open logs for details."));
        return;
    }

    osm_frames = osm_reader.ReadFrames(file_type_);
    if (osm_frames.size() == 0)
	{
        QtHelpers::LaunchMessageBox(QString("Error loading OSM file"), QString("Error reading OSM file. Close program and open logs for details."));
		return;
	}

    PlotOsmFrameData();
}

void ExternalFixedNoiseInformationWidget::PlotOsmFrameData()
{
    if (engineering_data != NULL) {

        // delete objects with existing data within them
        delete engineering_data;
    }

    engineering_data = new EngineeringData(osm_frames);
    plot_data = new EngineeringPlot(osm_frames, "SumCounts", {Quantity("SumCounts", Enums::PlotUnit::Undefined_PlotUnit), Quantity("Frames", Enums::PlotUnit::Undefined_PlotUnit)}); // TODO: Pull in metadata for this.

    track_info = new TrackInformation(osm_frames,file_type_);
    plot_data->set_plotting_track_frames(track_info->GetOsmPlottingTrackFrames(), track_info->GetTrackCount());

    plot_data->past_midnight = engineering_data->get_seconds_from_midnight();
    plot_data->past_epoch = engineering_data->get_seconds_from_epoch();
    plot_data->SetPlotClassification("");
    plot_data->PlotChart(osmTrackDataLoaded);

    btn_load_frames->setEnabled(true);
}

void ExternalFixedNoiseInformationWidget::getFrames()
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

    // get min frame for FixedNoise while limiting input between 1 and total messages
    start_frame = QInputDialog::getInt(this, "Exernal File Start Frame", prompt1, 1, 1, num_messages, 1, &ok);
    if (!ok)
        return;

    // get max frame for FixedNoise while limiting input between min and total messages
    stop_frame = QInputDialog::getInt(this, "Exernal File Stop Frame", prompt2, start_frame, start_frame, num_messages, 1, &ok);
    if (!ok)
        return;

    done(QDialog::Accepted);
}

void ExternalFixedNoiseInformationWidget::closeWindow()
{
    done(QDialog::Rejected);
}

void ExternalFixedNoiseInformationWidget::closeEvent(QCloseEvent* event)
{
    closeWindow();
}
