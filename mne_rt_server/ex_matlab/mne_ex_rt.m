clear all;
close all;
clc;

addpath('../../../mne-matlab/matlab');
javaaddpath(fileparts(mfilename('fullpath')));

%% connection information
mne_rt_server_ip            =	'localhost';%'172.21.16.63';%
mne_rt_server_commandPort	=	1738;
mne_rt_server_fiffDataPort	=	1739;

%% create command client
t_cmdClient = mne_rt_cmd_client(mne_rt_server_ip, mne_rt_server_commandPort);

%% create data client
t_dataClient = mne_rt_data_client(mne_rt_server_ip, mne_rt_server_fiffDataPort);

%% set data client alias -> for convinience (optional)
t_dataClient.setClientAlias('mne_ex_matlab'); % used in option 2 later on

%% example commands
t_helpInfo = t_cmdClient.sendCommand('help');
fprintf('### Help ###\n%s',t_helpInfo);
t_clistInfo = t_cmdClient.sendCommand('clist');
fprintf('### Client List ###\n%s',t_clistInfo);
t_conInfo = t_cmdClient.sendCommand('conlist');
fprintf('### Connector List ###\n%s',t_conInfo);

%% read meas info
% Option 1
t_aliasOrId = t_dataClient.getClientId();
% Option 2
%t_aliasOrId = 'mne_ex_matlab';
t_cmdClient.requestMeasInfo(t_aliasOrId);
t_measInfo = t_dataClient.readInfo();

%% start measurement
t_cmdClient.requestMeas(t_aliasOrId);

figure;
hold on;
h_old=plot(0,0);
while (true)
    fprintf('read buffer...');
    t_matRawBuffer = t_dataClient.readRawBuffer(t_measInfo.nchan);
    fprintf('(%d channels x %d samples) [done]\r\n', size(t_matRawBuffer,1), size(t_matRawBuffer,2));
    h = plot(t_matRawBuffer');
    delete(h_old);
    h_old = h;
    drawnow;
end

%% close the sockets
t_cmdClient.close();
t_dataClient.close();