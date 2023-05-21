#pragma once

#ifndef ANNOTATION_INFO_H
#define ANNOTATION_INFO_H

struct annotation_info {
	int x_pixel, y_pixel, frame_start, num_frames, font_size;
	int min_frame, max_frame, x_min_position, x_max_position, y_min_position, y_max_position;
	QString text, color;
};

#endif