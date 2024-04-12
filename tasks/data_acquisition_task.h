#pragma once

#include "../rtos_wrapper/rtos.h"

class DataAcquisitionTask : public wrtos::Task<static_cast<std::size_t>(wrtos::StackDepth::minimal)> {
public:
  virtual void Execute() override;
};