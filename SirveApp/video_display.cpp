#include "video_display.h"


VideoDisplay::VideoDisplay(int x_pixels, int y_pixels, int input_bit_level)
{
	zoom_manager = new VideoDisplayZoomManager(x_pixels, y_pixels);
	label = new EnhancedLabel(this);
	video_display_layout = new QVBoxLayout();
	setup_pinpoint_display();
	setup_labels();

	is_zoom_active = false;
	is_calculate_active = false;
	is_pinpoint_active = false;
	should_show_bad_pixels = false;

	histogram_plot = new HistogramLine_Plot(input_bit_level);

	counter = 0;
	starting_frame_number = 0;
	counter_record = 0;
	index_current_video = -1;
	record_frame = false;
	show_relative_histogram = false;

	image_x = x_pixels;
	image_y = y_pixels;
	number_pixels = image_x * image_y;
	max_bit_level = input_bit_level;
	number_of_frames = 0;

	banner_text = QString("EDIT CLASSIFICATION");
	banner_color = QColor("Red");
	tracker_color = QColor("Red");

	plot_tracks = false;
	display_boresight_txt = false;
	display_time = false;

	auto_lift_gain = false;
	lift = 0;
	gain = 1;
	// intializes color map to gray scale
	index_video_color = 0;
	colorTable = video_colors.maps[index_video_color].colors;
}

VideoDisplay::~VideoDisplay()
{
	delete label;

	delete lbl_frame_number;
	delete lbl_video_time_midnight;
	delete lbl_zulu_time;
	
	delete zoom_manager;
}

void VideoDisplay::setup_pinpoint_display()
{
	grp_pinpoint = new QGroupBox("Selected Pixels");
	grp_pinpoint->setMaximumHeight(200);

	pinpoint_layout = new QHBoxLayout();

	lbl_pinpoint = new QLabel();

	QPixmap pinpoint_image("icons/crosshair.png");
	QIcon pinpoint_icon(pinpoint_image);
	btn_pinpoint = new QPushButton();
	btn_pinpoint->setMaximumSize(40, 40);
	btn_pinpoint->setIcon(pinpoint_icon);
	btn_pinpoint->setToolTip("Pinpoint");
	btn_pinpoint->setCheckable(true);
	connect(btn_pinpoint, &QPushButton::clicked, this, &VideoDisplay::handle_btn_pinpoint);

	QVBoxLayout* button_layout = new QVBoxLayout();

	btn_pinpoint_bad_pixel = new QPushButton("Mark as Bad Pixels");
	connect(btn_pinpoint_bad_pixel, &QPushButton::clicked, this, &VideoDisplay::add_pinpoints_to_bad_pixel_map);
	btn_pinpoint_good_pixel = new QPushButton("Mark as Good Pixels");
	connect(btn_pinpoint_good_pixel, &QPushButton::clicked, this, &VideoDisplay::remove_pinpoints_from_bad_pixel_map);

	button_layout->addWidget(btn_pinpoint_bad_pixel);
	button_layout->addWidget(btn_pinpoint_good_pixel);

	QPixmap clear_image("icons/cancel.png");
	QIcon clear_icon(clear_image);
	btn_clear_pinpoints = new QPushButton();
	btn_clear_pinpoints->setMaximumSize(40, 40);
	btn_clear_pinpoints->setIcon(clear_icon);
	btn_clear_pinpoints->setToolTip("Clear");
	connect(btn_clear_pinpoints, &QPushButton::clicked, this, &VideoDisplay::clear_pinpoints);

	pinpoint_layout->addWidget(btn_pinpoint);
	pinpoint_layout->addWidget(lbl_pinpoint);
	pinpoint_layout->addStretch(1);
	pinpoint_layout->addLayout(button_layout);
	pinpoint_layout->addWidget(btn_clear_pinpoints);

	grp_pinpoint->setLayout(pinpoint_layout);

	video_display_layout->addStretch(1);
	video_display_layout->addWidget(grp_pinpoint);
}

void VideoDisplay::setup_labels()
{
	label->setBackgroundRole(QPalette::Base);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	label->setScaledContents(true);

	label->setObjectName("video_object");

	connect(label, &EnhancedLabel::highlighted_area, this, &VideoDisplay::handle_image_area_selection);
	connect(label, &EnhancedLabel::right_clicked, this, &VideoDisplay::unzoom);
	connect(label, &EnhancedLabel::clicked, this, &VideoDisplay::pinpoint);

	video_display_layout->insertWidget(0, label, 0, Qt::AlignHCenter);

	QHBoxLayout* hlayout_video_labels = new QHBoxLayout();
	lbl_frame_number = new QLabel("");
	lbl_video_time_midnight = new QLabel("");
	lbl_zulu_time = new QLabel("");
	lbl_frame_number->setAlignment(Qt::AlignLeft);
	lbl_video_time_midnight->setAlignment(Qt::AlignHCenter);
	lbl_zulu_time->setAlignment(Qt::AlignRight);

	hlayout_video_labels->addWidget(lbl_frame_number);
	hlayout_video_labels->addWidget(lbl_video_time_midnight);
	hlayout_video_labels->addWidget(lbl_zulu_time);

	video_display_layout->insertLayout(1, hlayout_video_labels);

	lbl_pinpoint->setText("");
}

void VideoDisplay::handle_btn_pinpoint(bool checked)
{
	if (checked)
	{
		emit clear_mouse_buttons();
		is_zoom_active = false;
		is_calculate_active = false;
		is_pinpoint_active = true;
	}
	else
	{
		is_pinpoint_active = false;
	}
	update_display_frame();
}

void VideoDisplay::set_starting_frame_number(unsigned int frame_number)
{
	starting_frame_number = frame_number;
}

void VideoDisplay::reclaim_label()
{
	video_display_layout->insertWidget(0, label, 0, Qt::AlignHCenter);
}

void VideoDisplay::clear_all_zoom_levels(int x_pixels, int y_pixels)
{
	delete zoom_manager;
	zoom_manager = new VideoDisplayZoomManager(x_pixels, y_pixels);

	pinpoint_indeces.clear();
	
	update_display_frame();
}

void VideoDisplay::receive_video_data(video_details& new_input)
{
	image_x = new_input.x_pixels;
	image_y = new_input.y_pixels;
	number_pixels = image_x*image_y;
	
	label->setMinimumWidth(image_x);
	label->setMinimumHeight(image_y);

	number_of_frames = new_input.frames_16bit.size();

	update_display_frame();
}

void VideoDisplay::update_banner_text(QString input_banner_text)
{
	banner_text = input_banner_text;
	update_display_frame();
}

void VideoDisplay::update_banner_color(QString input_color)
{
	QColor new_color(input_color);
	banner_color = new_color;
	update_display_frame();
}

void VideoDisplay::update_color_map(QString input_map)
{

	// find number of color maps
	int number_maps = video_colors.maps.size();

	// cycle through all color maps
	for (int i = 0; i < number_maps; i++)
	{
		// checks to find where input_map matches provided maps
		if (input_map == video_colors.maps[i].name)
			index_video_color = i;
	}

	// sets color table
	colorTable = video_colors.maps[index_video_color].colors;

	update_display_frame();
}

void VideoDisplay::update_tracker_color(QString input_color)
{
	QColor new_color(input_color);
	tracker_color = new_color;
}

void VideoDisplay::toggle_osm_tracks(bool input)
{
	plot_tracks = input;
}

void VideoDisplay::toggle_sensor_boresight_data(bool input)
{
	display_boresight_txt = input;
}

void VideoDisplay::toggle_relative_histogram()
{
	if (show_relative_histogram)
		show_relative_histogram = false;
	else
		show_relative_histogram = true;
}

void VideoDisplay::toggle_action_zoom(bool status)
{
	if (status) {
		is_zoom_active = true;
		is_calculate_active = false;
		is_pinpoint_active = false;
		btn_pinpoint->setChecked(false);
	}
	else {
		is_zoom_active = false;
	}

	update_display_frame();
}

void VideoDisplay::toggle_action_calculate_radiance(bool status)
{
	if (status) {

		is_zoom_active = false;
		is_calculate_active = true;
		is_pinpoint_active = false;
		btn_pinpoint->setChecked(false);
	}
	else {
		is_calculate_active = false;
	}
	
	update_display_frame();
}

void VideoDisplay::handle_image_area_selection(QRect area)
{
	// check to make sure rectangle doesn't exceed dimensions. if so, shorten
	if (area.x() + area.width() > image_x)
	{
		area.setWidth(image_x - area.x());
	}

	if (area.y() + area.height() > image_y)
	{
		area.setHeight(image_y - area.y());
	}

	if (is_zoom_active)
	{
		zoom_manager->zoom_image(area);
		update_display_frame();
	}
	else if (is_calculate_active)
	{
		calibrate(area);
		update_display_frame();
	}
	else
	{
		return;
	}
}

void VideoDisplay::calibrate(QRect area)
{
	// The calculation_region should be calculated by the zoom manager,
	// but I'm leaving this code as-is since it is unused and untestable
	size_t num_zooms = zoom_manager->zoom_list.size();

	if (num_zooms == 1) {
		calculation_region = area;
	}

	else {
		QRect adjusted_area = area;

		for (auto i = num_zooms - 1; i > 0; i--)
		{

			int* x1, * y1, * x2, * y2;
			x1 = new int;
			y1 = new int;
			x2 = new int;
			y2 = new int;

			adjusted_area.getCoords(x1, y1, x2, y2);

			QRect zoom = zoom_manager->zoom_list[i];

			double x1_position = *x1 * 1.0 / image_x;
			double y1_position = *y1 * 1.0 / image_y;
			int new_x1 = std::round(x1_position * zoom.width()) + zoom.x();
			int new_y1 = std::round(y1_position * zoom.height()) + zoom.y();

			double x2_position = *x2 * 1.0 / image_x;
			double y2_position = *y2 * 1.0 / image_y;
			int new_x2 = std::round(x2_position * zoom.width()) + zoom.x();
			int new_y2 = std::round(y2_position * zoom.height()) + zoom.y();

			adjusted_area.setCoords(new_x1, new_y1, new_x2, new_y2);
		}

		calculation_region = adjusted_area;
	}
}

void VideoDisplay::unzoom()
{
	if (zoom_manager->is_currently_zoomed())
	{
		zoom_manager->unzoom();
		update_display_frame();
	}
}

void VideoDisplay::pinpoint(QPoint origin)
{
	// Note that each element in zoom_list contains the _relative_ position within the previous zoom state
	// Simply storing the absolute position within the image makes a lot of code simpler - like the calculations in this function
	// However, due to the way we currently zoom using chained QImage.scaled calls (which require integer x/y/width/height) ...
	// ... the code must still maintain the relative zoom chain.

	// It may be worth "forcing" each view state to align (x/y/width/height) with pixel boundaries, which would enable us to ...
	// ... maintain only the absolute_zoom_list, but this would have implications for aspect ratio.
	// Storing the absolute zoom levels is not ideal (duplication), but a good half-way point that lets me keep moving for now.

	if (is_pinpoint_active) {
		absolute_zoom_info rectangle = zoom_manager->absolute_zoom_list[zoom_manager->zoom_list.size() - 1];
		double absolute_x = rectangle.x + rectangle.width * (1.0 * origin.x() / image_x);
		double absolute_y = rectangle.y + rectangle.height * (1.0 * origin.y() / image_y);

		unsigned int pinpoint_x = std::floor(absolute_x);
		unsigned int pinpoint_y = std::floor(absolute_y);

		unsigned int pinpoint_idx = pinpoint_y * image_x + pinpoint_x;
		
		//Disallow clicking an already-pinpointed pixel
		if ( std::find(pinpoint_indeces.begin(), pinpoint_indeces.end(), pinpoint_idx) != pinpoint_indeces.end())
		{
			return;
		}

		pinpoint_indeces.push_back(pinpoint_idx);

		if (pinpoint_indeces.size() > 3)
		{
			pinpoint_indeces.erase(pinpoint_indeces.begin());
		}
		// Should be able to just update_partial_frame here or something
		update_display_frame();
	}
}

void VideoDisplay::remove_pinpoints_from_bad_pixel_map()
{
	if (pinpoint_indeces.size() > 0)
	{
		emit remove_bad_pixels(pinpoint_indeces);
	}
}

void VideoDisplay::add_pinpoints_to_bad_pixel_map()
{
	if (pinpoint_indeces.size() > 0)
	{
		emit add_new_bad_pixels(pinpoint_indeces);
	}
}

void VideoDisplay::clear_pinpoints()
{
	pinpoint_indeces.clear();
	update_display_frame();
}

void VideoDisplay::update_display_frame()
{
	// In case update_display_frame is called before a video is fully placed 
	if (number_of_frames == 0 || number_of_frames < counter)
		return;

	//------------------------------------------------------------------------------------------------

	//Convert current frame to armadillo matrix
	std::vector<double> frame_vector(container.processing_states[container.current_idx].details.frames_16bit[counter].begin(),
		container.processing_states[container.current_idx].details.frames_16bit[counter].end());
	arma::vec image_vector(frame_vector);

	//Normalize the image to values between 0 - 1
	int max_value = std::pow(2, max_bit_level);
	image_vector = image_vector / max_value;

	if (image_vector.max() < 1) {
		double sigma = arma::stddev(image_vector);
		double meanVal = arma::mean(image_vector);
		image_vector = image_vector / (meanVal + 3. * sigma) - .5;
	}

	if (auto_lift_gain)
	{
		double sigma = arma::stddev(image_vector);
		double meanVal = arma::mean(image_vector);
		lift = meanVal - 3.*sigma;
		gain = meanVal + 3.*sigma;
		emit force_new_lift_gain(lift, gain);
	}
	//------------------------------------------------------------------------------------------------
	// Update the absolute histogram
	histogram_plot->plot_absolute_histogram(image_vector, lift, gain);

	// Correct image based on min/max value inputs
	ColorCorrection::update_color(image_vector, lift, gain);

	// Create points for the relative histogram
	if (show_relative_histogram) {
		histogram_plot->plot_relative_histogram(image_vector);
	}
	else
	{
		histogram_plot->rel_chart->removeAllSeries();
	}

	//------------------------------------------------------------------------------------------------

	// Put image into 8-bit format for displaying
	image_vector = image_vector * 255;

	//arma::vec out_frame_flat = arma::vectorise(image_vector);
	std::vector<double>out_vector = arma::conv_to<std::vector<double>>::from(image_vector);
	std::vector<uint8_t> converted_values(out_vector.begin(), out_vector.end());
	uint8_t* color_corrected_frame = converted_values.data();

	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------

	frame = QImage((uchar*)color_corrected_frame, image_x, image_y, QImage::Format_Grayscale8);

	// Convert image to format_indexed. allows color table to take effect on image
	frame = frame.convertToFormat(QImage::Format_Indexed8);
	frame.setColorTable(colorTable);

	// Convert image back to RGB to facilitate use of the colors 
	frame = frame.convertToFormat(QImage::Format_RGB888);

	if (should_show_bad_pixels)
	{
		for (auto i = 0; i < container.processing_states[0].replaced_pixels.size(); i++)
		{
			unsigned int pixel_index = container.processing_states[0].replaced_pixels[i];

			int pixel_x = pixel_index % image_x;
			int pixel_y = pixel_index / image_x;

			QRgb red = QColorConstants::Yellow.rgb();
			frame.setPixel(pixel_x, pixel_y, red);
		}
	}

	QString pinpoint_text("");
	for (auto idx = 0; idx < pinpoint_indeces.size(); idx++)
	{
		int pinpoint_idx = pinpoint_indeces[idx];
		
		if (pinpoint_idx < frame_vector.size())
		{
			int irradiance_value = frame_vector[pinpoint_idx];
			int pinpoint_x = pinpoint_idx % image_x;
			int pinpoint_y = pinpoint_idx / image_x;
			pinpoint_text += "Pixel: " + QString::number(pinpoint_x + 1) + "," + QString::number(pinpoint_y + 1) + ". Value: " + QString::number(irradiance_value);

			if ( std::find(container.processing_states[0].replaced_pixels.begin(), container.processing_states[0].replaced_pixels.end(), pinpoint_idx) != container.processing_states[0].replaced_pixels.end() )
			{
				pinpoint_text += " * (adjusted, bad pixel)";
			}

			if (is_pinpoint_active)
			{
				QRgb rgb_red = QColorConstants::Red.rgb();
				frame.setPixel(pinpoint_x, pinpoint_y, rgb_red);
			}
		}
		else
		{
			pinpoint_text += "Clicked outside of valid x/y coordinate range.";
		}

		pinpoint_text += "\n";
	}
	lbl_pinpoint->setText(pinpoint_text);

	//SIRVE's display and zoom logic (a simple recursive "zoom, scale, zoom, scale" thing using QImage as the base) is problematic.

	//Qt QImage .copy/.scaled calls are susceptible to resulting in pixel problems when things do not match both these criteria:
	//1) The sub_frame .copy'd from the original image exactly matches the viewport aspect ratio and
	//2) The width/height scale directly to the viewport in an integer ratio, i.e. each new pixel is exactly 2 or 3 or 4 new pixels wide/tall
	//Otherwise, Qt just does what it wants with the original pixels, stretching random columns/rows and not keeping square pixels

	//Note: the issue with "non-square pixels" is that they're not CONSISTENTLY non-square
	//Being non-square would be okay if, for example, they were all uniformly rectangular
	//The issue described above is that _some_ rows/columns are squished while _other_ rows/columns get stretched, in a random pattern

	//The ideal way to handle zoom is probably a bit more complicated
	//I think SIRVE should continue using a QImage so that we can do things like .setPixel and such and access the pixels directly
	//However, we don't necessarily need to bake generating a whole new QImage just for the zoom feature, and
	//We don't necessarily need to display the raw QImage that we've manipulated

	//For simply displaying the image after we've done all the manipulating, particularly if we want to allow ...
	// ... 3d transformations or alternative view angles like the requested "bowl" display, ...
	// ... we probably want to look at a QGraphicsView or something.
	// It's possible to put a QImage in a GraphicsScene via a QPixMap, e.g. via https://stackoverflow.com/questions/5960074/qimage-in-a-qgraphics-scene

	//A bonus of doing this stuff is that it may get easier to allow zooms/panning/etc. in the future, e.g.
	//https://stackoverflow.com/questions/35508711/how-to-enable-pan-and-zoom-in-a-qgraphicsview
	//or
	//https://stackoverflow.com/questions/60240192/zooming-in-out-on-images-qt-c

	for (int i = 0; i < zoom_manager->zoom_list.size(); i++)
	{
		QRect sub_frame = zoom_manager->zoom_list[i];

		// get sub-image
		frame = frame.copy(sub_frame);

		// scale to initial aspect ratio
		frame = frame.scaled(image_x, image_y);
	}

	if (zoom_manager->is_currently_zoomed())
	{
		label->setStyleSheet("#video_object { border: 3px solid blue; }");
	}
	else
	{
		label->setStyleSheet("#video_object { border: 1px solid light gray; }");
	}

	size_t num_osm_tracks = osm_track_frames[counter].tracks.size();
	size_t num_manual_tracks = manual_track_frames[counter].tracks.size();

	if (plot_tracks && (num_osm_tracks > 0 || num_manual_tracks > 0))
	{
		absolute_zoom_info final_zoom_level = zoom_manager->absolute_zoom_list[zoom_manager->absolute_zoom_list.size() - 1];
		double size_of_pixel_x = 1.0 * image_x / final_zoom_level.width;
		double size_of_pixel_y = 1.0 * image_y / final_zoom_level.height;

		int box_size = 5;
		double box_width = size_of_pixel_x - 1 + box_size * 2;
		double box_height = size_of_pixel_y - 1 + box_size * 2;

		QPainter rectangle_painter(&frame);
		rectangle_painter.setPen(QPen(tracker_color));

		for ( const auto &trackData : osm_track_frames[counter].tracks )
		{
			//The OSM tracks are stored offset from the center instead of the top left
			int x_center = image_x / 2 + trackData.second.centroid_x;
			int y_center = image_y / 2 + trackData.second.centroid_y;
			QRectF rectangle = get_rectangle_around_pixel(x_center, y_center, box_size, box_width, box_height);
			if (rectangle.isNull())
				continue;
			rectangle_painter.drawRect(rectangle);
		}

		rectangle_painter.setPen(QPen(banner_color));
		for ( const auto &trackData : manual_track_frames[counter].tracks )
		{
			int track_id = trackData.first;
			if (manual_track_ids_to_show.find(track_id) != manual_track_ids_to_show.end())
			{
				QRectF rectangle = get_rectangle_around_pixel(trackData.second.centroid_x, trackData.second.centroid_y, box_size, box_width, box_height);
				if (rectangle.isNull())
					continue;
				rectangle_painter.drawRect(rectangle);
			}
		}
	}

	if (display_boresight_txt) {
		QPainter p2(&frame);
		p2.setPen(QPen(banner_color));
		p2.setFont(QFont("Times", 8, QFont::Bold));

		QString boresight_txt = "Sensor Boresight \n Az: " + QString::number(display_data[counter].azimuth_sensor) + "\n El " + QString::number(display_data[counter].elevation_sensor);

		p2.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignLeft, boresight_txt);
	}

	lbl_frame_number->setText("Frame # " + QString::number(starting_frame_number + counter));

	double seconds_midnight = display_data[counter].seconds_past_midnight;
	lbl_video_time_midnight->setText("From Midnight " + QString::number(seconds_midnight, 'g', 8));

	QString zulu_time = get_zulu_time_string(seconds_midnight);
	lbl_zulu_time->setText(zulu_time);

	if (display_time) {
		QPainter p2(&frame);
		p2.setPen(QPen(banner_color));
		p2.setFont(QFont("Times", 8, QFont::Bold));

		p2.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignRight, zulu_time);
	}

	// ---------------------------------------------------------------------------------------

	// Draw annotations

	size_t num_annotations = annotation_list.size();

	// if there are annotations ...
	if (num_annotations > 0) {

		// for each annotation ...
		for (auto i = 0; i < num_annotations; i++) {

			// get frame information
			annotation_info a = annotation_list[i];
			unsigned int initial_frame_annotation = a.frame_start - a.min_frame;
			unsigned int last_frame_annotation = initial_frame_annotation + a.num_frames;

			// check that current frame is within bounds
			if (counter >= initial_frame_annotation && counter < last_frame_annotation) {

				QString annotation_color = a.color;
				int font_size = a.font_size;
				QString annotation_text = a.text;

				std::vector<int> loc = zoom_manager->get_position_within_zoom(a.x_pixel, a.y_pixel);
				int x = loc[0];
				int y = loc[1];

				if (loc[0] >= 0)
				{
					// write text
					QPainter p_a(&frame);
					p_a.setPen(QPen(annotation_color));
					p_a.setFont(QFont("Times", font_size));
					p_a.drawText(x, y, annotation_text);
				}
			}
		}

	}
	// ---------------------------------------------------------------------------------------

	// determine if radiance calculation is selected
	bool rectangle_drawn = calculation_region.width() > 1 && calculation_region.height() > 1;
	if (is_calculate_active && rectangle_drawn) {

		int* r1 = new int;
		int* r2 = new int;
		int* c1 = new int;
		int* c2 = new int;

		// get the coordinates of calculation region
		calculation_region.getCoords(c1, r1, c2, r2);
		std::vector<int> pt1 = zoom_manager->get_position_within_zoom(*c1, *r1);
		std::vector<int> pt2 = zoom_manager->get_position_within_zoom(*c2, *r2);

		// determine if calculation region is within zoomed image
		bool region_within_zoom = pt1[0] >= 0 && pt2[0] >= 0;
		if (rectangle_drawn && region_within_zoom) {

			// get frame data from original data set and convert mat
			std::vector<double> original_frame_vector(container.processing_states[0].details.frames_16bit[counter].begin(), container.processing_states[0].details.frames_16bit[counter].end());
			arma::vec original_image_vector(original_frame_vector);
			arma::mat original_mat_frame(original_image_vector);
			original_mat_frame.reshape(image_x, image_y);
			original_mat_frame = original_mat_frame.t();

			// get counts sub-matrix corresponding to the calculation region 
			unsigned int ur1 = (unsigned int)*r1;
			unsigned int uc1 = (unsigned int)*c1;
			unsigned int ur2 = (unsigned int)*r2;
			unsigned int uc2 = (unsigned int)*c2;

			arma::mat counts = original_mat_frame.submat(ur1, uc1, ur2, uc2);

			// clear all temporary variables
			original_frame_vector.clear();
			original_frame_vector.clear();
			original_mat_frame.clear();

			double frame_integration_time = frame_headers[counter].header.int_time;
			std::vector<double>measurements = model.measure_irradiance(*r1, *c1, *r2, *c2, counts, frame_integration_time);

			// -----------------------------------------------------------------------------------
			// print radiance calculation data onto frame
			QString max_value = QString::number(measurements[0]) + " W/m^2/sr";
			QString avg_value = QString::number(measurements[1]) + " W/m^2/sr";
			QString sum_value = QString::number(measurements[2]) + " W/m^2/sr";

			QString calculation_text = "***** Beta Calculation *****\n";
			calculation_text.append("Max Pixel: " + max_value + "\n");
			calculation_text.append("Avg Pixel: " + avg_value + "\n");
			calculation_text.append("Total: " + sum_value);

			QPainter painter_calculation_text(&frame);
			painter_calculation_text.setPen(QPen(banner_color));
			painter_calculation_text.setFont(QFont("Times", 8, QFont::Bold));
			painter_calculation_text.drawText(frame.rect(), Qt::AlignTop | Qt::AlignRight, calculation_text);
			painter_calculation_text.end();

			// -----------------------------------------------------------------------------------
			// draw rectangle of calculation region			
			QPainter calculation_area_painter(&frame);

			QPen pen_calculation_area;
			pen_calculation_area.setStyle(Qt::DashDotLine);
			pen_calculation_area.setWidth(3);
			pen_calculation_area.setBrush(tracker_color);

			QPoint top_left(pt1[0], pt1[1]);
			QPoint bottom_right(pt2[0], pt2[1]);
			QRect zoomed_rectange(top_left, bottom_right);

			calculation_area_painter.setPen(pen_calculation_area);
			calculation_area_painter.drawRect(zoomed_rectange);
		}
	}

	// Draw banner text
	QPainter p1(&frame);
	p1.setPen(QPen(banner_color));
	p1.setFont(QFont("Times", 12, QFont::Bold));
	p1.drawText(frame.rect(), Qt::AlignTop | Qt::AlignHCenter, banner_text);
	p1.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignHCenter, banner_text);

	if (record_frame && video_frame_number != counter) {
		video_frame_number = counter;
		add_new_frame(frame, CV_8UC3);
	}

	label->setPixmap(QPixmap::fromImage(frame));

	label->update();
	label->repaint();

	//counter++;
}

QRectF VideoDisplay::get_rectangle_around_pixel(int x_center, int y_center, int box_size, double box_width, double box_height)
{
	if (!zoom_manager->is_any_piece_within_zoom(x_center, y_center))
		return QRectF();

	std::vector<int> loc = zoom_manager->get_position_within_zoom(x_center, y_center);
	int x = loc[0];
	int y = loc[1];
	std::vector<int> loc2 = zoom_manager->get_position_within_zoom(x_center + 1, y_center + 1);
	int x2 = loc2[0];
	int y2 = loc2[1];

	// If the "centroid" pixel is not in the box, this helper method returns -1, -1 ...
	// So we position the box based on loc2, the pixel below and to the right
	int top_left_x, top_left_y;
	if (x < 0)
	{
		top_left_x = x2 - box_width - box_size;
		top_left_y = y2 - box_height - box_size;
	}
	else
	{
		top_left_x = x - box_size;
		top_left_y = y - box_size;
	}

	QRectF rectangle(top_left_x, top_left_y, box_width, box_height);
	return rectangle;
}

QString VideoDisplay::get_zulu_time_string(double seconds_midnight)
{
	int hour = seconds_midnight / 3600;
	int minutes = (seconds_midnight - hour * 3600) / 60;
	double seconds = seconds_midnight - hour * 3600 - minutes * 60;

	QString zulu_time("");
	if (hour < 10)
		zulu_time.append("0");
	zulu_time.append(QString::number(hour));
	zulu_time.append(":");

	if (minutes < 10)
		zulu_time.append("0");
	zulu_time.append(QString::number(minutes));
	zulu_time.append(":");

	if (seconds < 10)
		zulu_time.append("0");
	zulu_time.append(QString::number(seconds, 'f', 3));
	zulu_time.append("Z");
	return zulu_time;
}

void VideoDisplay::highlight_bad_pixels(bool status)
{
	should_show_bad_pixels = status;
}

void VideoDisplay::update_frame_data(std::vector<Plotting_Frame_Data> input_data)
{
	display_data = input_data;
}

void VideoDisplay::initialize_track_data(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input)
{
	osm_track_frames = osm_frame_input;
	manual_track_frames = manual_frame_input;
}

void VideoDisplay::update_manual_track_data(std::vector<TrackFrame> track_frame_input)
{
	manual_track_frames = track_frame_input;
}

void VideoDisplay::add_manual_track_id_to_show_later(int id)
{
	manual_track_ids_to_show.insert(id);
}

void VideoDisplay::hide_manual_track_id(int id)
{
	manual_track_ids_to_show.erase(id);
	update_display_frame();
}

void VideoDisplay::show_manual_track_id(int id)
{
	manual_track_ids_to_show.insert(id);
	update_display_frame();
}

void VideoDisplay::set_frame_data(std::vector<Plotting_Frame_Data> input_data, std::vector<ABIR_Frame>& input_frame_header)
{
	display_data = input_data;
	frame_headers = input_frame_header;
}

void VideoDisplay::set_calibration_model(CalibrationData input)
{
	model = input;
}

bool VideoDisplay::start_recording(double fps)
{

	QString file_name = QFileDialog::getSaveFileName(this, "Save File", "", "Video (*.avi)");

	if (file_name.isEmpty())
		return false;

	std::string file_string = file_name.toLocal8Bit().constData();

	video.open(file_string, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(image_x, image_y));

	bool video_opened = video.isOpened();

	if (video_opened)
		record_frame = true;

	return video_opened;
}

void VideoDisplay::add_new_frame(QImage& img, int format)
{
	QImage image = img.rgbSwapped();
	cv::Mat output_frame(image.height(), image.width(), format, image.bits(), image.bytesPerLine());
	video.write(output_frame);
}

void VideoDisplay::stop_recording()
{
	video.release();
	record_frame = false;
}


void VideoDisplay::save_frame()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images (*.png)"));

	if (!fileName.isEmpty()) {
		frame.save(fileName);
	}
}

void VideoDisplay::remove_frame()
{
	delete label;
	
	delete lbl_frame_number;
	delete lbl_video_time_midnight;
	delete lbl_zulu_time;

	label = new EnhancedLabel(this);
	setup_labels();

	histogram_plot->remove_histogram_plots();
	histogram_plot->initialize_histogram_plot();

	frame_data.clear();
	number_of_frames = 0;

	image_x = 0;
	image_y = 0;
	number_pixels = image_x * image_y;

	delete zoom_manager;
	zoom_manager = new VideoDisplayZoomManager(0, 0);
}

void VideoDisplay::update_specific_frame(unsigned int frame_number)
{
	counter = frame_number;
	update_display_frame();
}

void VideoDisplay::update_color_correction(double new_min_value, double new_max_value)
{
	lift = new_min_value;
	gain = new_max_value;
	update_display_frame();
}
