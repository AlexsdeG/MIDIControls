#pragma once

#include <Control_Surface.h>

namespace Config {

constexpr setting_t MAIN_BANK_COUNT = 6;
constexpr setting_t SHIFT_BANK_COUNT = 2;
constexpr setting_t SHIFT_BANK_BASE = 0;
constexpr setting_t SHIFT_BANK_ACTIVE = 1;
constexpr int8_t TRANSPOSE_MIN = -2;
constexpr int8_t TRANSPOSE_MAX = 2;

constexpr pin_t PIN_MATRIX_ROW_1 = 5;
constexpr pin_t PIN_MATRIX_ROW_2 = 4;
constexpr pin_t PIN_MATRIX_ROW_3 = 3;
constexpr pin_t PIN_MATRIX_ROW_4 = 2;

constexpr pin_t PIN_MATRIX_COL_1 = 6;
constexpr pin_t PIN_MATRIX_COL_2 = 7;
constexpr pin_t PIN_MATRIX_COL_3 = 8;
constexpr pin_t PIN_MATRIX_COL_4 = 9;

constexpr pin_t PIN_BLUE_BUTTON = 12;
constexpr pin_t PIN_RED_BUTTON = 11;
constexpr pin_t PIN_YELLOW_BUTTON = 10;

constexpr pin_t PIN_BLUE_LED = A4;
constexpr pin_t PIN_RED_LED = A5;

constexpr pin_t PIN_JOYSTICK_X = A3;
constexpr pin_t PIN_JOYSTICK_Y = A2;

constexpr pin_t PIN_ROTARY_1 = A0;
constexpr pin_t PIN_ROTARY_2 = A1;

constexpr pin_t PIN_IR = 13;

constexpr uint16_t IR_REMOTE_ADDRESS = 0x00;
constexpr uint8_t IR_CMD_PLAY = 0x45;
constexpr uint8_t IR_CMD_STOP = 0x46;
constexpr uint8_t IR_CMD_RECORD = 0x47;
constexpr uint8_t IR_CMD_REWIND = 0x44;
constexpr uint8_t IR_CMD_FAST_FORWARD = 0x40;
constexpr bool IR_IGNORE_REPEAT = true;
constexpr uint16_t IR_MIN_COMMAND_INTERVAL_MS = 120;

constexpr uint16_t JOYSTICK_TRANSPOSE_HIGH_THRESHOLD = 800;
constexpr uint16_t JOYSTICK_TRANSPOSE_LOW_THRESHOLD = 200;
constexpr uint16_t JOYSTICK_TRANSPOSE_RELEASE_LOW = 350;
constexpr uint16_t JOYSTICK_TRANSPOSE_RELEASE_HIGH = 650;

constexpr PinList<4> MATRIX_ROW_PINS = {
    PIN_MATRIX_ROW_1,
    PIN_MATRIX_ROW_2,
    PIN_MATRIX_ROW_3,
    PIN_MATRIX_ROW_4,
};

constexpr PinList<4> MATRIX_COL_PINS = {
    PIN_MATRIX_COL_1,
    PIN_MATRIX_COL_2,
    PIN_MATRIX_COL_3,
    PIN_MATRIX_COL_4,
};

constexpr AddressMatrix<4, 4> BANK_0_PIANO = {{
    {48, 49, 50, 51},
    {52, 53, 54, 55},
    {56, 57, 58, 59},
    {60, 61, 62, 63},
}};

constexpr AddressMatrix<4, 4> BANK_1_DRUMS = {{
    {36, 38, 40, 41},
    {43, 45, 47, 48},
    {50, 42, 46, 49},
    {51, 39, 44, 57},
}};

constexpr AddressMatrix<4, 4> BANK_2_DAW = {{
    {80, 81, 82, 83},
    {84, 85, 86, 87},
    {88, 89, 90, 91},
    {92, 93, 94, 95},
}};

constexpr AddressMatrix<4, 4> BANK_3_TRACK = {{
    {16, 17, 18, 19},
    {20, 21, 22, 23},
    {24, 25, 26, 27},
    {28, 29, 30, 31},
}};

constexpr AddressMatrix<4, 4> BANK_4_TRACK = {{
    {32, 33, 34, 35},
    {36, 37, 38, 39},
    {40, 41, 42, 43},
    {44, 45, 46, 47},
}};

constexpr AddressMatrix<4, 4> BANK_5_TRACK = {{
    {64, 65, 66, 67},
    {68, 69, 70, 71},
    {72, 73, 74, 75},
    {76, 77, 78, 79},
}};

constexpr Array<AddressMatrix<4, 4>, MAIN_BANK_COUNT> MATRIX_BANK_ADDRESSES = {{
    BANK_0_PIANO,
    BANK_1_DRUMS,
    BANK_2_DAW,
    BANK_3_TRACK,
    BANK_4_TRACK,
    BANK_5_TRACK,
}};

constexpr Array<MIDIChannelCable, MAIN_BANK_COUNT> MATRIX_BANK_CHANNELS = {{
    Channel_1,
    Channel_1,
    Channel_1,
    Channel_1,
    Channel_1,
    Channel_1,
}};

constexpr Array<MIDIAddress, SHIFT_BANK_COUNT> ROTARY_1_ADDRESSES = {{
    {MIDI_CC::Pan, Channel_1},
    {MIDI_CC::Expression_Controller, Channel_1},
}};

constexpr Array<MIDIAddress, SHIFT_BANK_COUNT> ROTARY_2_ADDRESSES = {{
    {MIDI_CC::Channel_Volume, Channel_1},
    {MIDI_CC::Effect_Control_1, Channel_1},
}};

constexpr Array<MIDIAddress, SHIFT_BANK_COUNT> RED_BUTTON_ADDRESSES = {{
    {MCU::RECORD, Channel_1},
    {MCU::UNDO, Channel_1},
}};

constexpr Array<MIDIAddress, SHIFT_BANK_COUNT> JOYSTICK_Y_ADDRESSES = {{
    {MIDI_CC::Modulation_Wheel, Channel_1},
    MIDIAddress::invalid(),
}};

} // namespace Config
