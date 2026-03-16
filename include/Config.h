#pragma once

#include <Control_Surface.h>

namespace Config {

enum class RedActionMessageType : uint8_t {
    Note,
    ControlChange,
};

constexpr setting_t MAIN_BANK_COUNT = 6;
constexpr setting_t SHIFT_BANK_COUNT = 2;
constexpr setting_t SHIFT_BANK_BASE = 0;
constexpr setting_t SHIFT_BANK_ACTIVE = 1;

constexpr setting_t BANK_PIANO = 0;
constexpr setting_t BANK_DRUM = 1;
constexpr setting_t BANK_CONTROL_1 = 2;
constexpr setting_t BANK_CONTROL_2 = 3;
constexpr setting_t BANK_CONTROL_3 = 4;
constexpr setting_t BANK_CONTROL_4 = 5;

constexpr uint8_t MODE_PIANO = 0;
constexpr uint8_t MODE_DRUM = 1;
constexpr uint8_t MODE_CONTROL = 2;

constexpr unsigned long BLUE_HOLD_THRESHOLD_MS = 1500;
constexpr unsigned long BLUE_SWITCH_FLASH_MS = 120;
constexpr unsigned long BLUE_INDICATE_ON_MS = 120;
constexpr unsigned long BLUE_INDICATE_OFF_MS = 120;

constexpr uint8_t SELECTOR_COL_S1 = 0; 
constexpr uint8_t SELECTOR_COL_S2 = 1;
constexpr uint8_t SELECTOR_COL_S3 = 2;
constexpr uint8_t SELECTOR_COL_S4 = 3;
constexpr uint8_t SELECTOR_ROW_S1 = 0; // Right-most physical row (S1..S4 top-to-bottom)
constexpr uint8_t SELECTOR_ROW_S2 = 1;
constexpr uint8_t SELECTOR_ROW_S3 = 2;
constexpr uint8_t SELECTOR_ROW_S4 = 3;

constexpr uint8_t PIANO_OCTAVE_UP_ROW = SELECTOR_ROW_S1;
constexpr uint8_t PIANO_OCTAVE_UP_COL = SELECTOR_COL_S1;
constexpr uint8_t PIANO_OCTAVE_DOWN_ROW = SELECTOR_ROW_S1;
constexpr uint8_t PIANO_OCTAVE_DOWN_COL = SELECTOR_COL_S2;

// Safe full-octave transpose range for current piano map:
// lowest note 55 can go down 4 octaves -> 7,
// highest note 73 can go up 4 octaves -> 121.
constexpr int8_t TRANSPOSE_MIN = -4;
constexpr int8_t TRANSPOSE_MAX = 4;

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
    {55, 56, 72, 65},
    {70, 73, 71, 64},
    {68, 63, 69, 62},
    {66, 61, 67, 60},
}};

constexpr AddressMatrix<4, 4> BANK_1_DRUMS = {{
    {52, 56, 44, 37},
    {51, 46, 42, 38},
    {49, 47, 40, 35},
    {50, 45, 39, 36},
}};

constexpr AddressMatrix<4, 4> BANK_2_DAW = {{
    {127, 126, 125, 124},
    {123, 122, 121, 120},
    {119, 118, 117, 116},
    {115, 114, 113, 112},
}};

constexpr AddressMatrix<4, 4> BANK_3_TRACK = {{
    {111, 110, 109, 108},
    {107, 106, 105, 104},
    {103, 102, 101, 100},
    {99, 98, 97, 96},
}};

constexpr AddressMatrix<4, 4> BANK_4_TRACK = {{
    {95, 94, 93, 92},
    {91, 90, 89, 88},
    {87, 86, 85, 84},
    {83, 82, 81, 80},
}};

constexpr AddressMatrix<4, 4> BANK_5_TRACK = {{
    {79, 78, 77, 76},
    {75, 74, 73, 72},
    {71, 70, 69, 68},
    {67, 66, 65, 64},
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
    Channel_16,
    Channel_16,
    Channel_16,
    Channel_16,
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
    {MCU::RECORD, Channel_16},
    {MCU::REC_RDY_1, Channel_16},
}};

constexpr RedActionMessageType RED_ACTION_MESSAGE_TYPE =
    RedActionMessageType::Note;

constexpr MIDIAddress RED_RECORD_FEEDBACK_ADDRESS = {MCU::RECORD, Channel_16};
constexpr MIDIAddress RED_MONITOR_FEEDBACK_ADDRESS = {MCU::REC_RDY_1, Channel_16};

constexpr Array<MIDIAddress, SHIFT_BANK_COUNT> JOYSTICK_Y_ADDRESSES = {{
    {MIDI_CC::Modulation_Wheel, Channel_1},
    MIDIAddress::invalid(),
}};

} // namespace Config
