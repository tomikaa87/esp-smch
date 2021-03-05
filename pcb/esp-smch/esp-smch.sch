EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "ESP-SMCH"
Date "2021-03-05"
Rev "1"
Comp "tomikaa87"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Module:WeMos_D1_mini U1
U 1 1 6042486B
P 5750 3450
F 0 "U1" H 5750 2561 50  0000 C CNN
F 1 "WeMos_D1_mini" H 5750 2470 50  0000 C CNN
F 2 "Module:WEMOS_D1_mini_light" H 5750 2300 50  0001 C CNN
F 3 "https://wiki.wemos.cc/products:d1:d1_mini#documentation" H 3900 2300 50  0001 C CNN
	1    5750 3450
	1    0    0    -1  
$EndComp
$Comp
L RF:NRF24L01_Breakout U2
U 1 1 60425BA8
P 8700 3350
F 0 "U2" H 9080 3396 50  0000 L CNN
F 1 "NRF24L01_Breakout" H 9080 3305 50  0000 L CNN
F 2 "RF_Module:nRF24L01_Breakout" H 8850 3950 50  0001 L CIN
F 3 "http://www.nordicsemi.com/eng/content/download/2730/34105/file/nRF24L01_Product_Specification_v2_0.pdf" H 8700 3250 50  0001 C CNN
	1    8700 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 3550 7200 3550
Wire Wire Line
	7200 3550 7200 3250
Wire Wire Line
	7200 3250 8200 3250
Wire Wire Line
	6150 3650 7300 3650
Wire Wire Line
	7300 3650 7300 3150
Wire Wire Line
	7300 3150 8200 3150
Wire Wire Line
	6150 3750 7400 3750
Wire Wire Line
	7400 3750 7400 3050
Wire Wire Line
	7400 3050 8200 3050
$Comp
L power:+3V3 #PWR0101
U 1 1 60428E63
P 5850 2450
F 0 "#PWR0101" H 5850 2300 50  0001 C CNN
F 1 "+3V3" H 5865 2623 50  0000 C CNN
F 2 "" H 5850 2450 50  0001 C CNN
F 3 "" H 5850 2450 50  0001 C CNN
	1    5850 2450
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0102
U 1 1 60429482
P 8700 2550
F 0 "#PWR0102" H 8700 2400 50  0001 C CNN
F 1 "+3V3" H 8715 2723 50  0000 C CNN
F 2 "" H 8700 2550 50  0001 C CNN
F 3 "" H 8700 2550 50  0001 C CNN
	1    8700 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 2450 5850 2650
Wire Wire Line
	8700 2550 8700 2750
Wire Wire Line
	8200 3650 7500 3650
Wire Wire Line
	7500 3650 7500 3350
Wire Wire Line
	7500 3350 7100 3350
Wire Wire Line
	7100 3350 7100 3050
Wire Wire Line
	7100 3050 6150 3050
Wire Wire Line
	8200 3550 7600 3550
Wire Wire Line
	7600 3550 7600 3450
Wire Wire Line
	7600 3450 7000 3450
Wire Wire Line
	7000 3450 7000 3150
Wire Wire Line
	7000 3150 6150 3150
Wire Wire Line
	6150 3250 6900 3250
Wire Wire Line
	6900 3250 6900 3850
Wire Wire Line
	6900 3850 7700 3850
Wire Wire Line
	7700 3850 7700 3350
Wire Wire Line
	7700 3350 8200 3350
$Comp
L power:GND #PWR0103
U 1 1 6042BA22
P 5750 4600
F 0 "#PWR0103" H 5750 4350 50  0001 C CNN
F 1 "GND" H 5755 4427 50  0000 C CNN
F 2 "" H 5750 4600 50  0001 C CNN
F 3 "" H 5750 4600 50  0001 C CNN
	1    5750 4600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 6042C385
P 8700 4150
F 0 "#PWR0104" H 8700 3900 50  0001 C CNN
F 1 "GND" H 8705 3977 50  0000 C CNN
F 2 "" H 8700 4150 50  0001 C CNN
F 3 "" H 8700 4150 50  0001 C CNN
	1    8700 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 4250 5750 4600
Wire Wire Line
	8700 3950 8700 4150
$Comp
L Device:C_Polarized C2
U 1 1 6042D4EC
P 4500 3450
F 0 "C2" H 4618 3496 50  0000 L CNN
F 1 "1000u" H 4618 3405 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D8.0mm_P3.50mm" H 4538 3300 50  0001 C CNN
F 3 "~" H 4500 3450 50  0001 C CNN
	1    4500 3450
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0105
U 1 1 6042FCBC
P 4500 3200
F 0 "#PWR0105" H 4500 3050 50  0001 C CNN
F 1 "+3V3" H 4515 3373 50  0000 C CNN
F 2 "" H 4500 3200 50  0001 C CNN
F 3 "" H 4500 3200 50  0001 C CNN
	1    4500 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 604301B3
P 4500 3700
F 0 "#PWR0106" H 4500 3450 50  0001 C CNN
F 1 "GND" H 4505 3527 50  0000 C CNN
F 2 "" H 4500 3700 50  0001 C CNN
F 3 "" H 4500 3700 50  0001 C CNN
	1    4500 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 3200 4500 3300
Wire Wire Line
	4500 3600 4500 3700
$Comp
L Device:C C1
U 1 1 60431047
P 3950 3450
F 0 "C1" H 4065 3496 50  0000 L CNN
F 1 "100n" H 4065 3405 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm" H 3988 3300 50  0001 C CNN
F 3 "~" H 3950 3450 50  0001 C CNN
	1    3950 3450
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR0107
U 1 1 60431986
P 3950 3200
F 0 "#PWR0107" H 3950 3050 50  0001 C CNN
F 1 "+3V3" H 3965 3373 50  0000 C CNN
F 2 "" H 3950 3200 50  0001 C CNN
F 3 "" H 3950 3200 50  0001 C CNN
	1    3950 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 60431CC4
P 3950 3700
F 0 "#PWR0108" H 3950 3450 50  0001 C CNN
F 1 "GND" H 3955 3527 50  0000 C CNN
F 2 "" H 3950 3700 50  0001 C CNN
F 3 "" H 3950 3700 50  0001 C CNN
	1    3950 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 3200 3950 3300
Wire Wire Line
	3950 3600 3950 3700
$Comp
L Mechanical:MountingHole H1
U 1 1 60443274
P 4050 2450
F 0 "H1" H 4150 2496 50  0000 L CNN
F 1 "MountingHole" H 4150 2405 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2_Pad" H 4050 2450 50  0001 C CNN
F 3 "~" H 4050 2450 50  0001 C CNN
	1    4050 2450
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 60443B8E
P 4050 2650
F 0 "H2" H 4150 2696 50  0000 L CNN
F 1 "MountingHole" H 4150 2605 50  0000 L CNN
F 2 "MountingHole:MountingHole_2.2mm_M2_Pad" H 4050 2650 50  0001 C CNN
F 3 "~" H 4050 2650 50  0001 C CNN
	1    4050 2650
	1    0    0    -1  
$EndComp
$Comp
L Graphic:Logo_Open_Hardware_Small LOGO1
U 1 1 6044CC03
P 7300 2600
F 0 "LOGO1" H 7300 2875 50  0001 C CNN
F 1 "Logo_Open_Hardware_Small" H 7300 2375 50  0001 C CNN
F 2 "Symbol:OSHW-Logo_5.7x6mm_Copper" H 7547 2625 50  0000 L CNN
F 3 "~" H 7300 2600 50  0001 C CNN
	1    7300 2600
	1    0    0    -1  
$EndComp
$EndSCHEMATC
