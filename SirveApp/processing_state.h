#include "video_details.h"
#include "deinterlace_type.h"

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
};