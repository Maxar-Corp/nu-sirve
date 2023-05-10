#include "fixed_aspect_ratio_frame.h"

FixedAspectRatioFrame::FixedAspectRatioFrame(QWidget* parent) :QFrame(parent)
{

	aspect_ratio_width = 4;
	aspect_ratio_height = 3;
	fixed_aspect_ratio = false;

}

FixedAspectRatioFrame::~FixedAspectRatioFrame()
{

}

void FixedAspectRatioFrame::set_aspect_ratio(int width, int height)
{
	aspect_ratio_width = width;
	aspect_ratio_height = height;
}

void FixedAspectRatioFrame::enable_fixed_aspect_ratio(bool enable)
{

	fixed_aspect_ratio = enable;

}

void FixedAspectRatioFrame::resizeEvent(QResizeEvent* event)
{
	// for all resize events, adjust minimum width to match the new height

	if (fixed_aspect_ratio) {


		double ar = aspect_ratio_width * 1.0 / aspect_ratio_height;
		int new_width = ar * this->height();

		this->setMinimumWidth(new_width);

	}

}
