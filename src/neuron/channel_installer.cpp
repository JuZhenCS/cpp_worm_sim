#include "neuron/neuron/channel_installer.hpp"

#include "neuron/channels/cca1.hpp"
#include "neuron/channels/egl36.hpp"
#include "neuron/channels/egl19.hpp"
#include "neuron/channels/egl2.hpp"
#include "neuron/channels/irk.hpp"
#include "neuron/channels/kqt3.hpp"
#include "neuron/channels/kcnl.hpp"
#include "neuron/channels/kvs1.hpp"
#include "neuron/channels/nca.hpp"
#include "neuron/channels/shk1.hpp"
#include "neuron/channels/shl1.hpp"
#include "neuron/channels/slo_coupled.hpp"
#include "neuron/channels/slo1_unc2.hpp"
#include "neuron/channels/unc2.hpp"

#include <memory>

namespace neuron {

void install_nca_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.nca_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<NcaChannel>(compartment.nca_conductance_nS));
        }
    }
}

void install_irk_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.irk_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<IrkChannel>(compartment.irk_conductance_nS));
        }
    }
}

void install_kqt3_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.kqt3_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Kqt3Channel>(compartment.kqt3_conductance_nS));
        }
    }
}

void install_egl2_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.egl2_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Egl2Channel>(compartment.egl2_conductance_nS));
        }
    }
}

void install_shk1_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.shk1_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Shk1Channel>(compartment.shk1_conductance_nS));
        }
    }
}

void install_kvs1_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.kvs1_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Kvs1Channel>(compartment.kvs1_conductance_nS));
        }
    }
}

void install_shl1_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.shl1_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Shl1Channel>(compartment.shl1_conductance_nS));
        }
    }
}

void install_egl36_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.egl36_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Egl36Channel>(compartment.egl36_conductance_nS));
        }
    }
}

void install_egl19_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.egl19_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Egl19Channel>(compartment.egl19_conductance_nS));
        }
    }
}

void install_cca1_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.cca1_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Cca1Channel>(compartment.cca1_conductance_nS));
        }
    }
}

void install_unc2_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.unc2_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Unc2Channel>(compartment.unc2_conductance_nS));
        }
    }
}

void install_kcnl_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.kcnl_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<KcnlChannel>(compartment.kcnl_conductance_nS));
        }
    }
}

void install_slo1_egl19_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.slo1_egl19_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<SloCoupledChannel>(
                "slo1_egl19", compartment.slo1_egl19_conductance_nS, SloKind::Slo1, CalciumPartner::Egl19));
        }
    }
}

void install_slo1_unc2_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.slo1_unc2_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Slo1Unc2Channel>(compartment.slo1_unc2_conductance_nS));
        }
    }
}

void install_slo2_egl19_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.slo2_egl19_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<SloCoupledChannel>(
                "slo2_egl19", compartment.slo2_egl19_conductance_nS, SloKind::Slo2, CalciumPartner::Egl19));
        }
    }
}

void install_slo2_unc2_channels(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        if (compartment.slo2_unc2_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<SloCoupledChannel>(
                "slo2_unc2", compartment.slo2_unc2_conductance_nS, SloKind::Slo2, CalciumPartner::Unc2));
        }
    }
}

void enable_calcium_internal(Cell& cell) {
    for (auto& compartment : cell.compartments) {
        compartment.calcium_internal_enabled = true;
        compartment.cai_uM_per_um2 = 0.05;
    }
}

}  // namespace neuron
