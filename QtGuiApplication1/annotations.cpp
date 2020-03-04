#include "annotations.h"

Annotations::Annotations(QWidget * parent) 
{
	initialize_gui();	

	connect(btn_ok, &QPushButton::pressed, this, &Annotations::ok);
	connect(btn_new, &QPushButton::pressed, this, &Annotations::add);
}

Annotations::~Annotations() {

	delete lbl_annotations;
	delete lbl_description;
	
	delete btn_ok;
	delete btn_edit;
	delete btn_new;
	delete btn_delete;

	delete lst_annotations;
}

void Annotations::initialize_gui()
{

	// define objects
	lbl_annotations = new QLabel(tr("Annotations"));
	lst_annotations = new QListWidget();
	lbl_description = new QLabel(tr("Descriptions"));

	btn_ok = new QPushButton(tr("OK"));
	btn_edit = new QPushButton(tr("Edit"));
	btn_new = new QPushButton(tr("New"));
	btn_delete = new QPushButton(tr("Delete"));

	// set gridlayout
	QGridLayout *mainLayout = new QGridLayout;
	//mainLayout->addWidget(lbl_annotations, 0, 0, 1, 4, Qt::AlignLeft);
	mainLayout->addWidget(lst_annotations, 0, 0, 4, 1);
	mainLayout->addWidget(lbl_description, 0, 1, 4, 1);
	mainLayout->addWidget(btn_ok, 0, 2);
	mainLayout->addWidget(btn_new, 1, 2);
	mainLayout->addWidget(btn_edit, 2, 2);
	mainLayout->addWidget(btn_delete, 3, 2);

	// set grid characterestics
	//mainLayout->setRowStretch(0, 0);
	mainLayout->setRowStretch(0, 100);
	mainLayout->setRowStretch(1, 100);
	mainLayout->setRowStretch(2, 100);
	mainLayout->setRowStretch(3, 100);

	mainLayout->setColumnMinimumWidth(0, 100);
	mainLayout->setColumnMinimumWidth(1, 150);

	mainLayout->setColumnStretch(0, 100);
	mainLayout->setColumnStretch(1, 100);
	mainLayout->setColumnStretch(2, 0);

	setLayout(mainLayout);
	setWindowTitle(tr("Annotations"));

}

void Annotations::ok()
{
	
	close();
}

void Annotations::add()
{
	// set user definable attributes
	annotation_info new_data;
	new_data.color = "red";
	new_data.font_size = 8;
	new_data.x_pixel = 1;
	new_data.y_pixel = 1;
	new_data.frame_start = 200;
	new_data.num_frames = 600;
	new_data.text = "Add Text";

	// set attributes for checking data
	new_data.min_frame = 200;
	new_data.max_frame = 800;
	new_data.x_min_position = 1;
	new_data.x_max_position = 640;
	new_data.y_min_position = 1;
	new_data.y_max_position = 480;
	
	// display new annotation screen
	NewAnnotation add_annotation(new_data);
	auto response = add_annotation.exec();

	std::cout << response;

}

void Annotations::edit()
{
	// set user definable attributes

	int index = lst_annotations->currentRow();
	annotation_info edit_data = data[index];

	// TODO copy old data into separate structure
	
	// display new annotation screen
	NewAnnotation add_annotation(edit_data);
	auto response = add_annotation.exec();

	// TODO if cancelled, put old data back

}
