#include "processing_state.h"

processingState create_processing_state_from_json(const QJsonObject & json_obj)
{
    QString method = json_obj.value("method").toString();

    if (method == "Original")
    {
        processingState temp = { ProcessingMethod::original };
        std::vector<unsigned int> replaced_pixels;
        for (auto json_item : json_obj.value("replaced_pixels").toArray())
        {
            replaced_pixels.push_back(json_item.toInt());
        }
        temp.replaced_pixels = replaced_pixels;
        return temp;
    }
    if (method == "ANS")
    {
        processingState temp = { ProcessingMethod::adaptive_noise_suppression };
        temp.ANS_relative_start_frame = json_obj.value("ANS_relative_start_frame").toInt();
        temp.ANS_num_frames = json_obj.value("ANS_num_frames").toInt();
        temp.ANS_shadow_threshold = json_obj.value("ANS_shadow_threshold").toInt();
		temp.ANS_hide_shadow = json_obj.value("ANS_hide_shadow").toBool();
        return temp;
    }
    if (method == "Deinterlace")
    {
        processingState temp = { ProcessingMethod::deinterlace };
        temp.deint_type = static_cast<DeinterlaceType>(json_obj.value("deint_type").toInt());
        return temp;
    }
    if (method == "FNS")
    {
        processingState temp = { ProcessingMethod::fixed_noise_suppression };
        temp.FNS_start_frame = json_obj.value("FNS_start_frame").toInt();
        temp.FNS_stop_frame = json_obj.value("FNS_stop_frame").toInt();
        temp.FNS_file_path = json_obj.value("FNS_file_path").toInt();
        return temp;
    }
     if (method == "Center on OSM")
    {
        processingState temp = { ProcessingMethod::center_on_OSM };
        temp.track_id = json_obj.value("Track_ID").toInt();
        return temp;
    }

    throw "Unexpected";
}