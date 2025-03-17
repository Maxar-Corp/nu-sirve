#ifndef PROCESSING_STATE_H
#define PROCESSING_STATE_H

#include "video_details.h"

#include <QJsonObject>
#include <QJsonArray>
#include <armadillo>

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
	processingState() = default;
	static processingState FromJson(const QJsonObject & json_obj);

    ProcessingMethod method = ProcessingMethod::original;
    VideoDetails details;

    //NOTE: This is a poor implementation of "polymorphic" configuration but can be cleaned up in a future refactor
    //These fields will only contain a value if the Processing_Method field is set to indicate they should
    //The burden is on consumers of the processingState struct to correctly interpret the fields
    int source_state_ID = -1;
    int state_ID = -1;
    
    std::vector<unsigned int> replaced_pixels;
    std::vector<unsigned int> ancestors;
    std::vector<unsigned int> descendants;

    int ANS_relative_start_frame = -1;
    int ANS_num_frames = -1;

    double weight = 0.0;
    int offset = 0;
    int shadow_threshold = 0;
    bool hide_shadow = false;

    QString FNS_file_path;
    int FNS_start_frame = -1;
    int FNS_stop_frame = -1;
    int frame0 = -1;

    int frame_stack_num_frames = 0;

	std::vector<std::vector<int>> offsets;
    arma::mat offsets_matrix;

    int track_id = -1;
    bool find_any_tracks = false;

    QString state_description;
    QString combobox_state_name;

    QString state_steps;
    QString process_steps;

    QString GetFriendlyDescription() const;
    QString GetComboBoxDescription() const;
    QJsonObject ToJson() const;
    const QString& GetStepName() const;
	void UpdateDescription();
	void UpdateMaxValue();
};


#endif
