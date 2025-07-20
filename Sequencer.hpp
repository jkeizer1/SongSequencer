#pragma once
namespace CLC_Synths {
	enum SEQRESET {
		NORESET = 0,
		RESET,
	};
	enum BEATSTATE {
		LOW,   // before a beat
		FIRSTHIGH,  // set on first rising edge of the beat clock voltage
		STILLHIGH,  // high as long as beat voltage is HIGH, and this will be across > 1 controller loops
	};
	class Sequencer { 
	private:
		// inputs
		int beatsPerBar;
		int bars;

		// state
		int beatCount;
		int targetBeats; // calculated
		SEQRESET resetStatus;
		BEATSTATE beatState;  // controlled by the Module based on voltage changes on the beat input

	public:

		// methods
		Sequencer();
		Sequencer(int p_beatsPerBar, int p_bars);
		void calcTargetBeats();
		void reset();
		void setReset();
		void countBeat(void);
		SEQRESET getResetStatus() const { return resetStatus; };
		void set_beatsPerBar(int b_beatsPerBar);
		void set_bars(int p_bars);
		void set_beatState(BEATSTATE p_beatState) { beatState = p_beatState;}
		
		int getbeatsPerBar() const { return beatsPerBar; };
		int getbars() const { return bars; };
		int getbeatCount() const { return beatCount; };
		int gettargetBeats() const { return targetBeats; };
		BEATSTATE getbeatState() const { return beatState; };
		
	};
	Sequencer::Sequencer() {
		beatsPerBar = 4;
		bars = 1;
		resetStatus = SEQRESET::NORESET;
		calcTargetBeats();
		reset();
	}
	Sequencer::Sequencer(int p_beatsPerBar, int p_bars) {
		beatsPerBar = p_beatsPerBar;
		bars = p_bars;
		resetStatus = SEQRESET::NORESET;
		calcTargetBeats();
		reset();
	}
	void Sequencer::calcTargetBeats() {
		targetBeats = beatsPerBar * bars;
	}
	void Sequencer::setReset() {
		resetStatus = SEQRESET::RESET;
	}
	void Sequencer::reset() {
		beatCount = 0;
		resetStatus = SEQRESET::NORESET;
		calcTargetBeats();
	}
	void Sequencer::countBeat() {
		beatCount += 1;
		if (beatCount >= targetBeats)
			resetStatus = SEQRESET::RESET;
	}
	void Sequencer::set_beatsPerBar(int p_beatsPerBar) {
		if (beatsPerBar == p_beatsPerBar)
			return;
		beatsPerBar = p_beatsPerBar;
		calcTargetBeats();
		reset();
	}
	void Sequencer::set_bars(int p_bars) {
		if (bars == p_bars)
			return;
		bars = p_bars;
		calcTargetBeats();
		reset();
	}
} // namespace
