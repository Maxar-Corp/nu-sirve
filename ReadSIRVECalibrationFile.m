function out = ReadSIRVECalibrationFile(fname)

fid = fopen(fname, 'rb');

out.calibration_available = fread(fid, 1, 'int32');
out.integration_time = fread(fid, 1, 'double');

% Read UTF-8 string (path_nuc)
len = fread(fid, 1, 'uint32');
out.path_nuc = native2unicode(fread(fid, len, 'uint8=>uint8')', 'UTF-8');

% Read UTF-8 string (path_image)
len = fread(fid, 1, 'uint32');
out.path_image = native2unicode(fread(fid, len, 'uint8=>uint8')', 'UTF-8');

disp("Path NUC: " + out.path_nuc);
disp("Path IMG: " + out.path_image);

read_selected = @(fid) struct( ...
    'valid_data', fread(fid, 1, 'int32'), ...
    'temperature_mean', fread(fid, 1, 'double'), ...
    'temperature_std', fread(fid, 1, 'double'), ...
    'num_frames', fread(fid, 1, 'int32'), ...
    'initial_frame', fread(fid, 1, 'int32'), ...
    'id', fread(fid, 1, 'int32'), ...
    'start_time', fread(fid, 1, 'double'), ...
    'stop_time', fread(fid, 1, 'double'), ...
    'calculated_irradiance', fread(fid, 1, 'double') ...
);

out.user_selection1 = read_selected(fid);
out.user_selection2 = read_selected(fid);

rows = fread(fid, 1, 'uint32');
cols = fread(fid, 1, 'uint32');
out.m = fread(fid, [cols, rows], 'double')';  % transpose to match MATLAB

rows = fread(fid, 1, 'uint32');
cols = fread(fid, 1, 'uint32');
out.b = fread(fid, [cols, rows], 'double')';  % transpose

fclose(fid);

end
