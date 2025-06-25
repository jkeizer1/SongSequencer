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

    float lastBeatVoltage; // for debugging
    _cell cell;
};

// constants
static const int PARAMS_PER_SEQUENCER = 2;
static const int PARAMS_PER_MASTERSTEP = 3;

// Parameter indices
enum {
    kParamResetInput,
    kParamBeatInput,
    kParamPitchCVOutput,
    kParamGateOutput,

    kParamSeq1CVInput,
    kParamSeq1GateInput,
    kParamSeq1ResetOutput,

    kParamSeq2CVInput,
    kParamSeq2GateInput,
    kParamSeq2ResetOutput,

    kParamSeq3CVInput,
    kParamSeq3GateInput,
    kParamSeq3ResetOutput,

    kParamSeq4CVInput,
    kParamSeq4GateInput,
    kParamSeq4ResetOutput,

    kParamSeq5CVInput,
    kParamSeq5GateInput,
    kParamSeq5ResetOutput,

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
    NT_PARAMETER_AUDIO_INPUT("Reset Input", 0, 1)   /* 0 is none */
    NT_PARAMETER_AUDIO_INPUT("Beat Input", 0, 2)
    NT_PARAMETER_CV_OUTPUT("Pitch CV Output", 0, 1)
    NT_PARAMETER_CV_OUTPUT("Gate Output", 0, 2)

    NT_PARAMETER_CV_INPUT("Seq A CV Input", 0, 5)
    NT_PARAMETER_CV_INPUT("Seq A Gate Input", 0, 6)
    NT_PARAMETER_CV_INPUT("Seq A Reset Output", 0, 0)

    NT_PARAMETER_CV_INPUT("Seq B CV Input", 0, 7)
    NT_PARAMETER_CV_INPUT("Seq B Gate Input", 0, 8)
    NT_PARAMETER_CV_INPUT("Seq B Reset Output", 0, 0)

    NT_PARAMETER_CV_INPUT("Seq C CV Input", 0, 9)
    NT_PARAMETER_CV_INPUT("Seq C Gate Input", 0, 10)
    NT_PARAMETER_CV_INPUT("Seq C Reset Output", 0, 0)

    NT_PARAMETER_CV_INPUT("Seq D CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq D Gate Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq D Reset Output", 0, 0)

    NT_PARAMETER_CV_INPUT("Seq E CV Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq E Gate Input", 0, 0)
    NT_PARAMETER_CV_INPUT("Seq E Reset Output", 0, 0)

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
    kParamResetInput,
    kParamBeatInput,
    kParamPitchCVOutput,
    kParamGateOutput
};
static const uint8_t sequencerAssignPageParams[] = {
    kParamSeq1CVInput,
    kParamSeq1GateInput,
    kParamSeq1ResetOutput,

    kParamSeq2CVInput,
    kParamSeq2GateInput,
    kParamSeq2ResetOutput,

    kParamSeq3CVInput,
    kParamSeq3GateInput,
    kParamSeq3ResetOutput,

    kParamSeq4CVInput,
    kParamSeq4GateInput,
    kParamSeq4ResetOutput,

    kParamSeq5CVInput,
    kParamSeq5GateInput,
    kParamSeq1ResetOutput
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
    alg->highSeqModule.reset();
    alg->highSeqModule.assertInitialized();
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

    // Update sequencer input and output bus assignments
    alg->sequencerCVInput[0] = self->v[kParamSeq1CVInput] - 1;
    alg->sequencerGateInput[0] = self->v[kParamSeq1GateInput] - 1;
    alg->sequencerResetOutput[0] = self->v[kParamSeq1ResetOutput] - 1;

    alg->sequencerCVInput[1] = self->v[kParamSeq2CVInput] - 1;
    alg->sequencerGateInput[1] = self->v[kParamSeq2GateInput] - 1;
    alg->sequencerResetOutput[1] = self->v[kParamSeq2ResetOutput] - 1;

    alg->sequencerCVInput[2] = self->v[kParamSeq3CVInput] - 1;
    alg->sequencerGateInput[2] = self->v[kParamSeq3GateInput] - 1;
    alg->sequencerResetOutput[2] = self->v[kParamSeq3ResetOutput] - 1;

    alg->sequencerCVInput[3] = self->v[kParamSeq4CVInput] - 1;
    alg->sequencerGateInput[3] = self->v[kParamSeq4GateInput] - 1;
    alg->sequencerResetOutput[3] = self->v[kParamSeq4ResetOutput] - 1;

    alg->sequencerCVInput[4] = self->v[kParamSeq5CVInput] - 1;
    alg->sequencerGateInput[4] = self->v[kParamSeq5GateInput] - 1;
    alg->sequencerResetOutput[4] = self->v[kParamSeq5ResetOutput] - 1;

    // Get pointers to input and output memory locations
    float* resetInput = busFrames + resetBusIN * numFrames;
    float* beatInput = busFrames + beatBusIN * numFrames;
    float* pitchOutput = busFrames + pitchBusOUT * numFrames;
    float* gateOutput = busFrames + gateBusOUT * numFrames;

    int masterStep;

    // Process busFrames
    for (int frame = 0; frame < numFrames; frame++) {

        // resetInput
        if (resetInput[frame] > 3.0f)
            alg->highSeqModule.reset();    // sends reset to all sequencers

        // distribute beat input to all 5 sequencers
        distributeBeatVoltage (beatInput[frame], alg);
        alg->lastBeatVoltage = beatInput[frame]; // Store last voltage for debugging

        // Process sequencer logic
        alg->highSeqModule.process();

        // Safety check; all step switches might be off
        masterStep = alg->highSeqModule.getMasterStep();
        if (masterStep < 0) {
            gateOutput[frame] = 0.0f;
            pitchOutput[frame] = 0.0f;
            return;
        }

        // Find the current sequencer based on the step
        int sequencer = alg->highSeqModule.steps[masterStep].getAssignedSeq();
        if (sequencer < 0 || sequencer > alg->highSeqModule.NUM_SEQUENCERS) {
            gateOutput[frame] = 0.0f;
            pitchOutput[frame] = 0.0f;
            return;
        }

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

        // reset cv input to sequencer reset output
        if (alg->sequencerResetOutput[sequencer] >= 0 && alg->sequencerResetOutput[sequencer] < 28) {
            float* cvOutput = busFrames + alg->sequencerResetOutput[sequencer] * numFrames;
            cvOutput[frame] = resetInput[frame];
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


// return controls to be used in the customUI and so overridden
uint32_t hasCustomUI (_NT_algorithm* self) {
    return (
            static_cast<uint16_t>(
                _NT_controls::kNT_potC |          // vertical cursor (y)
                _NT_controls::kNT_potR |          // horozontal cursor (x)
                _NT_controls::kNT_encoderR |      // change value at cursor position
                _NT_controls::kNT_encoderL        // not used but override default behaviour
                //_NT_controls::kNT_encoderButtonR  // enter value
            )
    );
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

    alg->cell.col = floor(data.pots[1]*8) + 1; // 8 steps ... resolves to 1..9
    alg->cell.row = floor(data.pots[2]*3) + 1; // 3 variables ... resolves to 1..4

    if (alg->cell.col > 8) alg->cell.col = 8;
    if (alg->cell.row > 3) alg->cell.row = 3;
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

    // LINE ONE - Master Step
    NT_drawText (1, y, "M_Step:", color, kNT_textLeft, kNT_textTiny);
    NT_intToString(buffer, masterStep);
    NT_drawText(30, y, buffer, color, kNT_textLeft, kNT_textTiny);

    // LINE ONE - overall highSeqModule State
    NT_drawText (60, y, "State", color, kNT_textLeft, kNT_textTiny);
    NT_intToString(buffer, alg->highSeqModule.getState());
    NT_drawText(90, y, buffer, color, kNT_textLeft, kNT_textTiny);

    // LINE ONE - Assigned Sequencer for masterStep or report -1
    NT_drawText (120, y, "ASEQ", color, kNT_textLeft, kNT_textTiny);
    if (masterStep >= 0) {
        NT_intToString(buffer, assignedSeq);
        NT_drawText(150, y, buffer, color, kNT_textLeft, kNT_textTiny);
    }
    else
        NT_drawText(150, y, "none", color, kNT_textLeft, kNT_textTiny);

    // LINE ONE - Beatcount for active sequencer
    NT_drawText (180, y, "Beat", color, kNT_textLeft, kNT_textTiny);
    if (masterStep >= 0) {

        if (assignedSeq >= 0 && assignedSeq < alg->highSeqModule.NUM_SEQUENCERS) {
           NT_intToString(buffer, alg->highSeqModule.sequencers[assignedSeq].getbeatCount());
           NT_drawText(210, y, buffer, color, kNT_textLeft, kNT_textTiny);
           NT_intToString(buffer, alg->highSeqModule.sequencers[assignedSeq].getbeatState());
           NT_drawText(240, y, buffer, color, kNT_textLeft, kNT_textTiny);
        }
        else
           NT_drawText(210, y, "invalid", color, kNT_textLeft, kNT_textTiny);
    }
    else
        NT_drawText(210, y, "none", color, kNT_textLeft, kNT_textTiny);

    // After LINE ONE DEBUGGING
    /*
    y += y_offset; // Move to a new line (e.g., y = 17)
    NT_drawText(1, y, "BeatV ", color, kNT_textLeft, kNT_textTiny);
    NT_floatToString(buffer, alg->lastBeatVoltage, 2); // Assuming NT_floatToString exists, or format manually
    NT_drawText(30, y, buffer, color, kNT_textLeft, kNT_textTiny);
    // DEBUGGING
    NT_drawText(60, y, "BeatBus ", color, kNT_textLeft, kNT_textTiny);
    NT_intToString(buffer, alg->v[kParamBeatInput]);
    NT_drawText(90, y, buffer, color, kNT_textLeft, kNT_textTiny);
    */

    // LINE TWO - Steps Titles Screen is 256x64, Draw steps 1..8
    int x_offset = 30;
    y += y_offset + 5;
    NT_drawShapeI(kNT_rectangle, 1, y-y_offset, 256, y, 3 );
    //NT_drawText (1, y, "STEP", 15, kNT_textLeft, kNT_textNormal);
    for (int step = 0; step < alg->highSeqModule.NUM_STEPS; step++) {
        NT_intToString(buffer, step+1);
        NT_drawText (x_offset * (step+1), y, buffer, color, kNT_textLeft, kNT_textNormal);
    }

    // LINE THREE - Assigned Sequencer
    y += y_offset;
    NT_drawText (1, y, "SEQ ", color, kNT_textLeft, kNT_textNormal);
    char labels[] = "ABCDE";
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
    NT_drawShapeI (kNT_circle, cursor.x, cursor.y, 5, 5);

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
