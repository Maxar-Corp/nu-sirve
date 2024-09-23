#pragma once

#include <vector>
#include <qobject.h>

#include <QString>
#include <QtWidgets>
#include "processing_state.h"

class VideoContainer : public QObject
{
	Q_OBJECT
	
    public:
        VideoContainer();
        ~VideoContainer();

		std::vector<processingState> processing_states;
		int current_idx;

        void ClearProcessingStates();
        void AddProcessingState(processingState new_details);
        processingState CopyCurrentState();
        processingState CopyCurrentStateIdx(int idx);
        void PopProcessingState();

		std::vector<processingState> get_processing_states();

	signals:
        void updateDisplayVideo();

        void statesCleared();
        void stateAdded(QString state_name, QString combobox_state_name, int index);
        void stateRemoved(ProcessingMethod method, int index);

	public slots:
        void SelectState(int idx);

	private:
};

