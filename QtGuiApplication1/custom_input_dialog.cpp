#include "custom_input_dialog.h"

CustomInputDialog::CustomInputDialog(QVector<QString> combo_box_options, QString dialog_title, QString combo_label, int * index_combo, Video * input_video, QWidget * parent)
{

	current_video = input_video;
	current_index = index_combo;

	initialize_gui(combo_box_options, dialog_title, combo_label);
	
	
	QComboBox *cmb_options;
	QPushButton *btn_ok, *btn_cancel;
	QLabel *lbl_display;

	QGridLayout *mainLayout;

	Video *current_video;


}

CustomInputDialog::~CustomInputDialog()
{
	delete cmb_options;
	delete btn_ok;
	delete btn_cancel;
	delete lbl_display;
	delete mainLayout;

}

void CustomInputDialog::initialize_gui(QVector<QString> combo_box_options, QString dialog_title, QString combo_label)
{

	// ------------------------------------------------------------
	// add color maps

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
	// ------------------------------------------------------------
	// set gridlayout

	mainLayout = new QGridLayout;
	
	mainLayout->addWidget(btn_ok, 0, 2);
	mainLayout->addWidget(btn_cancel, 1, 2);

	mainLayout->addWidget(lbl_display, 1, 0);
	mainLayout->addWidget(cmb_options, 4, 1);

	// set grid characterestics
	mainLayout->setRowStretch(0, 0);
	mainLayout->setRowStretch(1, 0);
	mainLayout->setRowStretch(2, 0);
	mainLayout->setRowStretch(3, 0);
	mainLayout->setRowStretch(4, 0);

	mainLayout->setColumnStretch(0, 100);
	mainLayout->setColumnStretch(1, 100);
	mainLayout->setColumnStretch(2, 0);

	setLayout(mainLayout);
	setWindowTitle(dialog_title);

}

void CustomInputDialog::combo_box_changed(const QString &text)
{

}

void CustomInputDialog::ok()
{
	done(QDialog::Accepted);
}

void CustomInputDialog::close_window()
{
	done(QDialog::Rejected);
}

void CustomInputDialog::closeEvent(QCloseEvent * event)
{
	close_window();
}
