
re = 6378.137;          % km
mu = 398600;            % km^3/s^2

alpha   = 1; % globe transparency level, 1 = opaque, through 0 = invisible
% Earth texture image
% Anything imread() will handle), but needs to be a 2:1 unprojected globe
% image.
% This is from NASA's Visible Earth site (http://visibleearth.nasa.gov/)
% The actual image link will likely move over time. This one is the 260KB
% version from this page: http://visibleearth.nasa.gov/view_rec.php?id=2430
image_file = 'land_ocean_ice_2048.jpg';
% Create figure
%figure;
hold on;
% Turn off the normal axes
set(gca, 'NextPlot','add', 'Visible','off');
axis equal;
axis auto;

axis vis3d;
% Create wireframe globe
% Create a 3D meshgrid of the sphere points using the ellipsoid function
[x, y, z] = ellipsoid(0, 0, 0, 1000*re, 1000*re, 1000*re);
globe = surf(x, y, -z, 'FaceColor', 'none', 'EdgeColor', 0.5*[1 1 1]);
% globe = surf(xs,ys,-zs, 'FaceColor', 'none', 'EdgeColor', 0.5*[1 1 1]);
% Load Earth image for texture map
cdata = imread(image_file);
% Set image as color data (cdata) property, and set face color to indicate
% a texturemap, which Matlab expects to be in cdata. Turn off the mesh edges.
set(globe, 'FaceColor', 'texturemap', 'CData', cdata, 'FaceAlpha', alpha, 'EdgeColor', 'none');

ax = axesm ('globe', 'Geoid',almanac('earth','radius','m'));

% display ground track
plot3m(data(:,6), data(:,7), (data(:,8)), 'b', 'LineWidth', 1.5);


set(gcf,'Renderer','OpenGL')