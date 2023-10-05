#include "video_display_zoom.h"

VideoDisplayZoomManager::VideoDisplayZoomManager(int x_pixels, int y_pixels)
{
    image_x = x_pixels;
    image_y = y_pixels;
	QRect new_zoom(0, 0, image_x, image_y);
	zoom_list.push_back(new_zoom);
	absolute_zoom_list.push_back(absolute_zoom_info {0, 0, 1.0*image_x, 1.0*image_y});
}

VideoDisplayZoomManager::~VideoDisplayZoomManager()
{
}

void VideoDisplayZoomManager::zoom_image(QRect area)
{
    // if width/heigh is less than 10 pixels long, then this was not a zoomable area and return
    if (area.width() < 10 || area.height() < 10)
    {
        return;
    }

    double height = area.height();
    double width = area.width();

    double x = area.x();
    double y = area.y();

    double aspect_ratio = 1.0 * image_x / image_y;
    double adj_width = height * aspect_ratio;
    double adj_height = width / aspect_ratio;

    // adjust size of box to fit aspect ratio and encompass the highlighted area selected
    if (adj_width > width) {

        width = adj_width;
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
        double delta = x + width - (1.0 * image_x);
        x = x - delta;
    }

    // if updated area exceeds height, move origin up
    if (y + height > image_y)
    {
        double delta = y + height - (1.0 * image_y);
        y = y - delta;
    }

    x = std::round(x);
    y = std::round(y);
    width = std::round(width);
    height = std::round(height);

	QRect new_zoom(x, y, width, height);
	zoom_list.push_back(new_zoom);

	absolute_zoom_info starting_rectangle = absolute_zoom_list[absolute_zoom_list.size() - 1];
	double absolute_x = starting_rectangle.x + x * starting_rectangle.width / image_x;
	double absolute_y = starting_rectangle.y + y * starting_rectangle.height / image_y;
	double absolute_width = starting_rectangle.width * (1.0 * width / image_x);
	double absolute_height = starting_rectangle.height * (1.0 * height / image_y);
	absolute_zoom_list.push_back(absolute_zoom_info {absolute_x, absolute_y, absolute_width, absolute_height});
}

void VideoDisplayZoomManager::unzoom()
{
    zoom_list.pop_back();
    absolute_zoom_list.pop_back();
}

bool VideoDisplayZoomManager::is_any_piece_within_zoom(int x0, int y0)
{
	absolute_zoom_info final_zoom_level = absolute_zoom_list[absolute_zoom_list.size() - 1];

	if (x0 + 1 < final_zoom_level.x)
		return false;

	if (x0 > final_zoom_level.x + final_zoom_level.width)
		return false;

	if (y0 + 1 < final_zoom_level.y)
		return false;

	if (y0 > final_zoom_level.y + final_zoom_level.height)
		return false;

	return true;
}

std::vector<int> VideoDisplayZoomManager::get_position_within_zoom(int x0, int y0)
{
	size_t num_zooms = zoom_list.size();
	bool pt_within_area = true;

	int x_center = x0;
	int y_center = y0;

	//for each zoom level ...
	for (auto j = 0; j < num_zooms; j++)
	{
		//define object location relative to zoom frame
		QRect sub_frame = zoom_list[j];
		x_center = x_center - sub_frame.x();
		y_center = y_center - sub_frame.y();

		//if object location exceeds frame, stop and prevent drawing
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

		//rescale pixels to image frame
		double temp_x = x_center * 1.0 / sub_frame.width() * image_x;
		double temp_y = y_center * 1.0 / sub_frame.height() * image_y;

		x_center = std::round(temp_x);
		y_center = std::round(temp_y);

	}

	//if point is within all zoom frames ...
	if (pt_within_area)
	{
		return std::vector<int> {x_center, y_center};
	}
	else
		return std::vector<int> {-1, -1};
}

bool VideoDisplayZoomManager::is_currently_zoomed()
{
    return zoom_list.size() > 1;
}