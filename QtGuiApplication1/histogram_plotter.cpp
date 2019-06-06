#include "histogram_plotter.h"

HistogramLine_Plot::HistogramLine_Plot(unsigned int max_levels, QWidget *parent)
{
	chart = new QChart();
	chart_view = new QChartView(chart);
	text = new QLabel(this);

	//chart->setTitle("Luminosity");
	chart->legend()->hide();

	QColor base_color(colors.GetCurrentColor());
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(3);

	//Assumes video data is 8 bits and 256 bins (one for each bit level)
	maximum_levels = max_levels;
	number_of_bins = std::pow(2, maximum_levels);

}

HistogramLine_Plot::~HistogramLine_Plot(){

	delete chart;
	delete text;
	delete chart_view;
}

/*void HistogramLine_Plot::create_histogram_data(std::vector<uint8_t*>& video_data, int num_pixels)
{

	video_frames = video_data;
	number_of_frames = video_frames.size();

	histogram_data.clear();
	histogram_data.reserve(number_of_frames);

	//TODO figure out how to enforce 8bit / 256 bin assumption
	
	for (int frame_number = 0; frame_number < number_of_frames; frame_number++) {

		std::vector<unsigned int> frame_histogram(number_of_bins, 0);

		for (int pixel_index = 0; pixel_index < num_pixels; pixel_index++)
		{
			uint8_t value = video_frames[frame_number][pixel_index];
			uint8_t index = (uint8_t)value;

			// Check if exceeds maximum bins, puts in last bin
			if (index > number_of_bins)
				index = number_of_bins - 1;

			frame_histogram[index]++;
		}

		histogram_data.push_back(frame_histogram);
	}
}
*/

QList<QPointF> HistogramLine_Plot::create_qpoints()
{
	
	std::vector<std::vector<double>> color_corrected_histogram;
	color_corrected_histogram.reserve(number_of_bins);

	int min_total, max_total, bin_pixels;
	double new_index;

	min_total = 0;
	max_total = 0;

	// Map old bins to new bin values. Sum up values at 0 and max bin size
	for (int i = 0; i < number_of_bins; i++)
	{ 
		new_index = color_correction.get_updated_color(i, number_of_bins);
		bin_pixels = histogram_data[counter][i];

		if (new_index == 0) {
			min_total += bin_pixels;
		}
		else if (new_index == number_of_bins - 1) {
			max_total += bin_pixels;
		}
		else {
			std::vector<double> temp;
			temp.push_back(new_index);
			temp.push_back(bin_pixels);

			color_corrected_histogram.push_back(temp);
		}
	}


	QList<QPointF> histogram_line;
	histogram_line.reserve(number_of_bins * 2);

	double current_x, next_x, bin_size;
	int number_color_corrected_bins = color_corrected_histogram.size();

	// Create line for histogram using the new bin points
	for (int i = 0; i < number_color_corrected_bins; i++) {

		current_x = color_corrected_histogram[i][0];
		bin_size = color_corrected_histogram[i][1];			

		// Add the 0 point on the first iteration
		if (i == 0)
		{
			QPointF temp_pt1(0, min_total);
			histogram_line.push_back(temp_pt1);

			QPointF temp_pt2(current_x, min_total);
			histogram_line.push_back(temp_pt2);
		}

		if (i < number_color_corrected_bins - 1) {

			next_x = color_corrected_histogram[i + 1][0];
			
			QPointF temp_pt1(current_x, bin_size);
			histogram_line.push_back(temp_pt1);

			QPointF temp_pt2(next_x, bin_size);
			histogram_line.push_back(temp_pt2);
		}
		else {
			double delta_x = color_corrected_histogram[i][0] - color_corrected_histogram[i - 1][0];
			next_x = color_corrected_histogram[i][0] + delta_x;

			if (next_x > number_of_bins - 1) {
				// Current value
				QPointF temp_pt1(current_x, bin_size);
				histogram_line.push_back(temp_pt1);

				// Connect bar to last value
				QPointF temp_pt2(number_of_bins -1, bin_size);
				histogram_line.push_back(temp_pt2);
			}
			else
			{
				// Current value
				QPointF temp_pt1(current_x, bin_size);
				histogram_line.push_back(temp_pt1);

				// Create the bar for the current value
				QPointF temp_pt2(next_x, bin_size);
				histogram_line.push_back(temp_pt2);

				// Close the bar to zero
				QPointF temp_pt3(next_x, 0);
				histogram_line.push_back(temp_pt3);

				// Move to the end of the histogram
				QPointF temp_pt4(number_of_bins - 1, 0);
				histogram_line.push_back(temp_pt4);

			}

			// Last Value Pt1
			QPointF temp_pt5(number_of_bins - 1, max_total);
			histogram_line.push_back(temp_pt5);

			// Last Value Pt2
			QPointF temp_pt6(number_of_bins, max_total);
			histogram_line.push_back(temp_pt6);
		}
	}
	   	 
	return histogram_line;
}

/*void HistogramLine_Plot::update_video_frames(std::vector<uint8_t*>& video_data, int num_pixels, unsigned int max_levels)
{
	
	counter = 70;
	
	maximum_levels = max_levels;
	number_of_bins = std::pow(2, maximum_levels);

	create_histogram_data(video_data, num_pixels);

	update_histogram_chart();

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);
}*/

void HistogramLine_Plot::receive_video_data(video_details &new_input)
{
	//update_video_frames(new_input.frames_8bit, new_input.number_pixels, 8);

	counter = 70;

	maximum_levels = 8;
	number_of_bins = std::pow(2, maximum_levels);
	number_of_frames = new_input.frames_8bit.size();

	histogram_data = new_input.histogram_data;

	update_histogram_chart();

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);
}

void HistogramLine_Plot::update_histogram_chart() {

	// Takes ~ 10 ms to run which equates to a max update rate of 100 fps

	if (counter == number_of_frames)
		counter = 0;

	QList<QPointF> histogram_line = create_qpoints();

	QLineSeries *series = new QLineSeries();
	series->setPen(pen);

	QColor base_color(colors.GetCurrentColor());
	series->setColor(base_color);
	series->append(histogram_line);

	chart->removeAllSeries();

	chart->addSeries(series);
	chart->createDefaultAxes();
	QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	y_axis->setMinorGridLineVisible(true);
	y_axis->setLabelsVisible(false);

	x_axis->setMin(0);
	x_axis->setMax(number_of_bins);
	x_axis->setMinorGridLineVisible(true);
	x_axis->setLabelsVisible(false);

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);

	counter++;
}

void HistogramLine_Plot::update_specific_histogram(unsigned int frame_number)
{
	counter = frame_number;
	update_histogram_chart();
}

void HistogramLine_Plot::update_color_correction(double lift, double gamma, double gain)
{
	color_correction.set_lift(lift);
	color_correction.set_gamma(gamma);
	color_correction.set_gain(gain);
}
