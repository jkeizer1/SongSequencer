#pragma once

#include "HighSeqModule.hpp"
#include "MasterStep.hpp"
#include "Sequencer.hpp"

//#include <iostream>
using namespace std;

namespace CLC_Synths {
	enum INITSTATE {
		NOTINITIALIZED = 0,
		INITIALIZED,
	};
	enum RESET_INDICATED {
		NO = 0,
		YES,
	};
	class HighSeqModule {
	public:
		// const
		static const int NUM_STEPS = 8;       // number of steps in the master sequencer
		static const int NUM_SEQUENCERS = 8; // number of sequencers being sequenced
		static const int MAX_REPEATS = 16;    // max number of repeats allowed
	private:
		// state
		INITSTATE moduleState;

		int masterStep;

		// methods
		int findFirstSwitch() const;
		int findNextStep() const;

	public:
		// state
		MasterStep steps[NUM_STEPS];
		//SWITCHSTATE switches[NUM_STEPS];
		Sequencer sequencers[NUM_SEQUENCERS];

		// methods
		HighSeqModule();
		bool guard() const;
		int getMasterStep() const { return masterStep; }
		int getState() const { return moduleState; }
		void assertInitialized();
        void reset(); 
		void process(); // process one microcontroller loop frame
	};

	HighSeqModule::HighSeqModule() {
		// don't allow any processing until steps, switches, and sequencers areall initialized
		moduleState = INITSTATE::NOTINITIALIZED;  

		// indicates no step has started when -1
		masterStep = -1;

		for (int sw = 0; sw < NUM_STEPS; sw++) steps[sw].set_switch (SWITCHSTATE::ON);
	}

	bool HighSeqModule::guard() const {
		// guard is to be called in the module before processing beats to guard against non-initialed variables
		return (moduleState == INITSTATE::INITIALIZED);
	}

	void HighSeqModule::assertInitialized() {
		// this is to be called after all physical module settings for steps and sequencers have been initialized
		moduleState = INITSTATE::INITIALIZED;
	}

	int HighSeqModule::findFirstSwitch() const {
		if (!guard()) return -1;   // this should never happen, but just in case
		int firstSwitch = 0;
		bool found = false;
		while (!found && (firstSwitch < NUM_STEPS)) {
			if (steps[firstSwitch].getOnOffSwitch() == SWITCHSTATE::ON)
				found = true;
			else
				firstSwitch += 1;
		}
		if (found) return (firstSwitch);
		else return (-1);
	}


	int HighSeqModule::findNextStep() const {
		int step = masterStep;
		int firstSwitch = -1;

		if (masterStep == -1) {
			firstSwitch = findFirstSwitch();
			if (firstSwitch == -1)
				return (-1);
			else
				return (firstSwitch);
		}

		bool found = false;
		if (++step >= NUM_STEPS) step = 0; // circular
		while (!found && (step != masterStep)) {
			if (steps[step].getOnOffSwitch() == SWITCHSTATE::ON) {
				found = true;
				return step;
			}
			if (++step >= NUM_STEPS) step = 0;
		}
		// If no other step is ON, check if current masterStep is ON
		if (steps[masterStep].getOnOffSwitch() == SWITCHSTATE::ON)
			return masterStep;
		return -1; // No active steps found
	}

/*
	int HighSeqModule::findNextStep() const {
		int step = masterStep;
		int firstSwitch = -1;

		if (masterStep == -1) {
			firstSwitch = findFirstSwitch();
			// if no switches are on, there is no target sequencer possible
			if (firstSwitch == -1)
				return (-1);
			else
				return (firstSwitch);
		}

		bool found = false;
		if (++step >= NUM_STEPS) step = 0; // circular

		while  (!found && (step != masterStep) ) {
			
			if (steps[step].getOnOffSwitch() == SWITCHSTATE::ON) {
				found = true;
				return step;
			} else 
				if (++step >= NUM_STEPS) step = 0;
		}
		return (masterStep);  // masterStep is the ONLY step on
	}
*/
    void HighSeqModule::reset() {
        masterStep = -1; // ::process will determine the correct starting step  JULY 5 set to -1
        masterStep = findNextStep(); // Set to first active step or -1 if none
        for (int s = 0; s < NUM_SEQUENCERS; s++) 
            sequencers[s].setReset();
    }

	void HighSeqModule::process() {   // called once per micro controller main loop process
		int s;
		int nextStep = -1;
		
		if (!guard()) 
			return;
		
		if (masterStep >= 0) {  // handles case where user switches off the currently running step, find next step
				if (steps[masterStep].getOnOffSwitch() == SWITCHSTATE::OFF)
					masterStep = findNextStep();
		}
		else
			masterStep = findNextStep(); 

		if (masterStep == -1) return;

		
		
		// at end of step repeat cyle, advance the master step sequencer
		if (steps[masterStep].getRepeatState() == REPEATSTATE::COMPLETE) {
			steps[masterStep].reset();			
			//sequencers[steps[masterStep].getAssignedSeq()].reset();
			nextStep = findNextStep();
		}
	
		// Reset sequencers 
		for (s = 0; s < NUM_SEQUENCERS; s++) {
			if (sequencers[s].getResetStatus() == SEQRESET::RESET) {
				// cout << "RESETING SEQUENCER for Seq: " << s << endl;
				sequencers[s].reset();
			}
		}

		// Count beats and repeats
		for (s = 0; s < NUM_SEQUENCERS; s++) {
			if (sequencers[s].getbeatState() == BEATSTATE::FIRSTHIGH) {
				//if (masterStep == s) {
					/*
					cout << "Seq: " << s << " Master Step: " <<  masterStep << endl;
					cout << "	BEAT" << endl;
					cout << "	Target beats: " << sequencers[s].gettargetBeats() << endl;
					cout << "	Beat count b4:   " << sequencers[s].getbeatCount() << endl;
					*/
				//}
				sequencers[s].countBeat();  
				//if (masterStep == s) {
					/*
					cout << "	+beatcount: " << sequencers[s].getbeatCount() << endl;
					cout << "	Resetstate: " << sequencers[s].getResetStatus() << endl;
					*/
				//}
				if ((sequencers[s].getResetStatus() == SEQRESET::RESET) &&
					(s == steps[masterStep].getAssignedSeq())) {
					/*
					cout << " 	Repeats: " << steps[s].getRepeats() << endl;
					cout << "	Count Repeats b4:" << steps[s].getCountRepeats() << endl;
					*/
					steps[masterStep].countRepeat();  // count repeat only counts if running and the step switch is on
					/*
					cout << "        +Count Repeats:" << steps[s].getCountRepeats() << endl;
					*/
					
					}
			}
		}
		if (nextStep != -1) {
			masterStep = nextStep;
			sequencers[steps[masterStep].getAssignedSeq()].reset();
		}
	}
} // namespace
