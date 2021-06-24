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

	// define buttons
	btn_ok = new QPushButton(tr("Ok"));
	btn_cancel = new QPushButton(tr("Cancel"));

	connect(btn_ok, &QPushButton::pressed, this, &CalibrationDialog::ok);
	connect(btn_cancel, &QPushButton::pressed, this, &CalibrationDialog::close_window);

	// ---------------------------------------------------------------------------

	// ------------------------------------------------------------
	// set gridlayout

	mainLayout = new QGridLayout;

	QLabel *lbl_display = new QLabel("Test Label");
	mainLayout->addWidget(lbl_display, 0, 0, 1, 2, Qt::AlignCenter);

	mainLayout->addWidget(btn_ok, 2, 0, 1, 1, Qt::AlignCenter);
	mainLayout->addWidget(btn_cancel, 2, 1, 1, 1, Qt::AlignCenter);

	mainLayout->setColumnMinimumWidth(0, 100);
	mainLayout->setColumnMinimumWidth(1, 100);

	// set grid characterestics
	mainLayout->setRowStretch(0, 1);
	mainLayout->setRowStretch(1, 1);
	mainLayout->setRowStretch(2, 1);

	mainLayout->setColumnStretch(0, 1);
	mainLayout->setColumnStretch(1, 1);

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

void CalibrationDialog::closeEvent(QCloseEvent* event)
{
	close_window();
}
