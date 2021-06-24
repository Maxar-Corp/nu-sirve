#include "calibration_data.h"

CalibrationData::CalibrationData()
{
	// set check variable to false to start
	calibration_available = false;

}

double CalibrationData::measure_irradiance(int ul_row, int ul_col, int lr_row, int lr_col)
{
	
	return 0.0;
}

// --------------------------------------------------------------------------------------------------------------------

CalibrationDialog::CalibrationDialog(QWidget* parent)
{
	initialize_gui();
}

CalibrationDialog::~CalibrationDialog()
{

}

void CalibrationDialog::initialize_gui()
{

	// define dialog buttons
	btn_ok = new QPushButton(tr("Ok"));
	btn_cancel = new QPushButton(tr("Cancel"));

	connect(btn_ok, &QPushButton::pressed, this, &CalibrationDialog::ok);
	connect(btn_cancel, &QPushButton::pressed, this, &CalibrationDialog::close_window);

	// ------------------------------------------------------------
	// set layout

	mainLayout = new QVBoxLayout();

	btn_get_nuc_file = new QPushButton("Import NUC File");
	lbl_nuc_filename = new QLabel("File: ");

	QFrame* horizontal_segment1 = new QFrame();
	horizontal_segment1->setFrameShape(QFrame::HLine);

	mainLayout->addWidget(btn_get_nuc_file, Qt::AlignCenter);
	mainLayout->addWidget(lbl_nuc_filename, Qt::AlignCenter);
	mainLayout->addWidget(horizontal_segment1);

	// ------------------------------------------------------------
	
	frame_plot = new FixedAspectRatioFrame();
	frame_plot->enable_fixed_aspect_ratio(true);
	frame_plot->resize(640, 480);

	lbl_temp1 = new QLabel("Temperature 1: ");
	lbl_temp2 = new QLabel("Temperature 2: ");
	QLabel* lbl_additional_frames = new QLabel("Additional Frames Used for Averaging:");
	txt_additional_frames = new QLineEdit("0");

	QHBoxLayout* hlayout_plot_row = new QHBoxLayout();
	hlayout_plot_row->addWidget(frame_plot);

	QVBoxLayout* vlayout_temperature_info = new QVBoxLayout();
	vlayout_temperature_info->addWidget(lbl_temp1);
	vlayout_temperature_info->addWidget(lbl_temp2);
	vlayout_temperature_info->addWidget(lbl_additional_frames);
	vlayout_temperature_info->addWidget(txt_additional_frames);

	hlayout_plot_row->addLayout(vlayout_temperature_info);

	mainLayout->addLayout(hlayout_plot_row);

	// ------------------------------------------------------------

	QHBoxLayout* hlayout_buttons = new QHBoxLayout();
	hlayout_buttons->addWidget(btn_ok, Qt::AlignCenter);
	hlayout_buttons->addWidget(btn_cancel, Qt::AlignCenter);

	mainLayout->addLayout(hlayout_buttons);

	setLayout(mainLayout);
	setWindowTitle("Set Calibration Data");

}

void CalibrationDialog::ok()
{
	done(QDialog::Accepted);
}

void CalibrationDialog::close_window()
{
	done(QDialog::Rejected);
}

void CalibrationDialog::import_nuc_file()
{
}

void CalibrationDialog::closeEvent(QCloseEvent* event)
{
	close_window();
}
