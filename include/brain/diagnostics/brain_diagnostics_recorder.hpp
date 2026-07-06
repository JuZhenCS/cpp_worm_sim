#pragma once

#include "brain/diagnostics/brain_diagnostics_observe.hpp"

#include <iosfwd>

namespace brain {

void write_brain_summary(const BrainDiagnosticsResult& result, std::ostream& output);

}  // namespace brain
