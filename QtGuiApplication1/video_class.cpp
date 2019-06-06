#include "video_class.h"


Video::Video(int x_pixels, int y_pixels)
{
	label = new QLabel(this);
	text = new QLabel(this);

	counter = 0;

	image_x = x_pixels;
	image_y = y_pixels;
	number_pixels = image_x * image_y;

	banner_text = QString("Insert Banner Here");
	banner_color = QColor("Red");
	plot_boresight = true;

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);

	for (int i = 0; i < 255; i++)
		colorTable.push_back(qRgb(255 - i, i, i));
}

Video::Video(std::vector<uint8_t*> &video_data, int x_pixels, int y_pixels)
{
	label = new QLabel(this);
	text = new QLabel(this);

	counter = 0;
	
	update_video_file(video_data, x_pixels, y_pixels);
}

Video::~Video()
{
	delete  label;
	delete text;
}

void Video::update_video_file(std::vector<uint8_t*>& video_data, int x_pixels, int y_pixels)
{
	
	frame_data = video_data;
	number_of_frames = video_data.size();
	
	image_x = x_pixels;
	image_y = y_pixels;
	number_pixels = image_x * image_y;
	//update_frame();

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);
}

void Video::receive_video_data(video_details &new_input)
{
	update_video_file(new_input.frames_8bit, new_input.x_pixels, new_input.y_pixels);

}

void Video::update_banner_text(QString input_banner_text, QColor input_banner_color)
{
	banner_text = input_banner_text;
	banner_color = input_banner_color;

}

void Video::update_frame()
{

	// Takes ~ 35-40 ms to run which equates to a max update rate of 25 fps

	if (counter == number_of_frames)
		counter = 0;

	uint8_t *color_corrected_frame = new uint8_t[number_pixels];
	double updated_value;

	for (int i = 0; i < number_pixels; i++)
	{
		updated_value = color_correction.get_updated_color(frame_data[counter][i], 256);
		color_corrected_frame[i] = (uint8_t)updated_value;
	}

	QImage frame((uchar *)color_corrected_frame, image_x, image_y, QImage::Format_Grayscale8);

	//QImage image_banners(image_x, image_y, QImage::Format_ARGB32);
	//image_banners.fill(qRgba(0, 0, 0, 0));
	
	frame.setColorTable(colorTable);
	frame = frame.convertToFormat(QImage::Format_RGB30);

	QPainter p(&frame);
	p.setPen(QPen(banner_color));
	p.setFont(QFont("Times", 12, QFont::Bold));
	p.drawText(frame.rect(), Qt::AlignTop | Qt::AlignHCenter, banner_text);
	p.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignHCenter, banner_text);

	if (plot_boresight) {

		QPainter rectangle_painter(&frame);
		QRectF rectangle(10.0, 20.0, 80.0, 60.0);

		rectangle_painter.setPen(QPen(Qt::red));
		//rectangle_painter.setBrush(QBrush(QColor(Qt::red)));
		rectangle_painter.drawRect(rectangle);
		//rectangle_painter.drawLine(5, 5, 50, 50);
		//rectangle_painter.end();
	}


	label->setPixmap(QPixmap::fromImage(frame));
	label->update();
	label->repaint();

	delete[] color_corrected_frame;

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);
	text->update();

	//counter++;
}

void Video::update_specific_frame(unsigned int frame_number)
{
	counter = frame_number;
	update_frame();
}

void Video::update_color_correction(double lift, double gamma, double gain)
{
	color_correction.set_lift(lift);
	color_correction.set_gamma(gamma);
	color_correction.set_gain(gain);
}
