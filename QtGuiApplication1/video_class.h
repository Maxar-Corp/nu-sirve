#pragma once

#ifndef VIDEO_CLASS_H
#define VIDEO_CLASS_H

#include <iostream>
#include <string>
#include <math.h>

//#include <QObject>
#include <QString>
#include <QTimer>
#include <QLabel>
#include <vector>
#include <math.h>
#include <qpainter.h>
#include <qbrush.h>

#include "abir_reader.h"
#include "color_correction_lgg.h"
#include "video_container.h"

class Video : public QWidget
{
    Q_OBJECT
public:
   
	Video(int x_pixles, int y_pixels);
	Video(std::vector<uint8_t*> &video_data, int x_pixels, int y_pixels);
	~Video();

    unsigned int number_of_frames;
	int timer_frequency;
    QLabel  *label;
    QLabel *text;
	Lift_Gamma_Gain color_correction;
	
	QString banner_text, boresight_text;
	QColor banner_color, boresight_color;
	QVector<QRgb> colorTable;
	
	bool plot_boresight;


public slots:
    void update_frame();	
	void update_specific_frame(unsigned int frame_number);
	void update_color_correction(double lift, double gamma, double gain);
	void update_video_file(std::vector<uint8_t*> &video_data, int x_pixels, int y_pixels);
	void receive_video_data(video_details &new_input);
	void update_banner_text(QString input_banner_text, QColor input_banner_color);

private:

	int image_x, image_y, number_pixels;

	std::vector<uint8_t*> frame_data;
    unsigned int counter;
};


#endif // VIDEO_CLASS_H
