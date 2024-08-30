#include "processing_state.h"
#include <armadillo>

processingState create_processing_state_from_json(const QJsonObject & json_obj)
{
    QString method = json_obj.value("method").toString();

    if (method == "Original")
    {
        processingState temp = { ProcessingMethod::original };
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        return temp;
    }
    if (method == "Replace Bad Pixels")
    {
        processingState temp = { ProcessingMethod::replace_bad_pixels };
        std::vector<unsigned int> replaced_pixels;
        for (auto json_item : json_obj.value("replaced_pixels").toArray())
        {
            replaced_pixels.push_back(json_item.toInt());
        }
        temp.replaced_pixels = replaced_pixels;
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        return temp;
    }
    if (method == "ANS")
    {
        processingState temp = { ProcessingMethod::adaptive_noise_suppression };
        temp.ANS_relative_start_frame = json_obj.value("ANS_relative_start_frame").toInt();
        temp.ANS_num_frames = json_obj.value("ANS_num_frames").toInt();
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }
    if (method == "Deinterlace")
    {
        processingState temp = { ProcessingMethod::deinterlace };
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }
    if (method == "FNS")
    {
        processingState temp = { ProcessingMethod::fixed_noise_suppression };
        temp.frame0 = json_obj.value("FNS_frame0").toInt();
        temp.FNS_start_frame = json_obj.value("FNS_start_frame").toInt();
        temp.FNS_stop_frame = json_obj.value("FNS_stop_frame").toInt();
        temp.FNS_file_path = json_obj.value("FNS_file_path").toString();
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }
    if (method == "Accumulator")
    {
        processingState temp = { ProcessingMethod::accumulator_noise_suppression };
        temp.weight = json_obj.value("weight").toDouble();
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        temp.shadow_threshold = json_obj.value("shadow_threshold").toInt();
        temp.hide_shadow = json_obj.value("hide_shadow").toBool();
        temp.offset = json_obj.value("offset").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }
     if (method == "Center on OSM")
    {
        processingState temp = { ProcessingMethod::center_on_OSM };
        temp.track_id = json_obj.value("Track_ID").toInt();
        std::vector<int> offsets0;
        std::vector<std::vector<int>> offsets;
        for (auto json_item : json_obj.value("offsets").toArray()){          
            offsets0.push_back(json_item.toInt());
        }
        arma::vec tmpvec = arma::conv_to<arma::vec>::from(offsets0);
        int numRows = tmpvec.n_elem/3;
        arma::mat tmpmat = arma::reshape(tmpvec,3,numRows).t();
        for (int i = 0; i<numRows; i++){
            offsets.push_back(arma::conv_to<std::vector<int>>::from(tmpmat.row(i)));
        }
        temp.offsets = offsets;
        temp.find_any_tracks = json_obj.value("find_any_tracks").toBool();
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }

    if (method == "Center on Manual")
    {
        processingState temp = { ProcessingMethod::center_on_manual };
        temp.track_id = json_obj.value("Track_ID").toInt();
        std::vector<int> offsets0;
        std::vector<std::vector<int>> offsets;

        for (auto json_item : json_obj.value("offsets").toArray()){          
            offsets0.push_back(json_item.toInt());
        }

        arma::vec tmpvec = arma::conv_to<arma::vec>::from(offsets0);
        int numRows = tmpvec.n_elem/3;
        arma::mat tmpmat = arma::reshape(tmpvec,3,numRows).t();
        for (int i = 0; i<numRows; i++){
            offsets.push_back(arma::conv_to<std::vector<int>>::from(tmpmat.row(i)));
        }
        temp.offsets = offsets;
        temp.find_any_tracks = json_obj.value("find_any_tracks").toBool();
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }

    if (method == "Center on Brightest")
    {
        processingState temp = { ProcessingMethod::center_on_brightest };
        temp.track_id = json_obj.value("Track_ID").toInt();
        std::vector<int> offsets0;
        std::vector<std::vector<int>> offsets;

        for (auto json_item : json_obj.value("offsets").toArray()){          
            offsets0.push_back(json_item.toInt());
        }

        arma::vec tmpvec = arma::conv_to<arma::vec>::from(offsets0);
        int numRows = tmpvec.n_elem/3;
        arma::mat tmpmat = arma::reshape(tmpvec,3,numRows).t();
        for (int i = 0; i<numRows; i++){
            offsets.push_back(arma::conv_to<std::vector<int>>::from(tmpmat.row(i)));
        }
        temp.offsets = offsets;
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }

    if (method == "Frame Stacking")
    {
        processingState temp = { ProcessingMethod::frame_stacking };
        temp.frame_stack_num_frames = json_obj.value("frame_stack_num_frames").toInt();
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }
    if (method == "RPCP")
    {
        processingState temp = { ProcessingMethod::RPCP_noise_suppression };
        temp.state_ID = json_obj.value("state_ID").toInt();
        temp.source_state_ID = json_obj.value("source_state_ID").toInt();
        std::vector<unsigned int>  ancestors;
        for (auto json_item : json_obj.value("ancestors").toArray())
        {
            ancestors.push_back(json_item.toInt());
        }
        temp.ancestors = ancestors;
        std::vector<unsigned int>  descendants;
        for (auto json_item : json_obj.value("descendants").toArray())
        {
            descendants.push_back(json_item.toInt());
        }
        temp.descendants = descendants;
        return temp;
    }
    throw "Unexpected";
}