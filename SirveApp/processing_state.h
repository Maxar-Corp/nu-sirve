#pragma once
#ifndef PROCESSING_STATE_H
#define PROCESSING_STATE_H

#include "video_details.h"

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

enum struct ProcessingMethod
{
	original,
    replace_bad_pixels,
	adaptive_noise_suppression,
	fixed_noise_suppression,
    RPCP_noise_suppression,
    accumulator_noise_suppression,
    deinterlace,
    center_on_OSM,
    center_on_manual,
    center_on_brightest,
    frame_stacking,
    state_description,
    descendants,
    ancestors,
    state_steps
};

struct processingState {
    ProcessingMethod method;
    VideoDetails details;

    //NOTE: This is a poor implementation of "polymorphic" configuration but can be cleaned up in a future refactor
    //These fields will only contain a value if the Processing_Method field is set to indicate they should
    //The burden is on consumers of the processingState struct to correctly interpret the fields
    int source_state_ID;
    int state_ID;
    
    std::vector<unsigned int> replaced_pixels;
    std::vector<unsigned int> ancestors;
    std::vector<unsigned int> descendants;

    int ANS_relative_start_frame;
    int ANS_num_frames;

    double weight;
    int offset;
    int shadow_threshold;
    bool hide_shadow;

    QString FNS_file_path;
    int FNS_start_frame;
    int FNS_stop_frame;
    int frame0;

    int frame_stack_num_frames;

	std::vector<std::vector<int>> offsets;
    int track_id;
    bool find_any_tracks;

    QString state_description;

    QString state_steps;
    QString process_steps;

    QString get_friendly_description() {
       switch (method)
        {
            case ProcessingMethod::original:
                return "Original ('Raw') Data";
                break;
            case ProcessingMethod::replace_bad_pixels:
                return "Replaced Bad Pixels\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +QString::number(replaced_pixels.size()) + " bad pixels replaced.\n"+\
                    "State steps: " + state_steps;
                break;
            case ProcessingMethod::adaptive_noise_suppression:{             
                return "Adaptive Noise Suppression\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"ANS: from " + QString::number(ANS_relative_start_frame) + ", averaging " + QString::number(ANS_num_frames)+" frames.\n"\
                    +"State steps: " + state_steps;
                break;
            }
            case ProcessingMethod::fixed_noise_suppression:{
                return "Fixed Noise Suppression\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"FNS: " + FNS_file_path + " Frames " + QString::number(FNS_start_frame) + " to " + QString::number(FNS_stop_frame)+".\n"\
                    +"State steps: " + state_steps;
                break;
            }
            case ProcessingMethod::RPCP_noise_suppression:{
                return "RPCP Noise Suppression\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"State steps: " + state_steps;
                break;
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
                break;
            }
            case ProcessingMethod::deinterlace:{
                return "Deinterlacing\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"State steps: " + state_steps;
                break;
            }
            case ProcessingMethod::center_on_OSM:{
                QString trackid;
                if (track_id <0){
                    trackid = "Primary Track";
                }
                else{
                    trackid = QString::number(track_id);
                }
                QString boolString = find_any_tracks ? "true" : "false";
                return "Centered on OSM\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"Centered on OSM: " + trackid + ". Find any tracks set to " + boolString +".\n"\
                    +"State steps: " + state_steps;
                break;
            }
            case ProcessingMethod::center_on_manual:{
                QString trackid;
                if (track_id <0){
                    trackid = "Primary Track";
                }
                else{
                    trackid = QString::number(track_id);
                }
                QString boolString = find_any_tracks ? "true" : "false";
                return "Centered on Manual\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"Centered on OSM: " + trackid + ". Find any tracks set to " + boolString +".\n"\
                    +"State steps: " + state_steps;
                break;
            }
            case ProcessingMethod::center_on_brightest:{
                return "Centered on Brightest\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"State steps: " + state_steps;
                break;
            }
            case ProcessingMethod::frame_stacking:{
                return "Frame Stacking\n<Previous State " + QString::number(source_state_ID) + ">\n"\
                    +"Process steps: " + process_steps +"\n"\
                    +"Frame Stack: Averaging " + QString::number(frame_stack_num_frames)+".\n"\
                    +"State steps: " + state_steps;
                break;
            }   
            default:
                return "Unknown";
                break;
        }
    };

    QString get_combobox_description() {
       switch (method)
        {
            case ProcessingMethod::original:
                return "Original ('Raw') Data";
                break;
            case ProcessingMethod::replace_bad_pixels:
                return "<Previous State " + QString::number(source_state_ID) + "> Replace Bad Pixels";;
                break;
            case ProcessingMethod::adaptive_noise_suppression:
                return "<Previous State " + QString::number(source_state_ID) + "> ANS";
                break;
            case ProcessingMethod::fixed_noise_suppression:
                //may potentially want to leave fns_file_path empty if it isn't an external file?
                return "<Previous State " + QString::number(source_state_ID) + "> FNS " ;
                break;
            case ProcessingMethod::RPCP_noise_suppression:
                return "<Previous State " + QString::number(source_state_ID) + "> RPCP";
                break;
            case ProcessingMethod::accumulator_noise_suppression:
                if (hide_shadow){
                    return "<Previous State " + QString::number(source_state_ID) + "> Rolling Mean Hide Shadow";
                }
                else{
                     return "<Previous State " + QString::number(source_state_ID) + "> Rolling Mean";
                }
                break;
            case ProcessingMethod::deinterlace:
                return "<Previous State " + QString::number(source_state_ID) + "> Deinterlace" ;
                break;
            case ProcessingMethod::center_on_OSM:
                return "<Previous State " + QString::number(source_state_ID) + "> Centered on OSM";
                break;
            case ProcessingMethod::center_on_manual:
                return "<Previous State " + QString::number(source_state_ID) + "> Centered on Manual";
                break;
            case ProcessingMethod::center_on_brightest:
                return "<Previous State " + QString::number(source_state_ID) + "> Centered on Brightest";
                break;
            case ProcessingMethod::frame_stacking:
                return "<Previous State " + QString::number(source_state_ID) + "> Frame Stack Averaging";
                break;
            default:
                return "Unknown";
                break;
        }
    };

    QJsonObject to_json() {
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
    };
};

processingState create_processing_state_from_json(const QJsonObject & json_obj);

#endif
