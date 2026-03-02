clear all;
clc;
addpath('Array Functions F5288v200'); 
addpath('Foundamental Functions F5288v200'); 
addpath('Foundamental Functions F5728 UDC'); 
addpath(genpath('chip_lev_ctrl'))
addpath(genpath('array_lev_ctrl'))
addpath('/home/wcsng-27/Downloads/mmwave_tb-20d0dc0de1bc32c1ac046c974f2902eafbd55713/hw_ctrl_src/extreme_waves_PAs/arduino_masterControl/matlab_source/uart2spi_lib/');
%arduino_setup;
obj = SPIasUART('/dev/ttyACM1', 115200);
%%

%writeDigitalPin(due,sprintf('D%d',RST_ARRAY),1);
%writeDigitalPin(due,sprintf('D%d',RST_ARRAY),0);
obj.setPinHigh(2);
obj.setPinLow(2);

%% Set switch period
% out = obj.setSwitchPeriod(10);
% disp(out);
%% Initilize beamformer chips biasing

clc;
for chip=0:1:7

   Initialize_BFChip(obj,chip);

end

% Turn beamformer chip ON/OFF
clc;

control_BF = 1; %1 = ON, 0 = OFF

Pol='H';
Turn_AllChannel(obj,Pol,'RX',control_BF);
%Pol='V';
%Turn_AllChannel(obj,Pol,'RX',control_BF);

Pol='H';
Turn_AllChannel(obj,Pol,'TX',control_BF);
%Pol='V';
%Turn_AllChannel(obj,Pol,'TX',control_BF);

%% Intialize UDC chip Bias and Set UDC gain
clc;
for chip=0:1:1
    Initialize_UDC(obj,chip);
end

% Turn UDC chip ON
clc;
control=1; % 1 = ON, 0 = OFF
for chip=0:1:1 % 0:H 1:V
    
  UDC_ENABLE(obj,chip,control);
  
end

% Change UDC VGA settings:

IFVGA1_H = 15;  %IFVGA1: 0~15 15 MAX
IFVGA2_H = 15;  %IFVGA2: 0~15 15 MAX
RFVGA_H  = 0;   %RFVGA:  0~15 0 MAX

IFVGA1_V = 15;  %IFVGA1: 0~15 15 MAX
IFVGA2_V = 15;  %IFVGA2: 0~15 15 MAX
RFVGA_V  = 0;   %RFVGA:  0~15 0 MAX

SETVGA_UDC_TX(obj,0,IFVGA1_H,IFVGA2_H,RFVGA_H);
%SETVGA_UDC_TX(obj,1,IFVGA1_V,IFVGA2_V,RFVGA_V);

%% Load Beamformer Calibration, change gain and Steer the Beam
clc;
Freq = 27e9;
Az_angle = 0;

VGA_TX_V = 0;  %VAG: 0~63 0:max
VGA_TX_H = 0;  %VAG: 0~63 0:max
VGA_RX_V = 0;  %VAG: 0~63 0:max
VGA_RX_H = 0;  %VAG: 0~63 0:max

addpath('SN006A_Calibration'); % Specify Unit S/N number

% HScan_Array(obj,Az_angle,Freq,'H','TX', VGA_TX_V);
% % HScan_Array(obj,Az_angle,Freq,'H','TX', VGA_TX_H);
% HScan_Array(obj,Az_angle,Freq,'V','RX', VGA_RX_V);
% % HScan_Array(obj,Az_angle,Freq,'H','RX', VGA_RX_H);

% Load the codebook to the LUT
  scan_angles = -60:3:60;

  scan_angles2D(1,:) = scan_angles;
  scan_angles2D(2,:) = zeros(1,length(scan_angles))
% scan_angles = [10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50, 10, -50];
% scan_angles = -50*ones(1, 40);
Pol = 'H';
mode = 'TX';
VGA = 0;
obj = load_LUTs(obj, scan_angles, Freq, Pol, mode, VGA);
return;
%% Set to RX
%obj.setPinLow(10);
obj.setPinHigh(10); %set Tx
%% Steer the beam to angle
clc;
Freq = 27e9;
Az_angle = 0;

VGA_TX_V = 0;  %VAG: 0~63 0:max
VGA_TX_H = 0;  %VAG: 0~63 0:max
VGA_RX_V = 0;  %VAG: 0~63 0:max
VGA_RX_H = 0;  %VAG: 0~63 0:max

addpath('SN006A_Calibration'); % Specify Unit S/N number

% HScan_Array(obj,Az_angle,Freq,'H','TX', VGA_TX_V);
% % HScan_Array(obj,Az_angle,Freq,'H','TX', VGA_TX_H);
%    HScan_Array(obj,Az_angle(angle),Freq,'V','RX', VGA_RX_V);
% % HScan_Array(obj,Az_angle,Freq,'H','RX', VGA_RX_H);

HScan_Array(obj,Az_angle,Freq,'V','RX', VGA_RX_V);
return


%%%-- initial setup done
%% Traditional beam scan
clc;
Freq = 27e9;
Az_angle = [10, -50];

VGA_TX_V = 0;  %VAG: 0~63 0:max
VGA_TX_H = 0;  %VAG: 0~63 0:max
VGA_RX_V = 0;  %VAG: 0~63 0:max
VGA_RX_H = 0;  %VAG: 0~63 0:max

addpath('SN006A_Calibration'); % Specify Unit S/N number

% HScan_Array(obj,Az_angle,Freq,'H','TX', VGA_TX_V);
% % HScan_Array(obj,Az_angle,Freq,'H','TX', VGA_TX_H);
for num=1:10
    for angle = 1:2
        HScan_Array(obj,Az_angle(angle),Freq,'V','RX', VGA_RX_V);
        pause(0.8);
    end
end
% % HScan_Array(obj,Az_angle,Freq,'H','RX', VGA_RX_H);

% HScan_Array(obj,angle,Freq,'V','RX', VGA_RX_V);


%% Reset the LUTs
obj.resetLUT();


%% Perform fast beam scan local
numCBRep = 100;
Pol = 'V';
mode = 'TX';
Mode_Sel(obj, mode); % Check if mode is set without calling mode select
tic
obj.fastBeamScanLocal(Pol, mode, numCBRep);
toc
% disp(length(out)/numCBRep);
%% Perform fast beam scan global
numCBRep = 100;
Pol = 'V';
mode = 'TX';
Mode_Sel(obj, mode); % Check if mode is set without calling mode select
tic
obj.fastBeamScanGlobal(Pol, mode, numCBRep);
toc
% disp(length(out)/numCBRep);
%% Optional: set TX/RX mode through MCU (not recommended)
% 
% mode='TX';
% Mode_Sel(obj,mode);

%% If using SMA FSW port, make sure to set 'D4' and 'D5' back to Digitalinput pin to avoid conflict
% 
% configurePin(due,'D5','DigitalInput');
% configurePin(due,'D4','DigitalInput');



