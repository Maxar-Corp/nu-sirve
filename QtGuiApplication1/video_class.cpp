#include "video_class.h"


Video::Video(int x_pixels, int y_pixels, int input_bit_level)
{
	label = new EnhancedLabel(this);
	label->setBackgroundRole(QPalette::Base);
	label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	label->setScaledContents(true);

	
	histogram_plot = new HistogramLine_Plot(input_bit_level);

	counter = 0;
	counter_record = 0;
	record_frame = false;
	show_relative_histogram = false;

	image_x = x_pixels;
	image_y = y_pixels;
	number_pixels = image_x * image_y;
	max_bit_level = input_bit_level;
	number_of_frames = 0;

	banner_text = QString("EDIT CLASSIFICATION");
	banner_color = QColor("Red");
	tracker_color = QColor("Red");
	plot_tracks = false;
	display_boresight_txt = false;
	display_tgt_pos_txt = false;

	// intializes color map to gray scale
	index_video_color = 0;
	colorTable = video_colors.maps[index_video_color].colors;

	connect(label, &EnhancedLabel::highlighted_area, this, &Video::zoom_image);
	connect(label, &EnhancedLabel::right_clicked, this, &Video::unzoom);

	label->setObjectName("video_object");
	label->setStyleSheet("#video_object { border: 1px solid light gray; }");
}

/*
Video::Video(std::vector<std::vector<uint16_t>> &video_data, int x_pixels, int y_pixels)
{
	label = new QLabel(this);
	text = new QLabel(this);

	counter = 0;
	
	update_video_file(video_data, x_pixels, y_pixels);
}
*/
Video::~Video()
{
	delete  label;
}

void Video::update_video_file(std::vector<std::vector<uint16_t>>& video_data, int x_pixels, int y_pixels)
{
	
	frame_data = video_data;
	number_of_frames = video_data.size();
	
	image_x = x_pixels;
	image_y = y_pixels;
	number_pixels = image_x * image_y;



}

void Video::clear_all_zoom_levels(int x_pixels, int y_pixels) {

	zoom_list.clear();
	QRect new_zoom(0, 0, x_pixels, y_pixels);
	zoom_list.push_back(new_zoom);

	// resets border color
	label->setObjectName("video_object");
	label->setStyleSheet("#video_object { border: 1px solid light gray; }");

}

void Video::receive_video_data(video_details &new_input)
{

	update_video_file(new_input.frames_16bit, new_input.x_pixels, new_input.y_pixels);

}

void Video::update_banner_text(QString input_banner_text)
{
	banner_text = input_banner_text;
	update_display_frame();
}

void Video::update_banner_color(QString input_color)
{
	QColor new_color(input_color);
	banner_color = new_color;
}

void Video::update_color_map(QString input_map)
{
	
	// find number of color maps
	int number_maps = video_colors.maps.size();
	
	// cycle through all color maps
	for (int i = 0; i < number_maps; i++)
	{
		// checks to find where input_map matches provided maps
		if (input_map == video_colors.maps[i].name)
			index_video_color = i;
	}

	// sets color table
	colorTable = video_colors.maps[index_video_color].colors;
}

void Video::update_tracker_color(QString input_color)
{
	QColor new_color(input_color);
	tracker_color = new_color;
}

void Video::toggle_osm_tracks(bool input)
{
	plot_tracks = input;
}

void Video::toggle_primary_track_data(bool input)
{
	display_tgt_pos_txt = input;
}

void Video::toggle_sensor_boresight_data(bool input)
{
	display_boresight_txt = input;
}

void Video::toggle_relative_histogram()
{
	if (show_relative_histogram)
		show_relative_histogram = false;
	else
		show_relative_histogram = true;
}

void Video::zoom_image(QRect info)
{

	// check to make sure rectangle doesn't exceed dimensions. if so, shorten
	if (info.x() + info.width() > image_x)
	{
		info.setWidth(image_x - info.x());
	}

	if (info.y() + info.height() > image_y)
	{
		info.setHeight(image_y - info.y());
	}

	// if width/heigh is less than 10 pixels long, then this was not a zoomable area and return
	if (info.width() < 10 || info.height() < 10)
	{
		return;
	}


	int height = info.height();
	int width = info.width();

	int x = info.x();
	int y = info.y();

	double aspect_ratio = image_x * 1.0 / image_y;
	double adj_width = height * aspect_ratio;
	double adj_height = width / aspect_ratio;

	// adjust size of box to fit aspect ratio and encompass the highlighted area selected
	if (adj_width > width) {
		
		width  = adj_width;	
		height = adj_width / aspect_ratio;
	}
	else if (adj_height > height) {
		
		width = adj_height * aspect_ratio;
		height = adj_height;
	}
	else {
		
	}

	// if updated area exceeds width, move origin to left
	if (x + width > image_x)
	{
		int delta = x + width - image_x;
		x = x - delta;
	}
	
	// if updated area exceeds height, move origin up
	if (y + height > image_y)
	{
		int delta = y + height - image_y;
		y = y - delta;
	}

	// set zoom area to appropriate values
	QRect new_zoom(x, y, width, height);
	zoom_list.push_back(new_zoom);

	// changes color of frame when zoomed within frame
	label->setObjectName("video_object");
	label->setStyleSheet("#video_object { border: 3px solid blue; }");

	update_display_frame();
}

void Video::unzoom(QPoint origin)
{
	
	if (zoom_list.size() > 1)
	{
		zoom_list.pop_back();
		update_display_frame();
	}
	
	// resets border color when not zoomed
	if (zoom_list.size() == 1)
	{
		label->setObjectName("video_object");
		label->setStyleSheet("#video_object { border: 1px solid light gray; }");
	}
}

std::vector<int> Video::get_position_within_zoom(int x0, int y0)
{
	
	int num_zooms = zoom_list.size();
	bool pt_within_area = true;

	int x_center = x0;
	int y_center = y0;

	// for each zoom level ...
	for (int j = 0; j < num_zooms; j++)
	{
		// define object location relative to zoom frame
		QRect sub_frame = zoom_list[j];
		x_center = x_center - sub_frame.x();
		y_center = y_center - sub_frame.y();

		// if object location exceeds frame, stop and prevent drawing
		if (x_center < 0 || x_center > sub_frame.width())
		{
			pt_within_area = false;
			break;
		}

		if (y_center < 0 || y_center > sub_frame.height())
		{
			pt_within_area = false;
			break;
		}

		// rescale pixels to image frame
		double temp_x = x_center * 1.0 / sub_frame.width() * image_x;
		double temp_y = y_center * 1.0 / sub_frame.height() * image_y;

		x_center = std::round(temp_x);
		y_center = std::round(temp_y);

	}

	// if point is within all zoom frames ...
	if (pt_within_area)
	{
		return std::vector<int> {x_center, y_center};
	}
	else
		return std::vector<int> {-1, -1};
}

void Video::update_display_frame()
{
	// In case update_display_frame is called before a video is fully placed 
	if (frame_data.size() < counter)
		return;
	
	//------------------------------------------------------------------------------------------------

	//Convert current frame to armadillo matrix
	std::vector<double> frame_vector(frame_data[counter].begin(), frame_data[counter].end());
	arma::vec image_vector(frame_vector);

	//Normalie the image to values between 0 - 1
	int max_value = std::pow(2, max_bit_level);
	image_vector = image_vector / max_value;
	
	//------------------------------------------------------------------------------------------------
	// Update the absolute histogram
	histogram_plot->plot_absolute_histogram(image_vector, color_correction.get_min(), color_correction.get_max());

	// Correct image based on min/max value inputs
	color_correction.get_updated_color(image_vector);

	// Create points for the relative histogram
	if (show_relative_histogram) {
		histogram_plot->plot_relative_histogram(image_vector);
	}
	else
	{
		histogram_plot->rel_chart->removeAllSeries();
	}

	//------------------------------------------------------------------------------------------------

	// Put image into 8-bit format for displaying
	image_vector = image_vector * 255;
	
	//arma::vec out_frame_flat = arma::vectorise(image_vector);
	std::vector<double>out_vector = arma::conv_to<std::vector<double>>::from(image_vector);
	std::vector<uint8_t> converted_values(out_vector.begin(), out_vector.end());
	uint8_t *color_corrected_frame = converted_values.data();

	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------

	frame = QImage((uchar *)color_corrected_frame, image_x, image_y, QImage::Format_Grayscale8);

	// Convert image to format_indexed. allows color table to take effect on image
	frame = frame.convertToFormat(QImage::Format_Indexed8);
	frame.setColorTable(colorTable);

	// Convert image back to RGB to facilitate use of the colors 
	frame = frame.convertToFormat(QImage::Format_RGB888);

	// -----------------------------------------------------------
	// loop thru all user set zooms
	for (int i = 0; i < zoom_list.size(); i++)
	{
		QRect sub_frame = zoom_list[i];
		
		// get sub-image
		frame = frame.copy(sub_frame);

		// scale to initial aspect ratio
		frame = frame.scaled(image_x, image_y);
	}

	// -----------------------------------------------------------
	// loop thru all tracks and plot thru lowest zoom

	int num_tracks = display_data[counter].ir_data.size();
	int num_zooms = zoom_list.size();

	if (plot_tracks & num_tracks > 0) {

		for (int i = 0; i < num_tracks; i++)
		{
			// Find location of object in frame
			int x_pixel = display_data[counter].ir_data[i].centroid_x;
			int y_pixel = display_data[counter].ir_data[i].centroid_y;

			int x_center = image_x / 2 + x_pixel;
			int y_center = image_y / 2 + y_pixel;

			bool pt_within_area = true;

			// for each zoom level ...
			for (int j = 0; j < num_zooms; j++)
			{
				// define object location relative to zoom frame
				QRect sub_frame = zoom_list[j];
				x_center = x_center - sub_frame.x();
				y_center = y_center - sub_frame.y();

				// if object location exceeds frame, stop and prevent drawing
				if (x_center < 0 || x_center > sub_frame.width())
				{
					pt_within_area = false;
					break;
				}

				if (y_center < 0 || y_center > sub_frame.height())
				{
					pt_within_area = false;
					break;
				}

				// rescale pixels to image frame
				double temp_x = x_center  * 1.0 / sub_frame.width() * image_x;
				double temp_y = y_center * 1.0 / sub_frame.height() * image_y;

				x_center = std::round(temp_x);
				y_center = std::round(temp_y);

			}

			// if point is within all zoom frames ...
			if (pt_within_area)
			{
				// draw rectangle around object				
				QPainter rectangle_painter(&frame);
				
				int box_size = 5;
				QRectF rectangle(x_center - box_size, y_center - box_size, box_size * 2, box_size * 2);

				rectangle_painter.setPen(QPen(tracker_color));
				rectangle_painter.drawRect(rectangle);
			}
		}
	}

	if (display_boresight_txt) {
		QPainter p2(&frame);
		p2.setPen(QPen(banner_color));
		p2.setFont(QFont("Times", 8, QFont::Bold));

		QString boresight_txt = "Sensor Boresight \n Az: " + QString::number(display_data[counter].azimuth_sensor) + "\n El " + QString::number(display_data[counter].elevation_sensor);

		p2.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignLeft, boresight_txt);
	}

	if (display_tgt_pos_txt) {
		QPainter p3(&frame);
		p3.setPen(QPen(banner_color));
		p3.setFont(QFont("Times", 8, QFont::Bold));

		double az_value = display_data[counter].azimuth_p_tgt;
		double el_value = display_data[counter].elevation_p_tgt;

		QString primary_tgt_text;
		if (az_value < -1000 || el_value < -1000)
			primary_tgt_text = "Target Metrics \n Az: NaN \n El NaN";
		else
			primary_tgt_text = "Target Metrics \n Az: " + QString::number(az_value) + "\n El " + QString::number(el_value);

		p3.drawText(frame.rect(), Qt::AlignTop | Qt::AlignLeft, primary_tgt_text);
	}

	// ---------------------------------------------------------------------------------------

	// Draw annotations
	
	int num_annotations = annotation_list.size();

	// if there are annotations ...
	if (num_annotations > 0) {

		// for each annotation ...
		for (int i = 0; i < num_annotations; i++) {

			// get frame information
			annotation_info a = annotation_list[i];
			int initial_frame_annotation = a.frame_start - a.min_frame;
			int last_frame_annotation = initial_frame_annotation + a.num_frames;
			
			// check that current frame is within bounds
			if (counter >= initial_frame_annotation && counter < last_frame_annotation) {

				QString annotation_color = a.color;
				int font_size = a.font_size;
				QString annotation_text = a.text;

				std::vector<int> loc = get_position_within_zoom(a.x_pixel, a.y_pixel);
				int x = loc[0];
				int y = loc[1];

				if (loc[0] >= 0)
				{
					// write text
					QPainter p_a(&frame);
					p_a.setPen(QPen(annotation_color));
					p_a.setFont(QFont("Times", font_size));
					p_a.drawText(x, y, annotation_text);
				}
			}
		}

	}
	// ---------------------------------------------------------------------------------------

	// Draw banner text
	QPainter p1(&frame);
	p1.setPen(QPen(banner_color));
	p1.setFont(QFont("Times", 12, QFont::Bold));
	p1.drawText(frame.rect(), Qt::AlignTop | Qt::AlignHCenter, banner_text);
	p1.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignHCenter, banner_text);

	bool video_open;
	if (record_frame && video_frame_number != counter) {
		video_frame_number = counter;
		add_new_frame(frame, CV_8UC3);
	}

	label->setPixmap(QPixmap::fromImage(frame));

	
	label->update();
	label->repaint();

	//counter++;
}


void Video::set_frame_data(std::vector<Plotting_Frame_Data>& input_data)
{
	display_data = input_data;
}

bool Video::start_recording(double fps)
{
	
	QString file_name = QFileDialog::getSaveFileName(this, "Save File", "","Video (*.avi)");
	
	if (file_name.isEmpty())
		return false;
	
	std::string file_string = file_name.toLocal8Bit().constData();
	
	video.open(file_string, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(image_x, image_y));

	bool video_opened = video.isOpened();
	
	if (video_opened)
		record_frame = true;

	return video_opened;
}

void Video::add_new_frame(QImage &img, int format)
{
	QImage image = img.rgbSwapped();
	cv::Mat output_frame(image.height(), image.width(),	format, image.bits(), image.bytesPerLine());
	video.write(output_frame);
}

void Video::stop_recording()
{
	video.release();
	record_frame = false;
}

void Video::save_frame()
{
	const QPixmap* pix = label->pixmap();
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "untitled.png", tr("Images (*.png)"));

	if (!fileName.isEmpty()) {
		//pix->save(fileName);
		frame.save(fileName);
	}
}

void Video::remove_frame()
{
	delete label;

	label = new EnhancedLabel(this);
	//label->update();
	label->repaint();

	histogram_plot->remove_histogram_plots();
	histogram_plot->initialize_histogram_plot();

	frame_data.clear();
	number_of_frames = 0;

	image_x = 0;
	image_y = 0;
	number_pixels = image_x * image_y;

}

void Video::update_specific_frame(unsigned int frame_number)
{
	counter = frame_number;
	update_display_frame();
}

/*
void Video::update_color_correction(double lift, double gamma, double gain)
{
	color_correction.set_min(lift);
	color_correction.set_max(gamma);
	color_correction.set_gain(gain);
}
*/

void Video::update_color_correction(double new_min_value, double new_max_value)
{
	color_correction.set_min(new_min_value);
	color_correction.set_max(new_max_value);
	update_display_frame();

}
