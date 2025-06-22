#pragma once
namespace CLC_Synths {

    enum SWITCHSTATE {
        OFF = 0,
        ON,
    };
    enum REPEATSTATE {
        NOTCOMPLETE = 0,
        COMPLETE,
    };

    // Master Sequencer Step (1 of 8). Each step is assigned a sequencer to run for a set number of repeats
    class MasterStep {
    private:
        // inputs
        SWITCHSTATE onOffSwitch;
        int repeats;
        int assignedSeq;

        // state
        REPEATSTATE repeatState;
        int countRepeats;
    public:
        // methods
        MasterStep();
        MasterStep(int p_assignedSeq, int p_repeats, SWITCHSTATE p_onOffSwitch);

        void set_sequencer(int p_assignedSeq);
        void set_repeats(int p_repeats);
        void set_switch(SWITCHSTATE p_onOffSwitch);
       
        REPEATSTATE getRepeatState() const { return repeatState; }
        int getAssignedSeq() const { return assignedSeq; }
        int getRepeats() const { return repeats; }
        int getCountRepeats() const { return countRepeats; }
        SWITCHSTATE getOnOffSwitch() const { return onOffSwitch; }

        void reset();
        void countRepeat();
    };
    MasterStep::MasterStep() {
        assignedSeq = 0;
        repeats = 0;
        onOffSwitch = SWITCHSTATE::ON;
        reset();
    }
    MasterStep::MasterStep(int p_assignedSeq, int p_repeats, SWITCHSTATE p_onOffSwitch) {
        assignedSeq = p_assignedSeq;
        repeats = p_repeats;
        onOffSwitch = p_onOffSwitch;
        reset();
    }

	void MasterStep::set_sequencer(int p_assignedSeq) { 
		if (assignedSeq == p_assignedSeq)
			return;
		assignedSeq = p_assignedSeq; 
		reset(); 
	}
	
	void MasterStep::set_repeats(int p_repeats) {
		if (repeats == p_repeats)
			return;
		repeats = p_repeats; 
		reset(); 
	}
	
	void MasterStep::set_switch(SWITCHSTATE p_onOffSwitch) {
		if (onOffSwitch == p_onOffSwitch)
			return;
		onOffSwitch = p_onOffSwitch;
		reset();
	}
		
    void MasterStep::reset() {
        countRepeats = 0;
        repeatState = REPEATSTATE::NOTCOMPLETE;
    }

    void MasterStep::countRepeat() {
        if (onOffSwitch == SWITCHSTATE::OFF)
            return;

        if (++countRepeats > repeats) {  // the Master sequencer will advance to next Master step
            repeatState = REPEATSTATE::COMPLETE;
        }
    }
} // namespace