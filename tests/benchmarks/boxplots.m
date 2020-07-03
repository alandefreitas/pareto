plot_histograms = true;
plot_hv_time = true;
plot_hv_gap = false;

if (plot_histograms || plot_hv_time || plot_hv_front)
[function_names, dimensions, data_structures, pareto_sizes, samples, aggregate_types, time] = read_data_structure_benchmark();
end

if (plot_histograms)
n_pareto_sizes = 3;
n_operations = 7;
n_structures = 6;
n_dimensions = 6;
group_averages = zeros(n_operations, n_dimensions, n_pareto_sizes);
structure_averages = zeros(n_operations, n_structures, n_dimensions, n_pareto_sizes);
[group_averages(1,:,1), structure_averages(1,:,:,1)] = plot_results('create_front_from_vector', 50,'Construct (\itn=50\rm)','construct_n_50', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(1,:,2), structure_averages(1,:,:,2)] = plot_results('create_front_from_vector', 500,'Construct (\itn=500\rm)','construct_n_500', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(1,:,3), structure_averages(1,:,:,3)] = plot_results('create_front_from_vector', 5000,'Construct (\itn=5000\rm)','construct_n_5000', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(2,:,1), structure_averages(2,:,:,1)] = plot_results('insert_in_front', 50,'Insertion (\itn=50\rm)','insertion_n_50', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(2,:,2), structure_averages(2,:,:,2)] = plot_results('insert_in_front', 500,'Insertion (\itn=500\rm)','insertion_n_500', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(2,:,3), structure_averages(2,:,:,3)] = plot_results('insert_in_front', 5000,'Insertion (\itn=5000\rm)','insertion_n_5000', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(3,:,1), structure_averages(3,:,:,1)] = plot_results('erase_from_front', 50,'Removal (\itn=50\rm)','removal_n_50', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(3,:,2), structure_averages(3,:,:,2)] = plot_results('erase_from_front', 500,'Removal (\itn=500\rm)','removal_n_500', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(3,:,3), structure_averages(3,:,:,3)] = plot_results('erase_from_front', 5000,'Removal (\itn=5000\rm)','removal_n_5000', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(4,:,1), structure_averages(4,:,:,1)] = plot_results('check_dominance', 50,'Check dominance (\itn=50\rm)','check_dominance_n_50', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(4,:,2), structure_averages(4,:,:,2)] = plot_results('check_dominance', 500,'Check dominance (\itn=500\rm)','check_dominance_n_500', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(4,:,3), structure_averages(4,:,:,3)] = plot_results('check_dominance', 5000,'Check dominance (\itn=5000\rm)','check_dominance_n_5000', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(5,:,1), structure_averages(5,:,:,1)] = plot_results('query_and_iterate', 50,'Query and iterate (\itn=50\rm)','query_n_50', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(5,:,2), structure_averages(5,:,:,2)] = plot_results('query_and_iterate', 500,'Query and iterate (\itn=500\rm)','query_n_500', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(5,:,3), structure_averages(5,:,:,3)] = plot_results('query_and_iterate', 5000,'Query and iterate (\itn=5000\rm)','query_n_5000', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(6,:,1), structure_averages(6,:,:,1)] = plot_results('nearest_and_iterate', 50,'Find nearest 5 and iterate (\itn=50\rm)','nearest_n_50', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(6,:,2), structure_averages(6,:,:,2)] = plot_results('nearest_and_iterate', 500,'Find nearest 5 and iterate (\itn=500\rm)','nearest_n_500', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(6,:,3), structure_averages(6,:,:,3)] = plot_results('nearest_and_iterate', 5000,'Find nearest 5 and iterate (\itn=5000\rm)','nearest_n_5000', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(7,:,1), structure_averages(7,:,:,1)] = plot_results('calculate_igd', 50,'IGD (\itn=50\rm)','igd_n_50', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(7,:,2), structure_averages(7,:,:,2)] = plot_results('calculate_igd', 500,'IGD (\itn=500\rm)','igd_n_500', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);
[group_averages(7,:,3), structure_averages(7,:,:,3)] = plot_results('calculate_igd', 5000,'IGD (\itn=5000\rm)','igd_n_5000', function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time);

% dimension averages per operation
unique_dimensions = [1,2,3,5,9,13];
unique_sizes = [50, 500, 5000];
close all;
for i=1:3
heatmap(unique_dimensions, {'Construct', 'Insert', 'Delete', 'Dominance', 'Query', 'Nearest', 'IGD'}, group_averages(:,:,i));
xlabel('m')
title(['Average time per operation (n = ', num2str(unique_sizes(i)), ')'])
saveas(gcf,['op_averages_n_', num2str(unique_sizes(i))],'epsc')
saveas(gcf,['op_averages_n_', num2str(unique_sizes(i))],'png')
end

% structure averages per operation
for i=1:3
for j=1:6
heatmap({'L','Q','K','B','R','*'}, {'Construct', 'Insert', 'Delete', 'Dominance', 'Query', 'Nearest', 'IGD'}, structure_averages(:,:,j,i));
xlabel('Data Structure')
title(['Normalized time per operation (n = ', num2str(unique_sizes(i)), ', m=', num2str(unique_dimensions(j)),')'])
saveas(gcf,['structure_averages_n_', num2str(unique_sizes(i)), '_m_', num2str(unique_dimensions(j))],'epsc')
saveas(gcf,['structure_averages_n_', num2str(unique_sizes(i)), '_m_', num2str(unique_dimensions(j))],'png')
end
        end
end

if (plot_hv_time)
plot_hv_time_heatmap('calculate_hypervolume', 'Hypervolume m=1', 'hypervolume_m_1', 1, function_names, dimensions, data_structures, pareto_sizes, samples, aggregate_types, time);
plot_hv_time_heatmap('calculate_hypervolume', 'Hypervolume m=2', 'hypervolume_m_2', 2, function_names, dimensions, data_structures, pareto_sizes, samples, aggregate_types, time);
plot_hv_time_heatmap('calculate_hypervolume', 'Hypervolume m=3', 'hypervolume_m_3', 3, function_names, dimensions, data_structures, pareto_sizes, samples, aggregate_types, time);
plot_hv_time_heatmap('calculate_hypervolume', 'Hypervolume m=5', 'hypervolume_m_5', 5, function_names, dimensions, data_structures, pareto_sizes, samples, aggregate_types, time);
plot_hv_time_heatmap('calculate_hypervolume', 'Hypervolume m=9', 'hypervolume_m_9', 9, function_names, dimensions, data_structures, pareto_sizes, samples, aggregate_types, time);
end

if (plot_hv_gap || plot_hv_front)
[dimensions2, pareto_sizes2, hv_samples, hvolumes] = read_hv_benchmark();
end

if (plot_hv_gap)
plot_hv_heatmap('Hypervolume Gap', 'hypervolume_gap', dimensions2, pareto_sizes2, hv_samples, hvolumes);
end

        close all

        function [group_median, structure_avgs] = plot_results(function_name, pareto_size, title_str, filename, function_names, dimensions, data_structures, pareto_sizes, aggregate_types, time)
                                                  % Usually 6 dimensions m=[1,2,3,5,9,13]
groups = unique(dimensions(ismember(function_names, function_name) & pareto_sizes == pareto_size & ismember(aggregate_types, '')));
n_groups = size(groups,2);
group_averages = zeros(1,n_groups);
group_median = zeros(1,n_groups);
% {'vector_tree_tag'}    {'kd_tree_tag'}    {'quad_tree_tag'}    {'boost_tree_tag'}    {'r_tree_tag'}    {'r_star_tree_tag'}
structures = {'vector_tree_tag', 'quad_tree_tag', 'kd_tree_tag', 'boost_tree_tag', 'r_tree_tag', 'r_star_tree_tag'};
n_structures = size(structures,2);
structure_avgs = zeros(n_structures, n_groups);
% 30 replicates in each experiment
replicates = time(ismember(function_names, function_name) & pareto_sizes == pareto_size & ismember(aggregate_types, '') & ismember(data_structures, structures{1}) & dimensions==1);
n_replicates = size(replicates,1);
% multi-dimensional matrix with all the data
y = zeros(n_replicates, n_groups, n_structures);
x = {'\itm=1','\itm=2','\itm=3','\itm=5','\itm=9','\itm=13'};
for group=1:n_groups
for structure=1:n_structures
y(:, group, structure) = time(ismember(function_names, function_name) & pareto_sizes == pareto_size & ismember(aggregate_types, '') & ismember(data_structures, structures{structure}) & dimensions==groups(group));
structure_avgs(structure, group) = mean(y(:, group, structure));
end
group_averages(group) = mean(mean(y(:,group,:)));
group_median(group) = mean(reshape(y(:,group,:),1,30*6));
group_max = max(max(y(:,group,:)));
group_min = min(min(y(:,group,:)));
y(:,group,:) = (y(:,group,:) - group_min) ./ (group_max - group_min);
structure_avgs(:, group) = ( structure_avgs(:, group) - group_min ) ./ (group_max - group_min);
end
        figure;
%figure('Renderer', 'painters', 'Position', [10 10 900 600])
% https://www.mathworks.com/matlabcentral/fileexchange/46545-alternative-box-plot?s_tid=mwa_osa_a
h = iosr.statistics.boxPlot(x, y,...
'symbolColor','k',...
'medianColor','k',...
'showMean', true,...
'boxcolor',{[0.6 0.6 1]; [1 0.6 0.6]; [0.6 1.0 0.6]; [1.0 0.7 0.2]; [1.0 1.0 0.4]; [0.8 0.4 0.8];},...
'groupLabels',{'L','Q','K','B','R','*'},...
'showScatter',true,...
'scatterAlpha', 0.2,...
'scatterColor', {[0.0 0.0 0.5]; [0.5 0.0 0.0]; [0.0 0.5 0.0]; [0.5 0.4 0.0]; [0.5 0.5 0]; [0.5 0.0 0.5];},...
'scaleWidth',true,...
'style','hierarchy',...
'xSeparator',true,...
'xSpacing', 'equal',...
'showLegend',false);
h.handles.fig.Position(3) = h.handles.fig.Position(3)*2;
title(title_str);
ylabel('Time');
yticks([0 1]);
yticklabels({'min','max'});
box on;
saveas(gcf,filename,'epsc')
saveas(gcf,filename,'png')
disp(title_str);
% for each group / dimension
for i=1:size(y,2)
p = anova2(reshape(y(:,i,:),30,6),1,'off');
disp(['i: ', num2str(i), ', anova p-value: ', num2str(p(1)), ' group average (ns): ', num2str(group_averages(i))]);
end
        end

function plot_hv_time_heatmap(function_name, title_str, filename, dimension, function_names, dimensions, data_structures, pareto_sizes, samples, aggregate_types, time)
% Usually 4 samples sizes (0,100,1000,10000)
unique_sample_sizes = unique(samples(dimensions == dimension & ismember(function_names, function_name) & ismember(aggregate_types, '')));
n_sample_sizes = size(unique_sample_sizes,2);

% {'vector_tree_tag'}    {'kd_tree_tag'}    {'quad_tree_tag'}    {'boost_tree_tag'}    {'r_tree_tag'}    {'r_star_tree_tag'}
structures = {'vector_tree_tag', 'quad_tree_tag', 'kd_tree_tag', 'boost_tree_tag', 'r_tree_tag', 'r_star_tree_tag'};
n_structures = size(structures,2);

% Usually 3 pareto sizes (50,500,5000)
unique_pareto_sizes = unique(pareto_sizes(dimensions == dimension & ismember(function_names, function_name) & ismember(aggregate_types, '')));
n_pareto_sizes = size(unique_pareto_sizes,2);

% multi-dimensional matrix with all the data
y = zeros(n_pareto_sizes * n_sample_sizes, n_structures);

for pareto_size=1:n_pareto_sizes
for sample_size=1:n_sample_sizes
for structure=1:n_structures
% size(time(ismember(data_structures, structure) & dimensions == unique_dimensions(i) & pareto_sizes == unique_pareto_sizes(j) & samples == unique_sample_sizes(k) & ismember(function_names, function_name) & ismember(aggregate_types, '')))
line = (pareto_size-1)*n_sample_sizes+sample_size;
replicates = time(ismember(data_structures, structures(structure)) & dimensions == dimension & pareto_sizes == unique_pareto_sizes(pareto_size) & samples == unique_sample_sizes(sample_size) & ismember(function_names, function_name) & ismember(aggregate_types, ''));
% disp(['struct:', structures(structure), ' n=', unique_pareto_sizes(pareto_size), ' s=', unique_sample_sizes(sample_size), ' replicates:', size(replicates,1)]);
if (unique_sample_sizes(sample_size) ~= 0)
y(line, structure) = mean(replicates);
else
if (structure == 1)
y(line, structure) = mean(replicates);
else
y(line, structure) = y(line, 1);
end
        end
end
        end
end

        x_labels = {'L','Q','K','B','R','*'};
y_labels = cell(1, n_pareto_sizes * n_sample_sizes);
for i=1:n_pareto_sizes
for j=1:n_sample_sizes
        y_labels{((i-1)*n_sample_sizes)+j} = strcat('n=', num2str(unique_pareto_sizes(i)), ', s=', num2str(unique_sample_sizes(j)));
end
        end

figure;
heatmap(x_labels, y_labels, y, 'ColorScaling','log');
title(title_str);
xlabel('Data structure');
ylabel('Pareto Sizes and Hypervolume Samples');
saveas(gcf,filename,'epsc')
saveas(gcf,filename,'png')
end

        function plot_hv_heatmap(title_str, filename, dimensions, pareto_sizes, hv_samples, hvolumes)
% Table / heatmap
% Columns: m
% Lines: size * samples
% Values: gap

        unique_dimensions = unique(dimensions);
n_dimensions = size(unique_dimensions,1);

y = zeros(12, n_dimensions);
x_labels = unique_dimensions;
y_labels = cell(1, 12);

for dimension=1:n_dimensions
        unique_pareto_sizes = unique(pareto_sizes(dimensions == unique_dimensions(dimension)));
n_pareto_sizes = size(unique_pareto_sizes,1);
unique_hv_samples = unique(hv_samples(dimensions == unique_dimensions(dimension)));
n_hv_samples = size(unique_hv_samples,1);
line = 1;
for pareto_size=1:n_pareto_sizes
        exact_hv = 0;
for hv_sample=1:n_hv_samples
        replicates = hvolumes(dimensions == unique_dimensions(dimension) & pareto_sizes == unique_pareto_sizes(pareto_size) & hv_samples == unique_hv_samples(hv_sample));
if (unique_hv_samples(hv_sample) == 0)
exact_hv = mean(replicates(1:30));
end
        deviation = abs((replicates(1:30) / exact_hv)-1);
if (unique_hv_samples(hv_sample) ~= 0)
y(line, dimension) = mean(deviation);
else
% deviation of exact hv is 0
% this removes approximation errors to avoid 4.4441e-16
% in our heatmap
y(line, dimension) = 0;
end
if (isempty(y_labels{line}))
if (unique_pareto_sizes(pareto_size) == 50)
y_labels{line} = strcat('n=', num2str(unique_pareto_sizes(pareto_size)), ' s=', num2str(unique_hv_samples(hv_sample)));
elseif ((unique_pareto_sizes(pareto_size) == 100) || (unique_pareto_sizes(pareto_size) == 500))
y_labels{line} = strcat('n={100,500} s=', num2str(unique_hv_samples(hv_sample)));
else
y_labels{line} = strcat('n={200,5000} s=', num2str(unique_hv_samples(hv_sample)));
end
        end
line = line + 1;
end
        end
end

        figure;
heatmap(x_labels, y_labels, y);
title(title_str);
xlabel('Dimensions');
ylabel('Pareto Sizes and Hypervolume Samples');
saveas(gcf,filename,'epsc')
saveas(gcf,filename,'png')
end

        function [function_names, dimensions, data_structures, pareto_sizes, samples, aggregate_types, time] = read_data_structure_benchmark()
                                                                                                               % Import data
        filename = 'data_structure_benchmark.csv';
delimiter = ',';
startRow = 11;

% Format for each line
% column4: double (%f)
formatSpec = '%q%*q%f%*q%*s%*s%*s%*s%*s%*s%[^\n\r]';

% Open file
fileID = fopen(filename,'r');

% Read columns
dataArray = textscan(fileID, formatSpec, 'Delimiter', delimiter, 'TextType', 'string', 'EmptyValue', NaN, 'HeaderLines' ,startRow-1, 'ReturnOnError', false, 'EndOfLine', '\r\n');

% Close file.
fclose(fileID);

% Get the tokens (function name, dimensions, data structure, size)
benchmark_description = dataArray{1};
[ignore, tokens] = regexp(benchmark_description, '^([^<]+)<(\d+), ([^>]+)>/(\d+)/(\d+)?/?.*iterations:\d+/?[^:]*:?8?(.*)', 'match', 'tokens');
N = size(dataArray{1},1);
function_names = cell(1,N);
dimensions = zeros(1,N);
data_structures = cell(1,N);
pareto_sizes = zeros(1,N);
samples = zeros(1,N);
aggregate_types = cell(1,N);
for i=1:N
        function_names{i} = tokens{i}{1}{1};
dimensions(i) = str2double(tokens{i}{1}{2});
data_structures{i} = tokens{i}{1}{3};
pareto_sizes(i) = str2double(tokens{i}{1}{4});
samples(i) = str2double(tokens{i}{1}{5});
aggregate_types{i} = tokens{i}{1}{6};
end

time = dataArray{2};

% Clear temporary variables
        clearvars filename delimiter startRow formatSpec fileID dataArray ans;
end

        function [dimensions, pareto_sizes, hv_samples, hvolumes] = read_hv_benchmark()
                                                                    % Import data
        filename = 'hv_benchmark.csv';
delimiter = ',';
startRow = 11;

% Format for each line
% column4: double (%f)
formatSpec = '%q%*q%f%*q%*s%*s%*s%*s%*s%*s%[^\n\r]';

% Open file
fileID = fopen(filename,'r');

% Read columns
dataArray = textscan(fileID, '%*s%*s%d%*s%d%d%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%f%*[^\n\r]', 'Delimiter', {',','"','<','>','/'}, 'TextType', 'string', 'HeaderLines' ,startRow-1, 'ReturnOnError', false, 'EndOfLine', '\r\n');
dimensions = dataArray{1};
pareto_sizes = dataArray{2};
hv_samples = dataArray{3};
hvolumes = dataArray{4};

% Close file.
fclose(fileID);

% Clear temporary variables
        clearvars filename delimiter startRow formatSpec fileID dataArray ans;
end