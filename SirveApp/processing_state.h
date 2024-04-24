#pragma once
#ifndef PROCESSING_STATE_H
#define PROCESSING_STATE_H

#include "video_details.h"
#include "deinterlace_type.h"

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

enum struct Processing_Method
{
	original,
	background_subtraction,
	non_uniformity_correction,
    deinterlace
};

struct processing_state {
	Processing_Method method;
	video_details details;

    //NOTE: This is a poor implementation of "polymorphic" configuration but can be cleaned up in a future refactor
    //These fields will only contain a value if the Processing_Method field is set to indicate they should
    //The burden is on consumers of the processing_state struct to correctly interpret the fields
    std::vector<unsigned int> replaced_pixels;

    int bgs_relative_start_frame;
    int bgs_num_frames;

    QString nuc_file_path;
    int nuc_start_frame;
    int nuc_stop_frame;

    deinterlace_type deint_type;
	
	bool bgs_hide_shadow;

    QString get_friendly_description() {
       switch (method)
        {
            case Processing_Method::original:
                if (replaced_pixels.size() > 0)
                {
                    return "Original (with replaced pixels)";
                }
                else
                {
                    return "Original";
                }
                break;
            case Processing_Method::background_subtraction:
                return "BGS - from " + QString::number(bgs_relative_start_frame) + ", averaging " + QString::number(bgs_num_frames) + " frames.  Hide Shadow option set to " + QString::boolean(bgs_hide_shadow);
                break;
            case Processing_Method::non_uniformity_correction:
                //may potentially want to leave nuc_file_path empty if it isn't an external file?
                return "NUC - " + QString::number(nuc_start_frame) + " to " + QString::number(nuc_stop_frame);
                break;
            case Processing_Method::deinterlace:
                return "Deinterlace - " + QString::number(deint_type);
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
            case Processing_Method::original:
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
            case Processing_Method::background_subtraction:
                state_object.insert("method", "Background Subtraction");
                state_object.insert("bgs_relative_start_frame", bgs_relative_start_frame);
                state_object.insert("bgs_num_frames", bgs_num_frames);
				state_object.insert("bgs_hide_shadow", bgs_hide_shadow);
                break;
            case Processing_Method::deinterlace:
                state_object.insert("method", "Deinterlace");
                state_object.insert("deint_type", QString::number(static_cast<int>(deint_type)));
                break;
            case Processing_Method::non_uniformity_correction:
                state_object.insert("method", "NUC");
                state_object.insert("nuc_start_frame", nuc_start_frame);
                state_object.insert("nuc_stop_frame", nuc_stop_frame);
                state_object.insert("nuc_file_path", nuc_file_path);
                break;
            default:
                state_object.insert("method", "error");
        }
        return state_object;
    };
};

processing_state create_processing_state_from_json(const QJsonObject & json_obj);

#endif