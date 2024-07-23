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
	adaptive_noise_suppression,
	fixed_noise_suppression,
    RPCP_noise_suppression,
    deinterlace,
    center_on_OSM,
    center_on_manual,
    center_on_brightest,
    frame_stacking,
    state_description,
    descendants,
    ancestors
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
    int ANS_shadow_threshold;
    bool ANS_hide_shadow;

    QString FNS_file_path;
    int FNS_start_frame;
    int FNS_stop_frame;

    int frame_stack_num_frames;

	std::vector<std::vector<int>> offsets;
    int track_id;
    bool find_any_tracks;

    QString state_description;

    QString get_friendly_description() {
       switch (method)
        {
            case ProcessingMethod::original:
                if (replaced_pixels.size() > 0)
                {
                    return "Original with replaced pixels";
                }
                else
                {
                    return "Original";
                }
                break;
            case ProcessingMethod::adaptive_noise_suppression:{
                QString boolString = ANS_hide_shadow ? "true" : "false";
                if (ANS_hide_shadow){
                    return "<Source State " + QString::number(source_state_ID) + "> ANS: from " + QString::number(ANS_relative_start_frame) + ", averaging " + QString::number(ANS_num_frames) + " frames. Hide Shadow option set to " + boolString + ". Shadow threshold set to " + QString::number(ANS_shadow_threshold) +".";
                }
                else
                {
                    return "<Source State " + QString::number(source_state_ID) + "> ANS: from " + QString::number(ANS_relative_start_frame) + ", averaging " + QString::number(ANS_num_frames) + " frames. Hide Shadow option set to " + boolString +".";
                }
                break;
            }
            case ProcessingMethod::fixed_noise_suppression:
                return "<Source State " + QString::number(source_state_ID) + "> FNS: " + QString::number(FNS_start_frame) + " to " + QString::number(FNS_stop_frame);
                break;
            case ProcessingMethod::RPCP_noise_suppression:
                return "<Source State " + QString::number(source_state_ID) + "> RPCP";
                break;
            case ProcessingMethod::deinterlace:
                return "<Source State " + QString::number(source_state_ID) + "> Deinterlace";
                break;
            case ProcessingMethod::center_on_OSM:{
                QString trackid;
                if (track_id <0){
                    trackid = "Primary Track";
                }
                else{
                    trackid = QString::number(track_id);
                }
                QString boolString = find_any_tracks ? "true" : "false";
                return "<Source State " + QString::number(source_state_ID) + "> Centered on OSM: " + trackid + " Find any tracks set to " + boolString +".";
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
                return "<Source State " + QString::number(source_state_ID) + "> Centered on Manual: " + trackid + " Find any tracks set to " + boolString +".";
                break;
            }
            case ProcessingMethod::center_on_brightest:
                return "<Source State " + QString::number(source_state_ID) + "> Centered on Brightest: " + QString::number(track_id);
                break;
            case ProcessingMethod::frame_stacking:
                return "<Source State " + QString::number(source_state_ID) + "> Frame Stack: Averaging " + QString::number(frame_stack_num_frames);
                break;
            default:
                return "Unknown";
                break;
        }
    };

    QString get_combobox_description() {
       switch (method)
        {
            case ProcessingMethod::original:
                if (replaced_pixels.size() > 0)
                {
                    return "Original with replaced pixels";
                }
                else
                {
                    return "Original";
                }
                break;
            case ProcessingMethod::adaptive_noise_suppression:
            if (ANS_hide_shadow){
                    return "<Source State " + QString::number(source_state_ID) + "> ANS Hide Shadow";
                }
                else
                {
                    return "<Source State " + QString::number(source_state_ID) + "> ANS";
                }
                break;
            case ProcessingMethod::fixed_noise_suppression:
                //may potentially want to leave fns_file_path empty if it isn't an external file?
                return "<Source State " + QString::number(source_state_ID) + "> FNS " ;
                break;
            case ProcessingMethod::RPCP_noise_suppression:
                return "<Source State " + QString::number(source_state_ID) + "> RPCP";
                break;
            case ProcessingMethod::deinterlace:
                return "<Source State " + QString::number(source_state_ID) + "> Deinterlace" ;
                break;
            case ProcessingMethod::center_on_OSM:
                return "<Source State " + QString::number(source_state_ID) + "> Centered on OSM";
                break;
            case ProcessingMethod::center_on_manual:
                return "<Source State " + QString::number(source_state_ID) + "> Centered on Manual";
                break;
            case ProcessingMethod::center_on_brightest:
                return "<Source State " + QString::number(source_state_ID) + "> Centered on Brightest";
                break;
            case ProcessingMethod::frame_stacking:
                return "<Source State " + QString::number(source_state_ID) + "> Frame Stack Averaging";
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
        switch (method)
        {
            case ProcessingMethod::original:
                {
                    state_object.insert("method", "Original");
                    QJsonArray pixels;
                    for (auto i = 0; i < replaced_pixels.size(); i++)
                    {
                        pixels.push_back(static_cast<int>(replaced_pixels[i]));
                    }
                    state_object.insert("replaced_pixels", pixels);
                    state_object.insert("state description",state_description);
                    break;
                }
            case ProcessingMethod::RPCP_noise_suppression:
                state_object.insert("method", "RPCP");
                state_object.insert("state description",state_description);
                break;
            case ProcessingMethod::adaptive_noise_suppression:
                state_object.insert("method", "ANS");
                state_object.insert("ANS_relative_start_frame", ANS_relative_start_frame);
                state_object.insert("ANS_num_frames", ANS_num_frames);
				state_object.insert("ANS_hide_shadow", ANS_hide_shadow);
                state_object.insert("ANS_shadow_threshold", ANS_shadow_threshold);
                state_object.insert("state description",state_description);
                break;
            case ProcessingMethod::deinterlace:
                state_object.insert("method", "Deinterlace");
                break;
            case ProcessingMethod::fixed_noise_suppression:
                state_object.insert("method", "FNS");
                state_object.insert("FNS_start_frame", FNS_start_frame);
                state_object.insert("FNS_stop_frame", FNS_stop_frame);
                state_object.insert("FNS_file_path", FNS_file_path);
                state_object.insert("state description",state_description);
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
                break;
            }
            case ProcessingMethod::frame_stacking:
                state_object.insert("method", "Frame Stacking");
                state_object.insert("frame_stack_num_frames", frame_stack_num_frames);
                state_object.insert("state description",state_description);
                break;
            default:
                state_object.insert("method", "error");
        }
        return state_object;
    };
};

processingState create_processing_state_from_json(const QJsonObject & json_obj);

#endif
