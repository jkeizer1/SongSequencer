#include <cstddef>
#include <new>
#include <math.h>
#include "api.h"
//#include "HighSeqModule.hpp"
//#include "MasterStep.hpp"
//#include "Sequencer.hpp"

//using namespace CLC_Synths;

struct SongSequencer : public _NT_algorithm {
    SongSequencer() {}
    ~SongSequencer() {}
};

// Parameter indices
enum {
    kParamBeatInput,
    kParamPitchCVOutput,
    kParamGateOutput,
    kParamOutputMode,
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

// Enum strings for Output Mode and Switch
static const char* const enumStringsOutputMode[] = {
    "Add",
    "Replace",
    nullptr
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
    {"Output Mode", 0, 1, 0, kNT_unitEnum, kNT_scalingNone, enumStringsOutputMode},
    NT_PARAMETER_CV_INPUT("Seq1 CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq1 Gate Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq2 CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq2 Gate Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq3 CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq3 Gate Input", 0, 0)
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
    kParamGateOutput,
    kParamOutputMode
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

_NT_algorithm* constructSongSequencer(const _NT_algorithmMemoryPtrs& ptrs,
                                      const _NT_algorithmRequirements& req,
                                      const int32_t* specifications) {
    SongSequencer* alg = new (static_cast<void*>(ptrs.sram)) SongSequencer();
    alg->parameters = songSequencerParameters;
    alg->parameterPages = &songSequencerParameterPagesStruct;

    return alg;
}


void stepSongSequencer(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
    SongSequencer* alg = static_cast<SongSequencer*>(self);
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
    nullptr, // parameterChanged,
    stepSongSequencer, // step function
    nullptr, // draw function
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
