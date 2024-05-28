#include "custom_input_dialog.h"

CustomInputDialog::CustomInputDialog(QVector<QString> combo_box_options, QString dialog_title, QString combo_label, int index_combo, QWidget * parent)
{

    InitializeGui(combo_box_options, dialog_title, combo_label);
	
	cmb_options->setCurrentIndex(index_combo);

	// Resize width of dialog box to fit dialog title length
	int title_length = dialog_title.length();

	// defaults to 15 in length. this is known to fit within title window
	if (title_length < 15)
		title_length = 15;

	int extra_length = title_length - 15;
	int per_length_increase = 5;

	// resize window 
	resize(225 + extra_length * per_length_increase, 50);

}

CustomInputDialog::~CustomInputDialog()
{
	delete cmb_options;
	delete btn_ok;
	delete btn_cancel;
	delete lbl_display;
	delete mainLayout;

}

void CustomInputDialog::InitializeGui(QVector<QString> combo_box_options, QString dialog_title, QString combo_label)
{

	// ------------------------------------------------------------
	// add color maps

	QList<QString> options;

	int num_options = combo_box_options.size();
	for (int i = 0; i < num_options; i++)
	{
		options.append(combo_box_options[i]);
	}

	cmb_options = new QComboBox();
	cmb_options->addItems(options);

	// ------------------------------------------------------------
	// define objects
	lbl_display = new QLabel(combo_label);
	
	// define buttons
	btn_ok = new QPushButton(tr("Ok"));
	btn_cancel = new QPushButton(tr("Cancel"));

    connect(btn_ok, &QPushButton::pressed, this, &CustomInputDialog::verifyCustomInputValues);
    connect(btn_cancel, &QPushButton::pressed, this, &CustomInputDialog::closeWindow);

	// ------------------------------------------------------------
	// set gridlayout

	mainLayout = new QGridLayout;

	mainLayout->addWidget(lbl_display, 0, 0, 1, 2, Qt::AlignCenter);
	mainLayout->addWidget(cmb_options, 1, 0, 1, 2, Qt::AlignCenter);

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
	setWindowTitle(dialog_title);
}

void CustomInputDialog::verifyCustomInputValues()
{
	done(QDialog::Accepted);
}

void CustomInputDialog::closeWindow()
{
	done(QDialog::Rejected);
}

void CustomInputDialog::closeEvent(QCloseEvent * event)
{
    closeWindow();
}
