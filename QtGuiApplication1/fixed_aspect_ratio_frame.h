#pragma once
#ifndef FIXED_ASPECT_RATIO_FRAME_H
#define FIXED_ASPECT_RATIO_FRAME_H

#include <QFrame>
#include <QMouseEvent> 


class FixedAspectRatioFrame : public QFrame
{
	Q_OBJECT
	public:
		FixedAspectRatioFrame(QWidget* parent = nullptr);
		~FixedAspectRatioFrame();

		int aspect_ratio_width, aspect_ratio_height;
		bool fixed_aspect_ratio;

		void set_aspect_ratio(int width, int height);
		void enable_fixed_aspect_ratio(bool enable);

	protected:
		void resizeEvent(QResizeEvent* event);

	signals:
		void resize_window();


};


#endif // FIXED_ASPECT_RATIO_FRAME_H