#pragma once

#include <Control_Surface.h>

namespace MIDIControls {

template <setting_t NumBanks, uint8_t NumRows, uint8_t NumCols>
class TransposedManyMatrixAddress {
 public:
  TransposedManyMatrixAddress(
      const Bank<NumBanks> &bank,
      const Array<AddressMatrix<NumRows, NumCols>, NumBanks> &addresses,
      const Array<MIDIChannelCable, NumBanks> &channelCNs,
      OutputBankConfig<> transposeConfig)
      : manyAddresses(bank, addresses, channelCNs), transposer(transposeConfig) {}

  void lock() {
    manyAddresses.lock();
    transposer.lock();
  }

  void unlock() {
    manyAddresses.unlock();
    transposer.unlock();
  }

  MIDIAddress getActiveAddress(uint8_t row, uint8_t col) const {
    return manyAddresses.getActiveAddress(row, col) + transposer.getAddressOffset();
  }

 private:
  Bankable::ManyAddresses::ManyMatrixAddresses<NumBanks, NumRows, NumCols>
      manyAddresses;
  OutputBankableMIDIAddress transposer;
};

template <setting_t NumBanks, uint8_t NumRows, uint8_t NumCols>
class TransposedManyAddressNoteButtonMatrix
    : public MIDIOutputElement,
      public AH::ButtonMatrix<
          TransposedManyAddressNoteButtonMatrix<NumBanks, NumRows, NumCols>,
          NumRows,
          NumCols> {
  using ButtonMatrix =
      AH::ButtonMatrix<TransposedManyAddressNoteButtonMatrix, NumRows, NumCols>;
  friend class AH::ButtonMatrix<TransposedManyAddressNoteButtonMatrix,
                                NumRows,
                                NumCols>;

 public:
  using ButtonEventCallback = bool (*)(uint8_t row, uint8_t col, bool pressed);

  TransposedManyAddressNoteButtonMatrix(
      const Bank<NumBanks> &bank,
      const PinList<NumRows> &rowPins,
      const PinList<NumCols> &colPins,
      const Array<AddressMatrix<NumRows, NumCols>, NumBanks> &notes,
      const Array<MIDIChannelCable, NumBanks> &channelCNs,
      OutputBankConfig<> transposeConfig,
      uint8_t velocity = 0x7F)
      : ButtonMatrix(rowPins, colPins),
        address({bank, notes, channelCNs, transposeConfig}),
        sender({velocity}) {}

  void begin() override { ButtonMatrix::begin(); }

  void update() override { ButtonMatrix::update(); }

  void setVelocity(uint8_t velocity) { sender.setVelocity(velocity); }
  uint8_t getVelocity() const { return sender.getVelocity(); }

  void setButtonEventCallback(ButtonEventCallback callback) {
    eventCallback = callback;
  }

 private:
  void onButtonChanged(uint8_t row, uint8_t col, bool state) {
    const bool pressed = state == LOW;
    bool suppressMIDISend = false;

    if (eventCallback) {
      suppressMIDISend = eventCallback(row, col, pressed);
    }

    if (pressed) {
      if (!activeButtons)
        address.lock();
      activeButtons++;
      if (!suppressMIDISend)
        sender.sendOn(address.getActiveAddress(row, col));
    } else {
      if (!suppressMIDISend)
        sender.sendOff(address.getActiveAddress(row, col));

      if (activeButtons > 0)
        activeButtons--;
      if (!activeButtons)
        address.unlock();
    }
  }

 private:
  TransposedManyMatrixAddress<NumBanks, NumRows, NumCols> address;
  uint16_t activeButtons = 0;
  ButtonEventCallback eventCallback;

 public:
  DigitalNoteSender sender;
};

}  // namespace MIDIControls
