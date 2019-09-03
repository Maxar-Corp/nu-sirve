#include "video_class.h"


Video::Video(int x_pixels, int y_pixels, int input_bit_level)
{
	label = new QLabel(this);
	text = new QLabel(this);

	counter = 0;
	counter_record = 0;
	record_frame = false;

	image_x = x_pixels;
	image_y = y_pixels;
	number_pixels = image_x * image_y;
	max_bit_level = input_bit_level;

	banner_text = QString("Insert Banner");
	banner_color = QColor("Red");
	plot_boresight = false;
	display_boresight_txt = false;
	display_tgt_pos_txt = false;

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);

	for (int i = 0; i < 255; i++)
		colorTable.push_back(qRgb(255 - i, i, i));
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
	delete text;
}

void Video::update_video_file(std::vector<std::vector<uint16_t>>& video_data, int x_pixels, int y_pixels)
{
	
	frame_data = video_data;
	number_of_frames = video_data.size();
	
	image_x = x_pixels;
	image_y = y_pixels;
	number_pixels = image_x * image_y;
	
	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);
}

void Video::receive_video_data(video_details &new_input)
{
	update_video_file(new_input.frames_16bit, new_input.x_pixels, new_input.y_pixels);

}

void Video::update_banner_text(QString input_banner_text, QColor input_banner_color)
{
	banner_text = input_banner_text;
	banner_color = input_banner_color;

}

void Video::toggle_osm_tracks()
{
	if (plot_boresight)
		plot_boresight = false;
	else
		plot_boresight = true;
}

void Video::toggle_primary_track_data()
{
	if (display_tgt_pos_txt)
		display_tgt_pos_txt = false;
	else
		display_tgt_pos_txt = true;
}

void Video::toggle_sensor_boresight_data()
{
	if (display_boresight_txt)
		display_boresight_txt = false;
	else
		display_boresight_txt = true;
}

void Video::update_display_frame()
{
	//Convert current frame to armadillo matrix
	std::vector<double> frame_vector(frame_data[counter].begin(), frame_data[counter].end());
	arma::vec temp(frame_vector);
	arma::mat frame_matrix(temp);
	frame_matrix.reshape(image_x, image_y);
	frame_matrix = frame_matrix.t();

	int max_value = std::pow(2, max_bit_level);
	double normalized_min_value, normalized_max_value;
	
	arma::mat color_corrected_matrix = color_correction.get_updated_color(frame_matrix, max_value, normalized_min_value, normalized_max_value);

	color_corrected_matrix = color_corrected_matrix * 255;

	//------------------------------------------------------------------------------------------------
	color_corrected_matrix = color_corrected_matrix.t();
	arma::vec out_frame_flat = arma::vectorise(color_corrected_matrix);
	std::vector<double>out_vector = arma::conv_to<std::vector<double>>::from(out_frame_flat);


	std::vector<uint8_t> converted_values(out_vector.begin(), out_vector.end());

	uint8_t *color_corrected_frame = converted_values.data();// new uint8_t[number_pixels];
	//double updated_value;

	//for (int i = 0; i < number_pixels; i++)
	//{
	//	updated_value = color_correction.get_updated_color(frame_data[counter][i], 256);
	//	color_corrected_frame[i] = (uint8_t)updated_value;
	//}

	QImage frame((uchar *)color_corrected_frame, image_x, image_y, QImage::Format_Grayscale8);

	frame.setColorTable(colorTable);
	frame = frame.convertToFormat(QImage::Format_RGB888);

	if (plot_boresight & display_data[counter].ir_data.size() > 0) {

		QPainter rectangle_painter(&frame);

		int x_pixel = display_data[counter].ir_data[0].centroid_x;
		int y_pixel = display_data[counter].ir_data[0].centroid_y;

		int x_center = image_x / 2 + x_pixel;
		int y_center = image_y / 2 + y_pixel;

		int box_size = 5;
		QRectF rectangle(x_center - box_size, y_center - box_size, box_size * 2, box_size * 2);

		rectangle_painter.setPen(QPen(Qt::red));
		rectangle_painter.drawRect(rectangle);
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

	//delete[] color_corrected_frame;

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);
	text->update();

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

	if (!fileName.isEmpty())
		pix->save(fileName);
}

void Video::update_specific_frame(unsigned int frame_number)
{
	counter = frame_number;
	update_display_frame();
}

void Video::update_color_correction(double lift, double gamma, double gain)
{
	color_correction.set_lift(lift);
	color_correction.set_gamma(gamma);
	color_correction.set_gain(gain);
}
