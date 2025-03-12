#include "processing_state.h"
#include <armadillo>
/**
 * \brief Creates a processing state from a JSON object.
 * \param json_obj The JSON object to create the processing state from.
 * \return The processing state created from the JSON object.
 */
processingState processingState::FromJson(const QJsonObject & json_obj)
{
    QString method = json_obj.value("method").toString();

    if (method == "Original ('Raw') Data")
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
        temp.frame0 = json_obj.value("frame0").toInt();
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



/**
 * \brief Updates the state_description and combobox_state_name fields based on state_ID and method.
 */
/**
 * \brief Returns a friendly description of the processing state, based on method.
 * \return A friendly description of the processing state.
 */
QString processingState::GetFriendlyDescription() const {
    switch (method)
    {
        case ProcessingMethod::original:
            return "Original ('Raw') Data";

        case ProcessingMethod::replace_bad_pixels:
            return "Replaced Bad Pixels\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +QString::number(replaced_pixels.size()) + " bad pixels replaced.\n"+\
                "State steps: " + state_steps;

        case ProcessingMethod::adaptive_noise_suppression:{
            return "Adaptive Noise Suppression\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +"ANS: from " + QString::number(ANS_relative_start_frame) + ", averaging " + QString::number(ANS_num_frames)+" frames.\n"\
                +"State steps: " + state_steps;

        }
        case ProcessingMethod::fixed_noise_suppression:{
            return "Fixed Noise Suppression\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +"FNS: " + FNS_file_path + " Frames " + QString::number(FNS_start_frame) + " to " + QString::number(FNS_stop_frame)+".\n"\
                +"State steps: " + state_steps;

        }
        case ProcessingMethod::RPCP_noise_suppression:{
            return "RPCP Noise Suppression\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +"State steps: " + state_steps;

        }
        case ProcessingMethod::accumulator_noise_suppression:{
            QString boolString = hide_shadow ? "true" : "false";
            if (hide_shadow){
                return "Rolling Mean Noise Suppression\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"Accumulator: Weight " + QString::number(weight) +"\n"\
                    +"Hide Shadow option set to " + boolString +". Shadow threshold set to " + QString::number(shadow_threshold)+".\n"\
                    +"Offset: " + QString::number(offset)+"\n"\
                    +"State steps: " + state_steps;
            }
            else{
                return "Rolling Mean Noise Suppression\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"Accumulator: Weight " + QString::number(weight) +"\n"\
                    +"State steps: " + state_steps;
            }

        }
        case ProcessingMethod::deinterlace:{
            return "Deinterlacing\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +"State steps: " + state_steps;

        }
        case ProcessingMethod::center_on_OSM:{
            QString trackid;
            if (track_id <0){
                trackid = "Primary";
            }
            else{
                trackid = QString::number(track_id);
            }
            QString boolString = find_any_tracks ? "true" : "false";
            return "Centered on OSM\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +"Centered on OSM Track: " + trackid + ". Find any tracks set to " + boolString +".\n"\
                +"State steps: " + state_steps;

        }
        case ProcessingMethod::center_on_manual:{
            QString trackid;
            if (track_id <0){
                trackid = "Primary";
            }
            else{
                trackid = QString::number(track_id);
            }
            QString boolString = find_any_tracks ? "true" : "false";
            return "Centered on Manual\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +"Centered on Manual Track: " + trackid + ". Find any tracks set to " + boolString +".\n"\
                +"State steps: " + state_steps;

        }
        case ProcessingMethod::center_on_brightest:{
            return "Centered on Brightest\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +"State steps: " + state_steps;

        }
        case ProcessingMethod::frame_stacking:{
            return "Frame Stacking\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                +"Process steps: " + process_steps +"\n"\
                +"Frame Stack: Averaging " + QString::number(frame_stack_num_frames)+".\n"\
                +"State steps: " + state_steps;

        }
        default:
            return "Unknown";       
    }
}

/**
 * \brief Returns a description of the processing state for use in a combobox, based on method.
 * \return A description of the processing state for use in a combobox.
 */
QString processingState::GetComboBoxDescription() const {
    switch (method)
    {
        case ProcessingMethod::original:
            return "Original ('Raw') Data";

        case ProcessingMethod::replace_bad_pixels:
            return "<Previous State " + QString::number(source_state_ID) + "> Replace Bad Pixels";;

        case ProcessingMethod::adaptive_noise_suppression:
            return "<Previous State " + QString::number(source_state_ID) + "> ANS";

        case ProcessingMethod::fixed_noise_suppression:
            //may potentially want to leave fns_file_path empty if it isn't an external file?
            return "<Previous State " + QString::number(source_state_ID) + "> FNS " ;

        case ProcessingMethod::RPCP_noise_suppression:
            return "<Previous State " + QString::number(source_state_ID) + "> RPCP";

        case ProcessingMethod::accumulator_noise_suppression:
            if (hide_shadow){
                return "<Previous State " + QString::number(source_state_ID) + "> Rolling Mean Hide Shadow";
            }
            else{
                return "<Previous State " + QString::number(source_state_ID) + "> Rolling Mean";
            }

        case ProcessingMethod::deinterlace:
            return "<Previous State " + QString::number(source_state_ID) + "> Deinterlace" ;

        case ProcessingMethod::center_on_OSM:
            return "<Previous State " + QString::number(source_state_ID) + "> Centered on OSM";

        case ProcessingMethod::center_on_manual:
            return "<Previous State " + QString::number(source_state_ID) + "> Centered on Manual";

        case ProcessingMethod::center_on_brightest:
            return "<Previous State " + QString::number(source_state_ID) + "> Centered on Brightest";

        case ProcessingMethod::frame_stacking:
            return "<Previous State " + QString::number(source_state_ID) + "> Frame Stack Averaging";

        default:
            return "Unknown";
    }
}

const QString& processingState::GetStepName() const
{
    static const QString step_names[] = {
        "Original ('Raw') Data",
        "Replace Bad Pixels",
        "RPCP Noise Suppression",
        "Rolling Mean Noise Suppression",
        "Adaptive Noise Suppression",
        "Deinterlace",
        "Fixed Noise Suppression",
        "Center on OSM",
        "Center on Manual",
        "Center on Brightest",
        "Frame Stacking"
    };

    return step_names[static_cast<int>(method)];
}

/**
 * \brief Returns a JSON object representing the processing state.
 * \return A JSON object representing the processing state.
 */
QJsonObject processingState::ToJson() const {
    QJsonObject state_object;
    state_object.insert("state_ID", state_ID);
    state_object.insert("source_state_ID", source_state_ID);
    QJsonArray jancestors;
    for (auto i = 0; i < ancestors.size(); i++)
    {
        jancestors.push_back(static_cast<int>(ancestors[i]));
    }
    state_object.insert("ancestors",jancestors);
    QJsonArray jdescendants;
    for (auto i = 0; i < descendants.size(); i++)
    {
        jdescendants.push_back(static_cast<int>(descendants[i]));
    }
    state_object.insert("descendants",jdescendants);
    state_object.insert("process_steps",process_steps);
    switch (method)
    {
        case ProcessingMethod::original:
            {
                state_object.insert("method", "Original ('Raw') Data");
                state_object.insert("state description",state_description);
                state_object.insert("state steps",state_steps);
                break;
            }
        case ProcessingMethod::replace_bad_pixels:
            {
                state_object.insert("method", "Replace Bad Pixels");
                QJsonArray pixels;
                for (auto i = 0; i < replaced_pixels.size(); i++)
                {
                    pixels.push_back(static_cast<int>(replaced_pixels[i]));
                }
                state_object.insert("replaced_pixels", pixels);
                state_object.insert("state description",state_description);
                state_object.insert("state steps",state_steps);
                break;
            }
        case ProcessingMethod::RPCP_noise_suppression:
            state_object.insert("method", "RPCP");
            state_object.insert("state description",state_description);
            state_object.insert("state steps",state_steps);
            break;
        case ProcessingMethod::accumulator_noise_suppression:
            state_object.insert("method", "Accumulator");
            state_object.insert("state description",state_description);
            state_object.insert("state steps",state_steps);
            state_object.insert("hide_shadow", hide_shadow);
            state_object.insert("shadow_threshold", shadow_threshold);
            state_object.insert("weight",weight);
            state_object.insert("offset",offset);
            break;
        case ProcessingMethod::adaptive_noise_suppression:
            state_object.insert("method", "ANS");
            state_object.insert("ANS_relative_start_frame", ANS_relative_start_frame);
            state_object.insert("ANS_num_frames", ANS_num_frames);
            state_object.insert("state description",state_description);
            state_object.insert("state steps",state_steps);
            break;
        case ProcessingMethod::deinterlace:
            state_object.insert("method", "Deinterlace");
            state_object.insert("state description",state_description);
            state_object.insert("state steps",state_steps);
            break;
        case ProcessingMethod::fixed_noise_suppression:
            state_object.insert("method", "FNS");
            state_object.insert("frame0",frame0);
            state_object.insert("FNS_start_frame", FNS_start_frame);
            state_object.insert("FNS_stop_frame", FNS_stop_frame);
            state_object.insert("FNS_file_path", FNS_file_path);
            state_object.insert("state description",state_description);
            state_object.insert("state steps",state_steps);
            break;
        case ProcessingMethod::center_on_OSM:
            {
                state_object.insert("method", "Center on OSM");
                state_object.insert("Track_ID", track_id);
                QJsonArray offsetsixy;
                for (auto i = 0; i < offsets.size(); i++){
                    for (auto j = 0; j < 3; j++){
                        offsetsixy.push_back(offsets[i][j]);
                    }
                }
                state_object.insert("offsets", offsetsixy);
                state_object.insert("Find_Any_Tracks",find_any_tracks);
                state_object.insert("state description",state_description);
                state_object.insert("state steps",state_steps);
                break;
            }
        case ProcessingMethod::center_on_manual:
            {
                state_object.insert("method", "Center on Manual");
                state_object.insert("Track_ID", track_id);
                QJsonArray offsetsixy;
                for (auto i = 0; i < offsets.size(); i++){
                    for (auto j = 0; j < 3; j++){
                        offsetsixy.push_back(offsets[i][j]);
                    }
                }
                state_object.insert("offsets", offsetsixy);
                state_object.insert("Find_Any_Tracks",find_any_tracks);
                state_object.insert("state description",state_description);
                state_object.insert("state steps",state_steps);
                break;
            }
        case ProcessingMethod::center_on_brightest:
            {
                state_object.insert("method", "Center on Brightest");
                state_object.insert("Track_ID", track_id);
                QJsonArray offsetsixy;
                for (auto i = 0; i < offsets.size(); i++){
                    for (auto j = 0; j < 3; j++){
                        offsetsixy.push_back(offsets[i][j]);
                    }
                }
                state_object.insert("offsets", offsetsixy);
                state_object.insert("state description",state_description);
                state_object.insert("state steps",state_steps);
                break;
            }
        case ProcessingMethod::frame_stacking:
            state_object.insert("method", "Frame Stacking");
            state_object.insert("frame_stack_num_frames", frame_stack_num_frames);
            state_object.insert("state description",state_description);
            state_object.insert("state steps",state_steps);
            break;
        default:
            state_object.insert("method", "error");
    }
    return state_object;
}

void processingState::UpdateDescription()
{
    state_description = "State " + QString::number(state_ID) + ": " + GetFriendlyDescription();
    combobox_state_name = combobox_state_name = QString::number(state_ID) + ": " + GetComboBoxDescription();
}

void processingState::UpdateMaxValue()
{
    auto maxVal = std::numeric_limits<uint16_t>::min();
    for (const auto& row : details.frames_16bit) {
        maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
    }
    details.max_value = maxVal;
}
