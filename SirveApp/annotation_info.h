#pragma once

#ifndef ANNOTATION_INFO_H
#define ANNOTATION_INFO_H

#include <QJsonObject>

struct annotation_info {
	int x_pixel, y_pixel, frame_start, num_frames, font_size;
	int min_frame, max_frame, x_min_position, x_max_position, y_min_position, y_max_position;
	QString text, color;

    QJsonObject to_json()
    {
        QJsonObject anno_object;
        anno_object.insert("color", color);
        anno_object.insert("text", text);
        anno_object.insert("min_frame", min_frame);
        anno_object.insert("max_frame", max_frame);
        anno_object.insert("x_min_position", x_min_position);
        anno_object.insert("x_max_position", x_max_position);
        anno_object.insert("y_min_position", y_min_position);
        anno_object.insert("y_max_position", y_max_position);
        anno_object.insert("x_pixel", x_pixel);
        anno_object.insert("y_pixel", y_pixel);
        anno_object.insert("frame_start", frame_start);
        anno_object.insert("num_frames", num_frames);
        anno_object.insert("font_size", font_size);
        return anno_object;
    }
};

annotation_info create_annotation_info_from_json(const QJsonObject & json_obj);

#endif