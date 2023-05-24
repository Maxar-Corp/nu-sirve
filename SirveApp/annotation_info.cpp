#include "annotation_info.h"

annotation_info create_annotation_info_from_json(const QJsonObject & json_obj)
{
    annotation_info temp = {};
    temp.text = json_obj.value("text").toString();
    temp.color = json_obj.value("color").toString();
    temp.x_pixel = json_obj.value("x_pixel").toInt();
    temp.y_pixel = json_obj.value("y_pixel").toInt();
    temp.frame_start = json_obj.value("frame_start").toInt();
    temp.num_frames = json_obj.value("num_frames").toInt();
    temp.font_size = json_obj.value("font_size").toInt();
    temp.min_frame = json_obj.value("min_frame").toInt();
    temp.max_frame = json_obj.value("max_frame").toInt();
    temp.x_min_position = json_obj.value("x_min_position").toInt();
    temp.x_max_position = json_obj.value("x_max_position").toInt();
    temp.y_min_position = json_obj.value("y_min_position").toInt();
    temp.y_max_position = json_obj.value("y_max_position").toInt();
    return temp;
}