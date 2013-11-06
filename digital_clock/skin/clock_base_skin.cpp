#include "clock_base_skin.h"

ClockBaseSkin::ClockBaseSkin() {
}

ClockBaseSkin::~ClockBaseSkin() {
}

const ClockBaseSkin::TSkinInfo& ClockBaseSkin::GetInfo() const {
  return info_;
}

void ClockBaseSkin::SetSeparators(const QString& seps) {
  seps_ = seps;
  seps_.remove(' ');
  ProcSeparators();
}