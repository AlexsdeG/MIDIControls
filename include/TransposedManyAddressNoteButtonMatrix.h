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
    : public Bankable::MIDIButtonMatrix<
          TransposedManyMatrixAddress<NumBanks, NumRows, NumCols>,
          DigitalNoteSender,
          NumRows,
          NumCols> {
 public:
  TransposedManyAddressNoteButtonMatrix(
      const Bank<NumBanks> &bank,
      const PinList<NumRows> &rowPins,
      const PinList<NumCols> &colPins,
      const Array<AddressMatrix<NumRows, NumCols>, NumBanks> &notes,
      const Array<MIDIChannelCable, NumBanks> &channelCNs,
      OutputBankConfig<> transposeConfig,
      uint8_t velocity = 0x7F)
      : Bankable::MIDIButtonMatrix<
            TransposedManyMatrixAddress<NumBanks, NumRows, NumCols>,
            DigitalNoteSender,
            NumRows,
            NumCols>({bank, notes, channelCNs, transposeConfig},
                      rowPins,
                      colPins,
                      {velocity}) {}

  void setVelocity(uint8_t velocity) { this->sender.setVelocity(velocity); }
  uint8_t getVelocity() const { return this->sender.getVelocity(); }
};

}  // namespace MIDIControls
