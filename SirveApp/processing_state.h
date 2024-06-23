#pragma once
#ifndef PROCESSING_STATE_H
#define PROCESSING_STATE_H

#include "video_details.h"
#include "deinterlace_type.h"

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

enum struct ProcessingMethod
{
	original,
	adaptive_noise_suppression,
	fixed_noise_suppression,
    deinterlace,
    center_on_OSM,
    center_on_manual
};

struct processingState {
    ProcessingMethod method;
    VideoDetails details;

    //NOTE: This is a poor implementation of "polymorphic" configuration but can be cleaned up in a future refactor
    //These fields will only contain a value if the Processing_Method field is set to indicate they should
    //The burden is on consumers of the processingState struct to correctly interpret the fields
    std::vector<unsigned int> replaced_pixels;

    int ANS_relative_start_frame;
    int ANS_num_frames;
    int ANS_shadow_threshold;

    QString FNS_file_path;
    int FNS_start_frame;
    int FNS_stop_frame;

    DeinterlaceType deint_type;
    int track_id;
	std::vector<std::vector<int>> offsets;

	bool ANS_hide_shadow;

    QString get_friendly_description() {
       switch (method)
        {
            case ProcessingMethod::original:
                if (replaced_pixels.size() > 0)
                {
                    return "Original (with replaced pixels)";
                }
                else
                {
                    return "Original";
                }
                break;
            case ProcessingMethod::adaptive_noise_suppression:
                return "ANS - from " + QString::number(ANS_relative_start_frame) + ", averaging " + QString::number(ANS_num_frames) + " frames.  Hide Shadow option set to " + QString::number(ANS_hide_shadow) + ". Shadow threshold set to " + QString::number(ANS_shadow_threshold);
                break;
            case ProcessingMethod::fixed_noise_suppression:
                //may potentially want to leave fns_file_path empty if it isn't an external file?
                return "FNS - " + QString::number(FNS_start_frame) + " to " + QString::number(FNS_stop_frame);
                break;
            case ProcessingMethod::deinterlace:
                return "Deinterlace - " + QString::number(deint_type);
                break;
            case ProcessingMethod::center_on_OSM:
                return "Centered on OSM - " + QString::number(track_id);
                break;
            case ProcessingMethod::center_on_manual:
                return "Centered on Manual - " + QString::number(track_id);
                break;
            default:
                return "Unknown";
                break;
        }
    };

    QJsonObject to_json() {
        QJsonObject state_object;
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
                    break;
                }
            case ProcessingMethod::adaptive_noise_suppression:
                state_object.insert("method", "ANS");
                state_object.insert("ANS_relative_start_frame", ANS_relative_start_frame);
                state_object.insert("ANS_num_frames", ANS_num_frames);
				state_object.insert("ANS_hide_shadow", ANS_hide_shadow);
                state_object.insert("ANS_shadow_threshold", ANS_shadow_threshold);
                break;
            case ProcessingMethod::deinterlace:
                state_object.insert("method", "Deinterlace");
                state_object.insert("deint_type", QString::number(static_cast<int>(deint_type)));
                break;
            case ProcessingMethod::fixed_noise_suppression:
                state_object.insert("method", "FNS");
                state_object.insert("FNS_start_frame", FNS_start_frame);
                state_object.insert("FNS_stop_frame", FNS_stop_frame);
                state_object.insert("FNS_file_path", FNS_file_path);
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
                break;
            }
            default:
                state_object.insert("method", "error");
        }
        return state_object;
    };
};

processingState create_processing_state_from_json(const QJsonObject & json_obj);

#endif
