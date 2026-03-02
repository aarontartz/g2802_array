clear all;
clc;

addpath('Array Functions F5288v200'); 
addpath('Foundamental Functions F5288v200'); 
addpath('Foundamental Functions F5728 UDC'); 
addpath(genpath('chip_lvl_ctrl'))
addpath(genpath('array_lev_ctrl'))
addpath('/home/wcsng-27/Downloads/mmwave_tb-20d0dc0de1bc32c1ac046c974f2902eafbd55713/hw_ctrl_src/extreme_waves_PAs/arduino_masterControl/matlab_source/uart2spi_lib/'); 

% Verify this is the correct port for the FTDI/Arduino
obj = SPIasUART('/dev/ttyUSB1', 115200);

obj.setPinHigh(2);
obj.setPinLow(2);

%% Initilize beamformer chips biasing
clc;
for chip=0:1:7
   Initialize_BFChip(obj,chip);
end

% Turn beamformer chip ON
clc;
control_BF = 1; % 1 = ON, 0 = OFF

Pol='H';
Turn_AllChannel(obj,Pol,'TX',control_BF);

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
IFVGA1_H = 15;  % IFVGA1: 0~15 15 MAX
IFVGA2_H = 15;  % IFVGA2: 0~15 15 MAX
RFVGA_H  = 0;   % RFVGA:  0~15 0 MAX

SETVGA_UDC_TX(obj,0,IFVGA1_H,IFVGA2_H,RFVGA_H);

%% Load Beamformer Calibration, change gain and Steer the Beam
clc;
Freq = 27e9;

VGA_TX_H = 0;  % VAG: 0~63 0:max

addpath('SN006A_Calibration'); % Specify Unit S/N number

% Load the codebook to the LUT
scan_angles = -60:3:60;

scan_angles2D(1,:) = scan_angles;
scan_angles2D(2,:) = zeros(1,length(scan_angles));

Pol = 'H';
mode = 'TX';
VGA = 0;

obj = load_LUTs(obj, scan_angles, Freq, Pol, mode, VGA);

% Halt execution here to hand control to the FPGA
return;
