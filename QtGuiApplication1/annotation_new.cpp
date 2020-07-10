#include "annotation_new.h"

NewAnnotation::NewAnnotation(annotation_info &data, Video *input_video, QWidget * parent)
{
	initialize_gui();

	// store current annotation being worked on
	current_data = &data;

	// pointer to video frame
	current_video = input_video;

	// set check values for class
	min_frame = data.min_frame;
	max_frame = data.max_frame;

	// set initial values for text edit boxes
	txt_annotation->setText(data.text);
	txt_frame_start->setText(QString::number(data.frame_start));
	txt_num_frames->setText(QString::number(data.num_frames));
	txt_x_loc->setText(QString::number(data.x_pixel));
	txt_y_loc->setText(QString::number(data.y_pixel));

	// set indices for combo boxes
	int index_colors = cmb_colors->findText(data.color);
	cmb_colors->setCurrentIndex(index_colors);

	int index_size = cmb_size->findText(QString::number(data.font_size));
	cmb_size->setCurrentIndex(index_size);

	// set connections
	connect(txt_annotation, &QLineEdit::editingFinished, this, &NewAnnotation::text_changed);
	connect(txt_frame_start, &QLineEdit::editingFinished, this, &NewAnnotation::frame_start_changed);
	connect(txt_num_frames, &QLineEdit::editingFinished, this, &NewAnnotation::number_of_frames_changed);
	connect(txt_x_loc, &QLineEdit::editingFinished, this, &NewAnnotation::x_location_changed);
	connect(txt_y_loc, &QLineEdit::editingFinished, this, &NewAnnotation::y_location_changed);

	connect(cmb_colors, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &NewAnnotation::color_changed);
	connect(cmb_size, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &NewAnnotation::font_size_changed);

	connect(btn_add, &QPushButton::pressed, this, &NewAnnotation::add);
	connect(btn_cancel, &QPushButton::pressed, this, &NewAnnotation::close_window);

}

NewAnnotation::~NewAnnotation()
{
	delete cmb_colors;
	delete cmb_size;
	
	delete txt_annotation;
	delete txt_x_loc;
	delete txt_y_loc;
	delete txt_frame_start;
	delete txt_num_frames;
	
	delete btn_add;
	delete btn_cancel;

	delete lbl_frame_start;
	delete lbl_num_frames;
	delete lbl_color;
	delete lbl_size; 
	delete lbl_x; 
	delete lbl_y;

	delete mainLayout;
}

int NewAnnotation::get_numeric_value(QString input)
{
	bool convert_value_numeric;
	int value = input.toInt(&convert_value_numeric);
	
	return value;
}

bool NewAnnotation::check_numeric_value(QString input)
{
	bool convert_value_numeric;
	int value = input.toInt(&convert_value_numeric);
	
	return convert_value_numeric;
}

void NewAnnotation::text_changed() {

	QString input = txt_annotation->text();
	current_data->text = input;

	current_video->update_display_frame();

}

void NewAnnotation::frame_start_changed() {
	
	// gets text
	QString input = txt_frame_start->text();
	
	//checks for numeric data
	bool is_numeric = check_numeric_value(input);
	if (is_numeric) {

		int new_value = get_numeric_value(input);
		
		// if new value is great than min frame and less than max frame, then sets the value
		if (new_value >= min_frame && new_value <= max_frame) {
			current_data->frame_start = new_value;
		}
		else
		{
			// if exceeds min/max, then reset to minimum frame
			current_data->num_frames = min_frame;
			txt_frame_start->setText(QString::number(min_frame));

			current_video->update_display_frame();
		}
	}
	else
	{
		QString msg("Input for starting frame is non-numeric");
		display_error(msg);
		txt_frame_start->setFocus();
	}

}

void NewAnnotation::number_of_frames_changed()
{
	// gets text
	QString input = txt_num_frames->text();
	
	//checks for numeric data
	bool is_numeric = check_numeric_value(input);
	if (is_numeric) {

		int new_value = get_numeric_value(input);

		// if number of frames exceeds maximum frames, then sets the value to max allowable
		if (current_data->frame_start + new_value < max_frame) {
			current_data->num_frames = get_numeric_value(input);
		}
		else
		{
			// if exceeds max, then reset to maximum frame
			current_data->num_frames = max_frame - min_frame;
			txt_num_frames->setText(QString::number(max_frame - min_frame));
		}

		current_video->update_display_frame();
	}
	else
	{
		QString msg("Input for number of frames is non-numeric");
		display_error(msg);
	}
}

void NewAnnotation::x_location_changed()
{
	// gets text
	QString input = txt_x_loc->text();
	
	//checks for numeric data
	bool is_numeric = check_numeric_value(input);
	if (is_numeric) {

		// checks to make sure min/max values not exceeded. if exceeded then replaces
		int new_x_position = get_numeric_value(input);

		if (new_x_position < current_data->x_min_position)
			new_x_position = current_data->x_min_position;

		if (new_x_position > current_data->x_max_position)
			new_x_position = current_data->x_max_position;

		// store and set text box to potential new value
		current_data->x_pixel = new_x_position;
		txt_x_loc->setText(QString::number(new_x_position));

		current_video->update_display_frame();
	}
	else
	{
		QString msg("Input for x-location is non-numeric");
		display_error(msg);
	}
}

void NewAnnotation::y_location_changed()
{
	// gets text
	QString input = txt_y_loc->text();
	
	//checks for numeric data
	bool is_numeric = check_numeric_value(input);
	if (is_numeric) {
		
		// checks to make sure min/max values not exceeded. if exceeded then replaces
		int new_y_position = get_numeric_value(input);

		if (new_y_position < current_data->y_min_position)
			new_y_position = current_data->y_min_position;

		if (new_y_position > current_data->y_max_position)
			new_y_position = current_data->y_max_position;

		// store and set text box to potential new value
		current_data->y_pixel = new_y_position;
		txt_y_loc->setText(QString::number(new_y_position));

		current_video->update_display_frame();
	}
	else
	{
		QString msg("Input for y-location is non-numeric");
		display_error(msg);
	}
}

void NewAnnotation::color_changed(const QString & text)
{
	QString input = text;
	current_data->color = input;

	current_video->update_display_frame();

}

void NewAnnotation::font_size_changed(const QString & text)
{
	int value = text.toInt();
	current_data->font_size = value;

	current_video->update_display_frame();
}

void NewAnnotation::initialize_gui()
{
	// ------------------------------------------------------------
	// add colors and sizes
	
	colors.append("red");
	colors.append("orange");
	colors.append("yellow");
	colors.append("green");
	colors.append("blue");
	colors.append("violet");

	// add sizes
	sizes.append("8");
	sizes.append("10");
	sizes.append("12");
	sizes.append("14");
	sizes.append("16");
	sizes.append("18");

	cmb_colors = new QComboBox();
	cmb_colors->addItems(colors);
	
	cmb_size = new QComboBox();
	cmb_size->addItems(sizes);
	// ------------------------------------------------------------

	// define objects
	lbl_frame_start = new QLabel(tr("First Frame"));
	lbl_num_frames = new QLabel(tr("# of Frames"));
	lbl_color = new QLabel(tr("Color"));
	lbl_size = new QLabel(tr("Font Size (pt)"));
	lbl_x = new QLabel(tr("X Position"));
	lbl_y = new QLabel(tr("Y Position"));

	btn_add = new QPushButton(tr("Add"));
	btn_cancel = new QPushButton(tr("Cancel"));

	txt_annotation = new QLineEdit(tr("Insert Text Here"));
	txt_frame_start = new QLineEdit(tr("1"));
	txt_num_frames = new QLineEdit(tr("1000"));
	txt_x_loc = new QLineEdit(tr("0"));
	txt_y_loc = new QLineEdit(tr("0"));

	// ------------------------------------------------------------
	// set gridlayout
	
	mainLayout = new QGridLayout;
	mainLayout->addWidget(txt_annotation, 0, 0, 1, 2);
	
	mainLayout->addWidget(btn_add, 0, 2);
	mainLayout->addWidget(btn_cancel, 1, 2);

	mainLayout->addWidget(lbl_x, 1, 0);
	mainLayout->addWidget(lbl_y, 1, 1);
	mainLayout->addWidget(txt_x_loc, 2, 0);
	mainLayout->addWidget(txt_y_loc, 2, 1);

	mainLayout->addWidget(lbl_size, 3, 0);
	mainLayout->addWidget(lbl_color, 3, 1);
	mainLayout->addWidget(cmb_size, 4, 0);
	mainLayout->addWidget(cmb_colors, 4, 1);
	mainLayout->addWidget(lbl_frame_start, 5, 0);
	mainLayout->addWidget(lbl_num_frames, 5, 1);
	mainLayout->addWidget(txt_frame_start, 6, 0);
	mainLayout->addWidget(txt_num_frames, 6, 1);

	
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
	setWindowTitle(tr("Add New Annotation"));

}

void NewAnnotation::display_error(QString msg)
{

	QMessageBox msgBox;
	msgBox.setWindowTitle(QString("Input Value Error"));
	QString box_text = msg;
	msgBox.setText(box_text);

	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();

	return;
}

void NewAnnotation::add()
{
	
	done(QDialog::Accepted);

	current_data = NULL;
}

void NewAnnotation::close_window()
{
	
	done(QDialog::Rejected);

	current_data = NULL;
}

void NewAnnotation::closeEvent(QCloseEvent * event)
{
	close_window();
}
