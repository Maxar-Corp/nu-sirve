#pragma once
#ifndef PROCESSING_STATE_H
#define PROCESSING_STATE_H

#include "video_details.h"
#include "deinterlace_type.h"

#include <QString>

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
    int bgs_relative_start_frame;
    int bgs_num_frames;

    QString nuc_file_path;
    int nuc_start_frame;
    int nuc_stop_frame;

    deinterlace_type deint_type;

    QString get_friendly_description() {
       switch (method)
        {
            case Processing_Method::original:
                return "Original";
                break;
            case Processing_Method::background_subtraction:
                return "BGS - from " + QString::number(bgs_relative_start_frame) + ", averaging " + QString::number(bgs_num_frames) + " frames";
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
};

#endif