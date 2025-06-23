#include <cstddef>
#include <new>
#include <math.h>
#include "api.h"
#include "HighSeqModule.hpp"
#include "MasterStep.hpp"
#include "Sequencer.hpp"

using namespace CLC_Synths;

struct SongSequencer : public _NT_algorithm {
    SongSequencer() {}
    ~SongSequencer() {}
    HighSeqModule highSeqModule;
    int sequencerCVInput[HighSeqModule::NUM_SEQUENCERS];   // CV input bus index for each sequencer (-1 = unassigned)
    int sequencerGateInput[HighSeqModule::NUM_SEQUENCERS]; // Gate input bus index for each sequencer (-1 = unassigned)
};

// constants
static const int PARAMS_PER_SEQUENCER = 2;
static const int PARAMS_PER_MASTERSTEP = 3;

// Parameter indices
enum {
    kParamBeatInput,
    kParamPitchCVOutput,
    kParamGateOutput,
    kParamSeq1CVInput,
    kParamSeq1GateInput,
    kParamSeq2CVInput,
    kParamSeq2GateInput,
    kParamSeq3CVInput,
    kParamSeq3GateInput,
    kParamSeq4CVInput,
    kParamSeq4GateInput,
    kParamSeq5CVInput,
    kParamSeq5GateInput,
    kParamSeq1BeatsPerBar,
    kParamSeq1Bars,
    kParamSeq2BeatsPerBar,
    kParamSeq2Bars,
    kParamSeq3BeatsPerBar,
    kParamSeq3Bars,
    kParamSeq4BeatsPerBar,
    kParamSeq4Bars,
    kParamSeq5BeatsPerBar,
    kParamSeq5Bars,
    kParamStep1Seq,
    kParamStep1Repeats,
    kParamStep1Switch,
    kParamStep2Seq,
    kParamStep2Repeats,
    kParamStep2Switch,
    kParamStep3Seq,
    kParamStep3Repeats,
    kParamStep3Switch,
    kParamStep4Seq,
    kParamStep4Repeats,
    kParamStep4Switch,
    kParamStep5Seq,
    kParamStep5Repeats,
    kParamStep5Switch,
    kParamStep6Seq,
    kParamStep6Repeats,
    kParamStep6Switch,
    kParamStep7Seq,
    kParamStep7Repeats,
    kParamStep7Switch,
    kParamStep8Seq,
    kParamStep8Repeats,
    kParamStep8Switch
};

static const char* const enumStringsSwitch[] = {
    "Off",
    "On",
    nullptr
};

// Parameter definitions
static const _NT_parameter songSequencerParameters[] = {
    NT_PARAMETER_AUDIO_INPUT("Beat Input", 1, 1)
    NT_PARAMETER_CV_OUTPUT("Pitch CV Output", 1, 2)
    NT_PARAMETER_CV_OUTPUT("Gate Output", 1, 3)
    NT_PARAMETER_CV_INPUT("Seq1 CV Input", 1, 5)
    NT_PARAMETER_CV_INPUT("Seq1 Gate Input", 1, 6)
    NT_PARAMETER_CV_INPUT("Seq2 CV Input", 1, 7)
    NT_PARAMETER_CV_INPUT("Seq2 Gate Input", 1, 8)
    NT_PARAMETER_CV_INPUT("Seq3 CV Input", 1, 9)
    NT_PARAMETER_CV_INPUT("Seq3 Gate Input", 1, 10)
    NT_PARAMETER_CV_INPUT("Seq4 CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq4 Gate Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq5 CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq5 Gate Input", 0, 0)
    {"Seq1 Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq1 Bars", 1, 8, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq2 Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq2 Bars", 1, 8, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq3 Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq3 Bars", 1, 8, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq4 Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq4 Bars", 1, 8, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq5 Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq5 Bars", 1, 8, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step1 Seq", 0, 4, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step1 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step1 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},
    {"Step2 Seq", 0, 4, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step2 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step2 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},
    {"Step3 Seq", 0, 4, 2, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step3 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step3 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},
    {"Step4 Seq", 0, 4, 3, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step4 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step4 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},
    {"Step5 Seq", 0, 4, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step5 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step5 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},
    {"Step6 Seq", 0, 4, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step6 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step6 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},
    {"Step7 Seq", 0, 4, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step7 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step7 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},
    {"Step8 Seq", 0, 4, 2, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step8 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step8 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch}
};

// Parameter pages
static const uint8_t routingPageParams[] = {
    kParamBeatInput,
    kParamPitchCVOutput,
    kParamGateOutput
};
static const uint8_t sequencerAssignPageParams[] = {
    kParamSeq1CVInput,
    kParamSeq1GateInput,
    kParamSeq2CVInput,
    kParamSeq2GateInput,
    kParamSeq3CVInput,
    kParamSeq3GateInput,
    kParamSeq4CVInput,
    kParamSeq4GateInput,
    kParamSeq5CVInput,
    kParamSeq5GateInput
};
static const uint8_t sequencerConfigPageParams[] = {
    kParamSeq1BeatsPerBar,
    kParamSeq1Bars,
    kParamSeq2BeatsPerBar,
    kParamSeq2Bars,
    kParamSeq3BeatsPerBar,
    kParamSeq3Bars,
    kParamSeq4BeatsPerBar,
    kParamSeq4Bars,
    kParamSeq5BeatsPerBar,
    kParamSeq5Bars
};
static const uint8_t stepConfigPageParams[] = {
    kParamStep1Seq,
    kParamStep1Repeats,
    kParamStep1Switch,
    kParamStep2Seq,
    kParamStep2Repeats,
    kParamStep2Switch,
    kParamStep3Seq,
    kParamStep3Repeats,
    kParamStep3Switch,
    kParamStep4Seq,
    kParamStep4Repeats,
    kParamStep4Switch,
    kParamStep5Seq,
    kParamStep5Repeats,
    kParamStep5Switch,
    kParamStep6Seq,
    kParamStep6Repeats,
    kParamStep6Switch,
    kParamStep7Seq,
    kParamStep7Repeats,
    kParamStep7Switch,
    kParamStep8Seq,
    kParamStep8Repeats,
    kParamStep8Switch
};
static const _NT_parameterPage songSequencerParameterPages[] = {
    {"Routing", ARRAY_SIZE(routingPageParams), routingPageParams},
    {"Seq Assign", ARRAY_SIZE(sequencerAssignPageParams), sequencerAssignPageParams},
    {"Seq Config", ARRAY_SIZE(sequencerConfigPageParams), sequencerConfigPageParams},
    {"Step Config", ARRAY_SIZE(stepConfigPageParams), stepConfigPageParams}
};

static const _NT_parameterPages songSequencerParameterPagesStruct = {
    ARRAY_SIZE(songSequencerParameterPages),
    songSequencerParameterPages
};


void assignSequencerParameters (_NT_algorithm* self) {

    SongSequencer* alg = static_cast<SongSequencer*>(self);

    // Initialize highSeqModule with default parameter values
    for (int s = 0; s < alg->highSeqModule.NUM_SEQUENCERS; s++) {
        alg->highSeqModule.sequencers[s].set_beatsPerBar(alg->v[kParamSeq1BeatsPerBar + s * 2]);
        alg->highSeqModule.sequencers[s].set_bars(alg->v[kParamSeq1Bars + s * 2]);
    }
    for (int i = 0; i < alg->highSeqModule.NUM_STEPS; i++) {
        int base = kParamStep1Seq + i * 3;
        alg->highSeqModule.steps[i].set_sequencer(alg->v[base]);
        alg->highSeqModule.steps[i].set_repeats(alg->v[base + 1]);
        alg->highSeqModule.steps[i].set_switch(static_cast<SWITCHSTATE>(alg->v[base + 2]));
    }
}


_NT_algorithm* constructSongSequencer(const _NT_algorithmMemoryPtrs& ptrs,
                                      const _NT_algorithmRequirements& req,
                                      const int32_t* specifications) {
    SongSequencer* alg = new (static_cast<void*>(ptrs.sram)) SongSequencer();
    alg->parameters = songSequencerParameters;
    alg->parameterPages = &songSequencerParameterPagesStruct;

    // Initialize highSeqModule with default parameter values
    for (int s = 0; s < alg->highSeqModule.NUM_SEQUENCERS; s++) {
        alg->highSeqModule.sequencers[s].set_beatsPerBar(alg->v[kParamSeq1BeatsPerBar + s * 2]);
        alg->highSeqModule.sequencers[s].set_bars(alg->v[kParamSeq1Bars + s * 2]);
    }
    for (int i = 0; i < alg->highSeqModule.NUM_STEPS; i++) {
        int base = kParamStep1Seq + i * 3;
        alg->highSeqModule.steps[i].set_sequencer(alg->v[base]);
        alg->highSeqModule.steps[i].set_repeats(alg->v[base + 1]);
        alg->highSeqModule.steps[i].set_switch(static_cast<SWITCHSTATE>(alg->v[base + 2]));
    }

    alg->highSeqModule.assertInitialized();
    return alg;
}

void distributeBeatVoltage (float beatVoltage, SongSequencer* alg) {
    // share beat input across all sequencers (different than VCV rack where each seq. has own beat input)
    for (int sequencer = 0; sequencer < alg->highSeqModule.NUM_SEQUENCERS; sequencer++) {
        if (beatVoltage < 5.0f) {
            alg->highSeqModule.sequencers[sequencer].set_beatState(BEATSTATE::LOW);
        }
        else {  // voltage is HIGH
            if (alg->highSeqModule.sequencers[sequencer].getbeatState() == BEATSTATE::LOW) {
                alg->highSeqModule.sequencers[sequencer].set_beatState(BEATSTATE::FIRSTHIGH);
                // cout << "SET FIRSTHIGH for seq: " << s << endl;
            }
            else if (alg->highSeqModule.sequencers[sequencer].getbeatState() == BEATSTATE::FIRSTHIGH) {
                alg->highSeqModule.sequencers[sequencer].set_beatState(BEATSTATE::STILLHIGH);
                // cout << "FIRSTHIGH --> STILLHIGH for seq: " << s << endl;
            }
            else {
                alg->highSeqModule.sequencers[sequencer].set_beatState(BEATSTATE::STILLHIGH);
                // cout << "SET STILLHIGH for seq: s" << s << endl;
            }
        } // voltage is high
    } // sequencer loop
}

void stepSongSequencer(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
    SongSequencer* alg = static_cast<SongSequencer*>(self);
    int numFrames = numFramesBy4 * 4;
    int beatBusIN = self->v[kParamBeatInput];
    int pitchBusOUT = self->v[kParamPitchCVOutput];
    int gateBusOUT = self->v[kParamGateOutput];

    // Update sequencer input bus assignments
    alg->sequencerCVInput[0] = self->v[kParamSeq1CVInput];
    alg->sequencerGateInput[0] = self->v[kParamSeq1GateInput];
    alg->sequencerCVInput[1] = self->v[kParamSeq2CVInput];
    alg->sequencerGateInput[1] = self->v[kParamSeq2GateInput];
    alg->sequencerCVInput[2] = self->v[kParamSeq3CVInput];
    alg->sequencerGateInput[2] = self->v[kParamSeq3GateInput];
    alg->sequencerCVInput[3] = self->v[kParamSeq4CVInput];
    alg->sequencerGateInput[3] = self->v[kParamSeq4GateInput];
    alg->sequencerCVInput[4] = self->v[kParamSeq5CVInput];
    alg->sequencerGateInput[4] = self->v[kParamSeq5GateInput];

    // Get pointers to input and output memory locations
    float* beatInput = busFrames + beatBusIN * numFrames;
    float* pitchOutput = busFrames + pitchBusOUT * numFrames;
    float* gateOutput = busFrames + gateBusOUT * numFrames;

    // find squencer to read cv and gate from based on the current active step of the mater sequencer
    int masterStep = alg->highSeqModule.getMasterStep();
    int sequencer = 0; // Default to sequencer 0 or handle differently

    // Validate masterstep (it can be -1 if all steps are OFF or possible other error conditions)
    if (masterStep >= 0 && masterStep < HighSeqModule::NUM_STEPS) {
        sequencer = alg->highSeqModule.steps[masterStep].getAssignedSeq();
    }
    else {  // no master step, bail and return: set pitch and gate outputs to 0.0f
        for (int frame = 0; frame < numFrames; frame++) {
            gateOutput[frame] = 0.0f;
            pitchOutput[frame] = 0.0f;
        }
        return;
    }

    // Process busFrames
    for (int frame = 0; frame < numFrames; frame++) {
        // distribute beat input to all 5 sequencers
        distributeBeatVoltage (beatInput[frame], alg);

        // Process sequencer logic
        alg->highSeqModule.process();

        // pitch cv input to pitch output
        float* cvInput; // used for both pitch and gate inputs
        if (alg->sequencerCVInput[sequencer] >= 0 && alg->sequencerCVInput[sequencer] < 28) {
            cvInput = busFrames + alg->sequencerCVInput[sequencer] * numFrames;
            float pitch = cvInput[frame];
            pitchOutput[frame] = pitch;
        } else {
            pitchOutput[frame] = 0.0f; // Fallback if bus is invalid
        }

        // gate cv input to gate output
        if (alg->sequencerGateInput[sequencer] >= 0 && alg->sequencerGateInput[sequencer] < 28) {
            cvInput = busFrames + alg->sequencerGateInput[sequencer] * numFrames;
            float gate = cvInput[frame];
            gateOutput[frame] = gate;
        } else {
            gateOutput[frame] = 0.0f; // fallback if bus is invalid
        }

    } // frame loop

} // step function


void parameterChanged(_NT_algorithm* self, int p) {

    SongSequencer* alg = static_cast<SongSequencer*>(self);

    // Handle sequencer config parameters BEATS PER BAR AND BARS
    for (int s = 0; s < alg->highSeqModule.NUM_SEQUENCERS; s++) {
        if (p == kParamSeq1BeatsPerBar + s * PARAMS_PER_SEQUENCER) {
            alg->highSeqModule.sequencers[s].set_beatsPerBar(self->v[p]);
        } else if (p == kParamSeq1Bars + s * PARAMS_PER_SEQUENCER) {
            alg->highSeqModule.sequencers[s].set_bars(self->v[p]);
        }
    }

    // Handle step config parameters ASSIGNED SEQUENCER AND REPEATS
    for (int i = 0; i < alg->highSeqModule.NUM_STEPS; i++) {
        int base = kParamStep1Seq + i * PARAMS_PER_MASTERSTEP;
        if (p == base) {
            alg->highSeqModule.steps[i].set_sequencer(self->v[p]);
        } else if (p == base + 1) {
            alg->highSeqModule.steps[i].set_repeats(self->v[p]);
        } else if (p == base + 2) {
            alg->highSeqModule.steps[i].set_switch(static_cast<SWITCHSTATE>(self->v[p]));
        }
    }
}

bool drawSongSequencer (_NT_algorithm* self) {
    const SongSequencer* alg = static_cast<const SongSequencer*>(self);
    char buffer[32];

    assignSequencerParameters (self);
    int color = 15;

    // LINE ONE - Basic Info
    int y = 10;
    int y_offset = 7;
    NT_drawText (1, y, "Step:", color, kNT_textLeft, kNT_textTiny);
    NT_intToString(buffer, alg->highSeqModule.getMasterStep());
    NT_drawText(30, y, buffer, color, kNT_textLeft, kNT_textTiny);

    NT_drawText (60, y, "State", color, kNT_textLeft, kNT_textTiny);
    NT_intToString(buffer, alg->highSeqModule.getState());
    NT_drawText(90, y, buffer, color, kNT_textLeft, kNT_textTiny);

    // LINE TWO - Steps Titles Screen is 256x64, Draw steps 1..8
    int offset = 30;
    y += y_offset;
    NT_drawText (1, y, "Step", color, kNT_textLeft, kNT_textTiny);
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        NT_intToString(buffer, step+1);
        NT_drawText (offset * (step+1), y, buffer, color, kNT_textLeft, kNT_textTiny);
    }

    // LINE THREE - Assigned Sequencer
    y += y_offset;
    NT_drawText (1, y, "Seq ", color, kNT_textLeft, kNT_textTiny);
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        NT_intToString(buffer, alg->highSeqModule.steps[step].getAssignedSeq());
        NT_drawText (offset * (step+1), y, buffer, color, kNT_textLeft, kNT_textTiny);
    }

    // LINE FOUR - Switch State
    y += y_offset;
    NT_drawText (1, y, "OnOff", color, kNT_textLeft, kNT_textTiny);
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        NT_intToString(buffer, alg->highSeqModule.steps[step].getOnOffSwitch());
        NT_drawText (offset * (step+1), y, buffer, color, kNT_textLeft, kNT_textTiny);
    }

    return true; //suppress native parameter line
}

void calculateStaticRequirementsSongSequencer(_NT_staticRequirements& req) {
    req.dram = 0; // No static DRAM needed
}

void calculateRequirementsSongSequencer(_NT_algorithmRequirements& req, const int32_t* specifications) {
    req.numParameters = ARRAY_SIZE(songSequencerParameters);
    req.sram = sizeof(SongSequencer);
    req.dram = 5 * 16 * sizeof(float);
    req.dtc = 0;
    req.itc = 0;
}

static const _NT_factory songSequencerFactory = {
    NT_MULTICHAR('C', 'L', 'C', '2'),  // guid
    "Song Sequencer", // name
    "A Sequencer Sequencer",  // descr
    0, // number of specifications
    nullptr, // specifications
    calculateStaticRequirementsSongSequencer,  // static requirements
    nullptr,  // initialise static memory
    calculateRequirementsSongSequencer,  // dynamic requirements
    constructSongSequencer,  // constructor
    parameterChanged,
    stepSongSequencer, // step function
    drawSongSequencer, // draw function
    nullptr, // midirealtime
    nullptr, // midi message
    kNT_tagUtility, // NT tags
    nullptr, // hasCustomUi
    nullptr, // customUI
    nullptr, // setupUI
    nullptr, // serialise
    nullptr // deserialise,
//    nullptr  // midiSysEX --- V9
};

extern "C" {
    uintptr_t pluginEntry(_NT_selector selector, uint32_t data) {
        switch (selector) {
            case kNT_selector_version:
                return kNT_apiVersionCurrent;
            case kNT_selector_numFactories:
                return 1;
            case kNT_selector_factoryInfo:
                if (data == 0)
                    return reinterpret_cast<uintptr_t>(&songSequencerFactory);
            return 0;
            default:
                return 0;
        }
    }
}
