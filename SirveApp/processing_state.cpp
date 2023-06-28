#include "processing_state.h"

processing_state create_processing_state_from_json(const QJsonObject & json_obj)
{
    QString method = json_obj.value("method").toString();

    if (method == "Original")
    {
        return processing_state { Processing_Method::original };
    }
    if (method == "Background Subtraction")
    {
        processing_state temp = { Processing_Method::background_subtraction };
        temp.bgs_relative_start_frame = json_obj.value("bgs_relative_start_frame").toInt();
        temp.bgs_num_frames = json_obj.value("bgs_num_frames").toInt();
        return temp;
    }
    if (method == "Deinterlace")
    {
        processing_state temp = { Processing_Method::deinterlace };
        temp.deint_type = static_cast<deinterlace_type>(json_obj.value("deint_type").toInt());
        return temp;
    }
    if (method == "NUC")
    {
        processing_state temp = { Processing_Method::non_uniformity_correction };
        temp.nuc_start_frame = json_obj.value("nuc_start_frame").toInt();
        temp.nuc_stop_frame = json_obj.value("nuc_stop_frame").toInt();
        temp.nuc_file_path = json_obj.value("nuc_file_path").toInt();
        return temp;
    }
    if (method == "Bad Pixel Identification")
    {
        return processing_state { Processing_Method::bad_pixel_identification };
    }

    throw "Unexpected";
}