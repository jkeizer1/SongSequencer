#include <cstddef>
#include <new>
#include <math.h>
#include "api.h"
#include "HighSeqModule.hpp"
#include "MasterStep.hpp"
#include "Sequencer.hpp"

using namespace CLC_Synths;

struct _cursor {
    int x;
    int y;
    _cursor () : x(0), y(0) {}
};

struct _cell {
    int row;
    int col;
    _cell () : row(1), col(1) {}
};

struct SongSequencer : public _NT_algorithm {
    SongSequencer() {}
    ~SongSequencer() {}
    HighSeqModule highSeqModule;

    int sequencerCVInput[HighSeqModule::NUM_SEQUENCERS];      // CV input bus index for each sequencer (-1 = unassigned)
    int sequencerGateInput[HighSeqModule::NUM_SEQUENCERS];    // Gate input bus index for each sequencer (-1 = unassigned)
    int sequencerResetOutput[HighSeqModule::NUM_SEQUENCERS];    // Reset output bus index for each sequencer (-1 = unassigned)
    int sequencerSelectOutput[HighSeqModule::NUM_SEQUENCERS];    // NT Step Sequencer Select for each sequencer (-1 = unassigned)
    int sequencerTransposeInput[HighSeqModule::NUM_SEQUENCERS];    // Transpose input bus index for each sequencer (-1 = unassigned)
    int sequencerCVAssignableInput[HighSeqModule::NUM_SEQUENCERS];    // Assignable CV input bus for each sequencer (-1 = unassigned)
    bool editMode;

    bool triggerActive;
    int triggerFrameCounter;
    bool triggerHandled;
    float SAMPLE_RATE = 48000;
    float FRAME_TIME_MS = (1.f/SAMPLE_RATE) * 1000.f;
    float TRIGGER_FRAME_TARGET_MS = 25.0f;
    float TRIGGER_FRAMES_NEEDED = TRIGGER_FRAME_TARGET_MS / FRAME_TIME_MS;

    bool resetdebug;
    bool resetdebugever;

    float selectorVoltsOut;

//    _NT_uiData lastUiData; // Store last UI data for debugging

    float lastBeatVoltage; // for debugging
    float debugVal;
    _cell cell;
};

// constants
static const int PARAMS_PER_SEQUENCER = 2;
static const int PARAMS_PER_MASTERSTEP = 3;
static const float SEQ12THV = 1.f/12.f;  // 1 12th of a volt to provide volts per octave note increments

// Parameter indices
enum {
    kParamResetInput,
    kParamBeatInput,
    kParamPitchCVOutput,
    kParamGateOutput,
    kParamAssignableOutput,

    kParamSeq1CVInput,
    kParamSeq1GateInput,
    kParamSeq1ResetOutput,
    kParamSeq1SeqSelectOutput,
    kParamSeq1SeqSelectValue,
    kParamSeq1TransposeInput,
    kParamSeq1AssignableCVInput,

    kParamSeq2CVInput,
    kParamSeq2GateInput,
    kParamSeq2ResetOutput,
    kParamSeq2SeqSelectOutput,
    kParamSeq2SeqSelectValue,
    kParamSeq2TransposeInput,
    kParamSeq2AssignableCVInput,

    kParamSeq3CVInput,
    kParamSeq3GateInput,
    kParamSeq3ResetOutput,
    kParamSeq3SeqSelectOutput,
    kParamSeq3SeqSelectValue,
    kParamSeq3TransposeInput,
    kParamSeq3AssignableCVInput,

    kParamSeq4CVInput,
    kParamSeq4GateInput,
    kParamSeq4ResetOutput,
    kParamSeq4SeqSelectOutput,
    kParamSeq4SeqSelectValue,
    kParamSeq4TransposeInput,
    kParamSeq4AssignableCVInput,

    kParamSeq5CVInput,
    kParamSeq5GateInput,
    kParamSeq5ResetOutput,
    kParamSeq5SeqSelectOutput,
    kParamSeq5SeqSelectValue,
    kParamSeq5TransposeInput,
    kParamSeq5AssignableCVInput,

    kParamSeq6CVInput,
    kParamSeq6GateInput,
    kParamSeq6ResetOutput,
    kParamSeq6SeqSelectOutput,
    kParamSeq6SeqSelectValue,
    kParamSeq6TransposeInput,
    kParamSeq6AssignableCVInput,

    kParamSeq7CVInput,
    kParamSeq7GateInput,
    kParamSeq7ResetOutput,
    kParamSeq7SeqSelectOutput,
    kParamSeq7SeqSelectValue,
    kParamSeq7TransposeInput,
    kParamSeq7AssignableCVInput,

    kParamSeq8CVInput,
    kParamSeq8GateInput,
    kParamSeq8ResetOutput,
    kParamSeq8SeqSelectOutput,
    kParamSeq8SeqSelectValue,
    kParamSeq8TransposeInput,
    kParamSeq8AssignableCVInput,

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

    kParamSeq6BeatsPerBar,
    kParamSeq6Bars,

    kParamSeq7BeatsPerBar,
    kParamSeq7Bars,

    kParamSeq8BeatsPerBar,
    kParamSeq8Bars,

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

// Enum strings for Output Mode
static const char* const enumStringsSequencers[] = {
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    nullptr  // Null terminator
};


// Parameter definitions
static const _NT_parameter songSequencerParameters[] = {
    NT_PARAMETER_AUDIO_INPUT("Reset Input", 0, 1)   /* 0 is none */
    NT_PARAMETER_AUDIO_INPUT("Beat Input", 0, 2)
    NT_PARAMETER_CV_OUTPUT("Pitch CV Output", 0, 13) // Output 1
    NT_PARAMETER_CV_OUTPUT("Gate Output", 0, 14)     // Output 2
    NT_PARAMETER_CV_OUTPUT("Assignable Output", 0, 0)

    NT_PARAMETER_CV_INPUT("A CV Input", 0, 3)
    NT_PARAMETER_CV_INPUT("A Gate Input", 0, 4)
    NT_PARAMETER_CV_INPUT("A Reset Output", 0, 18)
    NT_PARAMETER_CV_INPUT("A St.Seq. Output", 0, 0)
    {"Seq A ST Seq", 1, 32, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    NT_PARAMETER_CV_INPUT("A Transpose Input", 0, 0)
    NT_PARAMETER_CV_INPUT("A Assignable CV Input", 0, 0)

    NT_PARAMETER_CV_INPUT("B CV Input", 0, 5)
    NT_PARAMETER_CV_INPUT("B Gate Input", 0, 6)
    NT_PARAMETER_CV_INPUT("B Reset Output", 0, 18)
    NT_PARAMETER_CV_INPUT("B St.Seq. Output", 0, 0)
    {"Seq B ST Seq", 1, 32, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    NT_PARAMETER_CV_INPUT("B Transpose Input", 0, 0)
    NT_PARAMETER_CV_INPUT("B Assignable CV Input", 0, 0)

    NT_PARAMETER_CV_INPUT("C CV Input", 0, 7)
    NT_PARAMETER_CV_INPUT("C Gate Input", 0, 8)
    NT_PARAMETER_CV_INPUT("C Reset Output", 0, 18)
    NT_PARAMETER_CV_INPUT("C St.Seq. Output", 0, 0)
    {"Seq C ST Seq", 1, 32, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    NT_PARAMETER_CV_INPUT("C Transpose Input", 0, 0)
    NT_PARAMETER_CV_INPUT("C Assignable CV Input", 0, 0)

    NT_PARAMETER_CV_INPUT("D CV Input", 0, 9)
    NT_PARAMETER_CV_INPUT("D Gate Input", 0, 10)
    NT_PARAMETER_CV_INPUT("D Reset Output", 0, 18)
    NT_PARAMETER_CV_INPUT("D St.Seq. Output", 0, 0)
    {"Seq D ST Seq", 1, 32, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    NT_PARAMETER_CV_INPUT("D Transpose Input", 0, 0)
    NT_PARAMETER_CV_INPUT("D Assignable CV Input", 0, 0)

    NT_PARAMETER_CV_INPUT("E CV Input", 0, 11)
    NT_PARAMETER_CV_INPUT("E Gate Input", 0, 12)
    NT_PARAMETER_CV_INPUT("E Reset Output", 0, 18)
    NT_PARAMETER_CV_INPUT("E St.Seq. Output", 0, 0)
    {"Seq E ST Seq", 1, 32, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    NT_PARAMETER_CV_INPUT("E Transpose Input", 0, 0)
    NT_PARAMETER_CV_INPUT("E Assignable CV Input", 0, 0)

    NT_PARAMETER_CV_INPUT("F CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("F Gate Input", 0, 0)
    NT_PARAMETER_CV_INPUT("F Reset Output", 0, 0)
    NT_PARAMETER_CV_INPUT("FA St.Seq. Output", 0, 0)
    {"Seq F ST Seq", 1, 32, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    NT_PARAMETER_CV_INPUT("F Transpose Input", 0, 0)
    NT_PARAMETER_CV_INPUT("F Assignable CV Input", 0, 0)

    NT_PARAMETER_CV_INPUT("G CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("G Gate Input", 0, 0)
    NT_PARAMETER_CV_INPUT("G Reset Output", 0, 0)
    NT_PARAMETER_CV_INPUT("G St.Seq. Output", 0, 0)
    {"Seq G ST Seq", 1, 32, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    NT_PARAMETER_CV_INPUT("G Transpose Input", 0, 0)
    NT_PARAMETER_CV_INPUT("G Assignable CV Input", 0, 0)

    NT_PARAMETER_CV_INPUT("H CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("H Gate Input", 0, 0)
    NT_PARAMETER_CV_INPUT("H Reset Output", 0, 0)
    NT_PARAMETER_CV_INPUT("A St.Seq. Output", 0, 0)
    {"Seq H ST Seq", 1, 32, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    NT_PARAMETER_CV_INPUT("H Transpose Input", 0, 0)
    NT_PARAMETER_CV_INPUT("H Assignable CV Input", 0, 0)

    {"Seq A Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq A Bars", 1, 16, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq B Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq B Bars", 1, 16, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq C Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq C Bars", 1, 16, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq D Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq D Bars", 1, 16, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq E Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq E Bars", 1, 16, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq F Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq F Bars", 1, 16, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq G Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq G Bars", 1, 16, 1, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq H Beats/Bar", 1, 16, 4, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Seq H Bars", 1, 16, 1, kNT_unitNone, kNT_scalingNone, nullptr},

    {"Step1 Seq", 0, 7, 0, kNT_unitEnum, kNT_scalingNone, enumStringsSequencers },
    {"Step1 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step1 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},

    {"Step2 Seq", 0, 7, 0, kNT_unitEnum, kNT_scalingNone, enumStringsSequencers },
    {"Step2 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step2 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},

    {"Step3 Seq", 0, 7, 0, kNT_unitEnum, kNT_scalingNone, enumStringsSequencers },
    {"Step3 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step3 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},

    {"Step4 Seq", 0, 7, 0, kNT_unitEnum, kNT_scalingNone, enumStringsSequencers },
    {"Step4 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step4 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},

    {"Step5 Seq", 0, 7, 0, kNT_unitEnum, kNT_scalingNone, enumStringsSequencers },
    {"Step5 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step5 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},

    {"Step6 Seq", 0, 7, 0, kNT_unitEnum, kNT_scalingNone, enumStringsSequencers },
    {"Step6 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step6 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},

    {"Step7 Seq", 0, 7, 0, kNT_unitEnum, kNT_scalingNone, enumStringsSequencers },
    {"Step7 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step7 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch},

    {"Step8 Seq", 0, 7, 0, kNT_unitEnum, kNT_scalingNone, enumStringsSequencers },
    {"Step8 Repeats", 0, 16, 0, kNT_unitNone, kNT_scalingNone, nullptr},
    {"Step8 Switch", 0, 1, 1, kNT_unitEnum, kNT_scalingNone, enumStringsSwitch}
};

// Parameter pages
static const uint8_t routingPageParams[] = {
    kParamResetInput,
    kParamBeatInput,
    kParamPitchCVOutput,
    kParamGateOutput,
    kParamAssignableOutput,
};
static const uint8_t sequencerAssignPageParams[] = {
    kParamSeq1CVInput,
    kParamSeq1GateInput,
    kParamSeq1ResetOutput,
    kParamSeq1SeqSelectOutput,
    kParamSeq1SeqSelectValue,
    kParamSeq1TransposeInput,
    kParamSeq1AssignableCVInput,

    kParamSeq2CVInput,
    kParamSeq2GateInput,
    kParamSeq2ResetOutput,
    kParamSeq2SeqSelectOutput,
    kParamSeq2SeqSelectValue,
    kParamSeq2TransposeInput,
    kParamSeq2AssignableCVInput,

    kParamSeq3CVInput,
    kParamSeq3GateInput,
    kParamSeq3ResetOutput,
    kParamSeq3SeqSelectOutput,
    kParamSeq3SeqSelectValue,
    kParamSeq3TransposeInput,
    kParamSeq3AssignableCVInput,

    kParamSeq4CVInput,
    kParamSeq4GateInput,
    kParamSeq4ResetOutput,
    kParamSeq4SeqSelectOutput,
    kParamSeq4SeqSelectValue,
    kParamSeq4TransposeInput,
    kParamSeq4AssignableCVInput,

    kParamSeq5CVInput,
    kParamSeq5GateInput,
    kParamSeq5ResetOutput,
    kParamSeq5SeqSelectOutput,
    kParamSeq5SeqSelectValue,
    kParamSeq5TransposeInput,
    kParamSeq5AssignableCVInput,

    kParamSeq6CVInput,
    kParamSeq6GateInput,
    kParamSeq6ResetOutput,
    kParamSeq6SeqSelectOutput,
    kParamSeq6SeqSelectValue,
    kParamSeq6TransposeInput,
    kParamSeq6AssignableCVInput,

    kParamSeq7CVInput,
    kParamSeq7GateInput,
    kParamSeq7ResetOutput,
    kParamSeq7SeqSelectOutput,
    kParamSeq7SeqSelectValue,
    kParamSeq7TransposeInput,
    kParamSeq7AssignableCVInput,

    kParamSeq8CVInput,
    kParamSeq8GateInput,
    kParamSeq8ResetOutput,
    kParamSeq8SeqSelectOutput,
    kParamSeq8SeqSelectValue,
    kParamSeq8TransposeInput,
    kParamSeq8AssignableCVInput

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
    kParamSeq5Bars,
    kParamSeq6BeatsPerBar,
    kParamSeq6Bars,
    kParamSeq7BeatsPerBar,
    kParamSeq7Bars,
    kParamSeq8BeatsPerBar,
    kParamSeq8Bars
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
    alg->editMode = false;
    alg->highSeqModule.reset();
    alg->highSeqModule.assertInitialized();

    alg->triggerActive = false;
    alg->triggerFrameCounter = 0;
    alg->triggerHandled = false;

    alg->SAMPLE_RATE = NT_globals.sampleRate;
    alg->FRAME_TIME_MS = (1.f/alg->SAMPLE_RATE) * 1000.f;
    alg->TRIGGER_FRAME_TARGET_MS = 25.0f;
    alg->TRIGGER_FRAMES_NEEDED = alg->TRIGGER_FRAME_TARGET_MS / alg->FRAME_TIME_MS;

    alg->selectorVoltsOut = 0.f;

    alg->resetdebug = false;
    alg->resetdebugever = false;

    return alg;
}

void distributeBeatVoltage (float beatVoltage, SongSequencer* alg) {
    // share beat input across all sequencers (different than VCV rack where each seq. has own beat input)
    for (int sequencer = 0; sequencer < alg->highSeqModule.NUM_SEQUENCERS; sequencer++) {
        if (beatVoltage < 3.0f) {
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

    int resetBusIN = self->v[kParamResetInput] - 1;
    int beatBusIN = self->v[kParamBeatInput] - 1;
    int pitchBusOUT = self->v[kParamPitchCVOutput] - 1;
    int gateBusOUT = self->v[kParamGateOutput] - 1;
    int assignableBusOUT = self->v[kParamAssignableOutput] - 1;

    // Update sequencer input and output bus assignments
    alg->sequencerCVInput[0] = self->v[kParamSeq1CVInput] - 1;
    alg->sequencerGateInput[0] = self->v[kParamSeq1GateInput] - 1;
    alg->sequencerResetOutput[0] = self->v[kParamSeq1ResetOutput] - 1;
    alg->sequencerSelectOutput[0] = self->v[kParamSeq1SeqSelectOutput] - 1;
    alg->sequencerTransposeInput[0] = self->v[kParamSeq1TransposeInput] - 1;
    alg->sequencerCVAssignableInput[0] = self->v[kParamSeq1AssignableCVInput] - 1;

    alg->sequencerCVInput[1] = self->v[kParamSeq2CVInput] - 1;
    alg->sequencerGateInput[1] = self->v[kParamSeq2GateInput] - 1;
    alg->sequencerResetOutput[1] = self->v[kParamSeq2ResetOutput] - 1;
    alg->sequencerSelectOutput[1] = self->v[kParamSeq2SeqSelectOutput] - 1;
    alg->sequencerTransposeInput[1] = self->v[kParamSeq2TransposeInput] - 1;
    alg->sequencerCVAssignableInput[1] = self->v[kParamSeq2AssignableCVInput] - 1;

    alg->sequencerCVInput[2] = self->v[kParamSeq3CVInput] - 1;
    alg->sequencerGateInput[2] = self->v[kParamSeq3GateInput] - 1;
    alg->sequencerResetOutput[2] = self->v[kParamSeq3ResetOutput] - 1;
    alg->sequencerSelectOutput[2] = self->v[kParamSeq3SeqSelectOutput] - 1;
    alg->sequencerTransposeInput[2] = self->v[kParamSeq3TransposeInput] - 1;
    alg->sequencerCVAssignableInput[2] = self->v[kParamSeq3AssignableCVInput] - 1;

    alg->sequencerCVInput[3] = self->v[kParamSeq4CVInput] - 1;
    alg->sequencerGateInput[3] = self->v[kParamSeq4GateInput] - 1;
    alg->sequencerResetOutput[3] = self->v[kParamSeq4ResetOutput] - 1;
    alg->sequencerSelectOutput[3] = self->v[kParamSeq4SeqSelectOutput] - 1;
    alg->sequencerTransposeInput[3] = self->v[kParamSeq4TransposeInput] - 1;
    alg->sequencerCVAssignableInput[3] = self->v[kParamSeq4AssignableCVInput] - 1;

    alg->sequencerCVInput[4] = self->v[kParamSeq5CVInput] - 1;
    alg->sequencerGateInput[4] = self->v[kParamSeq5GateInput] - 1;
    alg->sequencerResetOutput[4] = self->v[kParamSeq5ResetOutput] - 1;
    alg->sequencerSelectOutput[4] = self->v[kParamSeq5SeqSelectOutput] - 1;
    alg->sequencerTransposeInput[4] = self->v[kParamSeq5TransposeInput] - 1;
    alg->sequencerCVAssignableInput[4] = self->v[kParamSeq5AssignableCVInput] - 1;

    alg->sequencerCVInput[5] = self->v[kParamSeq6CVInput] - 1;
    alg->sequencerGateInput[5] = self->v[kParamSeq6GateInput] - 1;
    alg->sequencerResetOutput[5] = self->v[kParamSeq6ResetOutput] - 1;
    alg->sequencerSelectOutput[5] = self->v[kParamSeq6SeqSelectOutput] - 1;
    alg->sequencerTransposeInput[5] = self->v[kParamSeq6TransposeInput] - 1;
    alg->sequencerCVAssignableInput[5] = self->v[kParamSeq6AssignableCVInput] - 1;

    alg->sequencerCVInput[6] = self->v[kParamSeq7CVInput] - 1;
    alg->sequencerGateInput[6] = self->v[kParamSeq7GateInput] - 1;
    alg->sequencerResetOutput[6] = self->v[kParamSeq7ResetOutput] - 1;
    alg->sequencerSelectOutput[6] = self->v[kParamSeq7SeqSelectOutput] - 1;
    alg->sequencerTransposeInput[6] = self->v[kParamSeq7TransposeInput] - 1;
    alg->sequencerCVAssignableInput[7] = self->v[kParamSeq7AssignableCVInput] - 1;

    alg->sequencerCVInput[7] = self->v[kParamSeq8CVInput] - 1;
    alg->sequencerGateInput[7] = self->v[kParamSeq8GateInput] - 1;
    alg->sequencerResetOutput[7] = self->v[kParamSeq8ResetOutput] - 1;
    alg->sequencerSelectOutput[7] = self->v[kParamSeq8SeqSelectOutput] - 1;
    alg->sequencerTransposeInput[7] = self->v[kParamSeq8TransposeInput] - 1;
    alg->sequencerCVAssignableInput[7] = self->v[kParamSeq8AssignableCVInput] - 1;

    // Get pointers to input and output memory locations
    float* resetInput = busFrames + resetBusIN * numFrames;
    float* beatInput = busFrames + beatBusIN * numFrames;
    float* pitchOutput = busFrames + pitchBusOUT * numFrames;
    float* gateOutput = busFrames + gateBusOUT * numFrames;
    float* assignableOutput = busFrames + assignableBusOUT * numFrames;

    int masterStep;

    // Process busFrames
    for (int frame = 0; frame < numFrames; frame++) {

        // distribute beat input to all 5 sequencers
        distributeBeatVoltage (beatInput[frame], alg);
        alg->lastBeatVoltage = beatInput[frame]; // Store last voltage for debugging

        // Process sequencer logic
        alg->highSeqModule.process();

        // resetInput
        if (resetInput[frame] > 3.0f) {
            alg->highSeqModule.reset();    // sends reset to all sequencers
            alg->triggerFrameCounter = 0;
            alg->triggerActive = true;
            //return;
        }

        // Safety check; all step switches might be off
        masterStep = alg->highSeqModule.getMasterStep();
        if (masterStep < 0) {
            gateOutput[frame] = 0.0f;
            pitchOutput[frame] = 0.0f;
            //assignableOutput[frame] = 0.0f;
            continue;
        }

        // Find the current sequencer based on the step
        int sequencer = alg->highSeqModule.steps[masterStep].getAssignedSeq();
        if (sequencer < 0 || sequencer >= alg->highSeqModule.NUM_SEQUENCERS) {
            gateOutput[frame] = 0.0f;
            pitchOutput[frame] = 0.0f;
            //assignableOutput[frame] = 0.0f;
            continue;
        }

        // Handle Reset
        if (alg->sequencerResetOutput[sequencer] >= 0 && alg->sequencerResetOutput[sequencer] < 28) {
            int seqReset = alg->highSeqModule.sequencers[sequencer].getResetStatus();
            float* cvOutput = busFrames + alg->sequencerResetOutput[sequencer] * numFrames;

            // Start a new trigger only if not already active and reset condition is met
            if (seqReset == SEQRESET::RESET  && !alg->triggerActive &&
                alg->highSeqModule.sequencers[sequencer].getbeatState() == BEATSTATE::FIRSTHIGH) {
                alg->triggerActive = true;
                alg->triggerFrameCounter = 0;
                alg->triggerHandled = true;
            }

            // Manage trigger duration
            if (alg->triggerActive) {
                alg->triggerFrameCounter += 1;
                if (alg->triggerFrameCounter >= alg->TRIGGER_FRAMES_NEEDED) {
                    alg->triggerFrameCounter = 0;
                    alg->triggerActive = false;
                    alg->triggerHandled = false;
                }
                cvOutput[frame] = 10.0f;
                } else {
                    cvOutput[frame] = 0.0f;
            }
        }

        // NT Step Sequencer CV Select Output
        float* selOutput;

        if (alg->sequencerSelectOutput[sequencer] >= 0 && alg->sequencerSelectOutput[sequencer] < 28) {
            // Calculate the correct parameter index for Seq X ST Seq
            int paramIndex = kParamSeq1SeqSelectValue + (sequencer * 7);
            alg->selectorVoltsOut = ( alg->v[paramIndex] - 1) * SEQ12THV;

            selOutput = busFrames + alg->sequencerSelectOutput[sequencer] * numFrames;
            selOutput[frame] = alg->selectorVoltsOut;
        }

        // pitch cv input to pitch output and transpose
        float* cvInput; // used for both cv and gate inputs
        if (alg->sequencerCVInput[sequencer] >= 0 && alg->sequencerCVInput[sequencer] < 28) {

            // get transpose input
            float transposeInputval = 0.0f;
            alg->debugVal = alg->sequencerTransposeInput[sequencer];

            if (alg->sequencerTransposeInput[sequencer] >= 0) {
                cvInput = busFrames + alg->sequencerTransposeInput[sequencer] * numFrames;
                transposeInputval = cvInput[frame];
            }

            cvInput = busFrames + alg->sequencerCVInput[sequencer] * numFrames;
            float pitch = cvInput[frame];
            pitchOutput[frame] = pitch + transposeInputval;
        } else {
            pitchOutput[frame] = 0.0f; // Fallback if bus is invalid
        }

        // assignable cv input to assignable cv output
        if (alg->sequencerCVInput[sequencer] >= 0 && alg->sequencerCVInput[sequencer] < 28) {
            // get assignable CV input
            cvInput = busFrames + alg->sequencerCVAssignableInput[sequencer] * numFrames;
            float assignableCVvalue = cvInput[frame];
            assignableOutput[frame] = assignableCVvalue;
        } else {
            assignableOutput[frame] = 0.0f; // Fallback if bus is invalid
        }

        // gate cv input to gate output
        if (alg->sequencerGateInput[sequencer] >= 0 && alg->sequencerGateInput[sequencer] < 28) {
            cvInput = busFrames + alg->sequencerGateInput[sequencer] * numFrames;
            float gate = cvInput[frame];
            gateOutput[frame] = gate;
        } else {
            gateOutput[frame] = 0.0f; // fallback if bus is invalid
        }

/*
        // reset cv input to sequencer reset output
        if (alg->sequencerResetOutput[sequencer] >= 0 && alg->sequencerResetOutput[sequencer] < 28) {
            float* cvOutput = busFrames + alg->sequencerResetOutput[sequencer] * numFrames;
            cvOutput[frame] = resetInput[frame];
        }
*/

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


// return controls to be used in the customUI and so overridden
uint32_t hasCustomUI (_NT_algorithm* self) {
    return  kNT_encoderL | kNT_encoderR | kNT_potR | kNT_potButtonR;
}


/*
 * struct _NT_uiData
 { *
 float		pots[3];		// current pot positions [0.0-1.0]
 uint16_t	controls;		// current button states, and which pots changed (_NT_controls)
 uint16_t 	lastButtons;	// previous button states
 int8_t		encoders[2];	// encoder change ±1 or 0
 uint8_t		unused[2];
 };
 // screen is 256x64 - each byte contains two pixels
 */
void customUI (_NT_algorithm* self, const _NT_uiData& data) {
    SongSequencer* alg = static_cast<SongSequencer*>(self);

//  alg->lastUiData = data; // Store UI data for debugging in draw

    // left encoder - horozontal cursor
    if (data.encoders[0] != 0)
        alg->cell.col += data.encoders[0];

    // right encoder - vertical cursor
    if (data.encoders[1] != 0)
        alg->cell.row += data.encoders[1];

    if (alg->cell.col < 1) alg->cell.col = 1;
    if (alg->cell.col > 8) alg->cell.col = 8;
    if (alg->cell.row < 1) alg->cell.row = 1;
    if (alg->cell.row > 3) alg->cell.row = 3;

    // toggle edit modes
    if (  (data.controls & kNT_potButtonR)  )
        alg->editMode = true;
    else {
        alg->editMode = false;
        return;
    }

    // In Edit
    int param;
    int value;
    int offset = (alg->cell.col-1) * 3 ;

    switch (alg->cell.row) {
        case 1:
            param = kParamStep1Seq + offset;
            value = round (8 * data.pots[2]);
            NT_setParameterFromUi( NT_algorithmIndex( self ), param + NT_parameterOffset(), value );
            break;
        case 2:
            param = kParamStep1Repeats + offset;
            value = round (16 * data.pots[2]);
            NT_setParameterFromUi( NT_algorithmIndex( self ), param + NT_parameterOffset(), value );
            break;
        case 3:
            param = kParamStep1Switch + offset;
            value = round (1 * data.pots[2]);
            NT_setParameterFromUi( NT_algorithmIndex( self ), param + NT_parameterOffset(), value );
        break;
    }
}


bool drawSongSequencer (_NT_algorithm* self) {
    const SongSequencer* alg = static_cast<const SongSequencer*>(self);
    char buffer[32];
    _cursor cursor;

    assignSequencerParameters (self);
    int color = 15;

    // LINE ONE - Basic Info
    int y = 10;
    int y_offset = 11;
    int masterStep = alg->highSeqModule.getMasterStep();
    int assignedSeq = -1;

    if (masterStep >= 0)
        assignedSeq = alg->highSeqModule.steps[masterStep].getAssignedSeq();

    // LINE ONE - overall highSeqModule State

    // LINE ONE - Bars/Beats per Bar for active sequencer
    NT_drawText (0, y, "Bars/Bpb" , color, kNT_textLeft, kNT_textNormal);
    if (masterStep >= 0) {
        if (assignedSeq >= 0 && assignedSeq < alg->highSeqModule.NUM_SEQUENCERS) {
            // Bars
            NT_intToString(buffer, alg->highSeqModule.sequencers[assignedSeq].getbars());
            NT_drawText(58, y, buffer, color, kNT_textLeft, kNT_textNormal);

            if (alg->highSeqModule.sequencers[assignedSeq].getbars() < 10) {
                NT_drawText(65, y, "/", color, kNT_textLeft, kNT_textNormal);
                // Beats per bar
                NT_intToString(buffer, alg->highSeqModule.sequencers[assignedSeq].getbeatsPerBar());
                NT_drawText(71, y, buffer, color, kNT_textLeft, kNT_textNormal);
            } else {
                NT_drawText(71, y, "/", color, kNT_textLeft, kNT_textNormal);
                // Beats per bar
                NT_intToString(buffer, alg->highSeqModule.sequencers[assignedSeq].getbeatsPerBar());
                NT_drawText(77, y, buffer, color, kNT_textLeft, kNT_textNormal);
            }

        }
        else NT_drawText(58, y, "--/--", color, kNT_textLeft, kNT_textTiny);
    }
    else
        NT_drawText(58, y, "--", color, kNT_textLeft, kNT_textTiny);


    // LINE ONE - Repeat countfor active sequencer
    NT_drawText (96, y, "Rep" , color, kNT_textLeft, kNT_textNormal);
    if (masterStep >= 0) {
        if (assignedSeq >= 0 && assignedSeq < alg->highSeqModule.NUM_SEQUENCERS) {
            NT_intToString(buffer, alg->highSeqModule.steps[masterStep].getCountRepeats());
            NT_drawText(122, y, buffer, color, kNT_textLeft, kNT_textNormal);
        }
        else NT_drawText(122, y, "--", color, kNT_textLeft, kNT_textTiny);
    }
    else
        NT_drawText(122, y, "--", color, kNT_textLeft, kNT_textTiny);

    // LINE ONE - Current Bar
    NT_drawText (141, y, "Bar", color, kNT_textLeft, kNT_textNormal);
    if (masterStep >= 0) {
        if (assignedSeq >= 0 && assignedSeq < alg->highSeqModule.NUM_SEQUENCERS) {
            // bar = floor (current beat / beats per bar + 1
            int bar = floor(alg->highSeqModule.sequencers[assignedSeq].getbeatCount() /
                            alg->highSeqModule.sequencers[assignedSeq].getbeatsPerBar()) + 1;
            NT_intToString(buffer, bar);
            NT_drawText(167, y, buffer, color, kNT_textLeft, kNT_textNormal);
        }
        else NT_drawText(167, y, "--", color, kNT_textLeft, kNT_textTiny);
    }
    else
        NT_drawText(167, y, "--", color, kNT_textLeft, kNT_textTiny);


    // LINE ONE - Beatcount for active sequencer
    NT_drawText (186, y, "Beat", color, kNT_textLeft, kNT_textNormal);
    if (masterStep >= 0) {
        if (assignedSeq >= 0 && assignedSeq < alg->highSeqModule.NUM_SEQUENCERS) {
           int beat = 1 + floor(alg->highSeqModule.sequencers[assignedSeq].getbeatCount() %
                      alg->highSeqModule.sequencers[assignedSeq].getbeatsPerBar());
           NT_intToString(buffer, beat);
           NT_drawText(218, y, buffer, color, kNT_textLeft, kNT_textNormal);
        }
        else
           NT_drawText(218, y, "--", color, kNT_textLeft, kNT_textTiny);
    }
    else
        NT_drawText(218, y, "--", color, kNT_textLeft, kNT_textTiny);

    // selectorVoltsOut

    //float testVal = alg->v[kParamSeq1SeqSelectValue + sequencer + NT_parameterOffset()];
    //float testVal = alg->v[kParamSeq1SeqSelectValue + assignedSeq];
    //NT_floatToString(buffer, testVal);
    NT_floatToString(buffer, alg->selectorVoltsOut);
    NT_drawText (230, y, buffer, color, kNT_textLeft, kNT_textNormal);


    // debugVal
    //NT_intToString(buffer, alg->debugVal);
    //NT_drawText (230, y, buffer, color, kNT_textLeft, kNT_textNormal);

    // debug reset
/*
    if (alg->resetdebug) {
        NT_drawText (230, y, "R", color, kNT_textLeft, kNT_textNormal);
    } else {
        NT_drawText (230, y, "N", color, kNT_textLeft, kNT_textNormal);
    }

    if (alg->resetdebugever) {
        NT_drawText (243, y, "Y", color, kNT_textLeft, kNT_textNormal);
    }
    else
        NT_drawText (243, y, "N", color, kNT_textLeft, kNT_textNormal);
*/

    // LINE TWO - Steps Titles Screen is 256x64, Draw steps 1..8
    int x_offset = 30;
    y += y_offset + 5;
    NT_drawShapeI(kNT_rectangle, 1, y-y_offset, 256, y, 3 );
    //NT_drawText (1, y, "STEP", 15, kNT_textLeft, kNT_textNormal);
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        NT_intToString(buffer, step+1);
        NT_drawText (x_offset * (step+1), y - 2, buffer, color, kNT_textLeft, kNT_textNormal);
        if (step == masterStep)
            NT_drawShapeI (kNT_circle, x_offset * (step+1) + 2, y-5, 6, 6);
            //NT_drawShapeI (kNT_box, x_offset * (step+1) + 2, y-5, 6, 6);
    }

    // LINE THREE - Assigned Sequencer
    y += y_offset;
    NT_drawText (1, y, "SEQ ", color, kNT_textLeft, kNT_textNormal);
    char labels[] = "ABCDEFGH";
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        int seq = alg->highSeqModule.steps[step].getAssignedSeq();
        buffer[0] = labels[seq];
        NT_drawText (x_offset * (step+1), y, buffer, color, kNT_textLeft, kNT_textNormal);
    }

    // LINE FOUR - Repeats
    y += y_offset;
    NT_drawText (1, y, "REP ", color, kNT_textLeft, kNT_textNormal);
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        int repeats = alg->highSeqModule.steps[step].getRepeats();
        NT_intToString(buffer, repeats);
        NT_drawText (x_offset * (step+1), y, buffer, color, kNT_textLeft, kNT_textNormal);
    }

/*
    // LINE FIVE - Current Repeat Count
    y += y_offset;
    NT_drawText (1, y, "REP#", color, kNT_textLeft, kNT_textNormal);
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        int countRepeats = alg->highSeqModule.steps[step].getCountRepeats();
        NT_intToString(buffer, countRepeats);
        NT_drawText (x_offset * (step+1), y, buffer, 3, kNT_textLeft, kNT_textNormal);
    }
*/

    // LINE SIX - Switch State
    y += y_offset;
    NT_drawText (1, y, "On", color, kNT_textLeft, kNT_textNormal);
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        if (alg->highSeqModule.steps[step].getOnOffSwitch()) {
            NT_drawText (x_offset * (step+1), y, "Y", color, kNT_textLeft, kNT_textNormal);
        } else {
            NT_drawText (x_offset * (step+1), y, "-", color, kNT_textLeft, kNT_textNormal);
        }
    }

    // draw a cursor
    cursor.x = alg->cell.col * x_offset + 2;
    cursor.y = y_offset + (alg->cell.row+1) * y_offset;
    if (alg->editMode)  // Draw does not seem to be called whilst Pot Button is depressed
        NT_drawShapeI (kNT_box, cursor.x, cursor.y, 5, 5);
    else
        NT_drawShapeI (kNT_circle, cursor.x, cursor.y, 6, 6);


    return true; //suppress native parameter line
}

void calculateStaticRequirementsSongSequencer(_NT_staticRequirements& req) {
    req.dram = 0; // No static DRAM needed
}

void calculateRequirementsSongSequencer(_NT_algorithmRequirements& req, const int32_t* specifications) {
    req.numParameters = ARRAY_SIZE(songSequencerParameters);
    req.sram = sizeof(SongSequencer);

    // req.dram = 28 * 128 * sizeof(float); // Support 28 buses, assume 128 frames per block
    //req.dram = 28 * 128 * sizeof(float); // Support 28 buses, assume 128 frames per block
    req.dram = 0;

    req.dtc = 0;
    req.itc = 0;
    /*
    req.numParameters = ARRAY_SIZE(songSequencerParameters);
    req.sram = sizeof(SongSequencer);
    req.dram = 8 * 16 * sizeof(float);
    req.dtc = 0;
    req.itc = 0;
    */
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
    hasCustomUI, // hasCustomUi
    customUI, // customUI
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
