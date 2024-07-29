Proteus : UGen {
	var <>id, <>desc;
	*ar { |in0=0, in1=0, id, bypass=0, mul = 1, add = 0|
		//[in0, in1, in2, id, oversample].postln;
		if(in0.rate!='audio'){in0 = K2A.ar(in0)};
		if(in1.rate!='audio'){in1 = K2A.ar(in1)};

		^this.multiNew('audio', id, in0, in1, bypass).madd(mul, add);
	}

	init { arg ... theInputs;
		"init".postln;
		this.id = theInputs[0];
		theInputs.postln;
		// store the inputs as an array
		inputs = theInputs[1..];
	}

	*loadModel {|synth, id, path|
		//get the index from SynthDescLib
		var defName = synth.defName.asSymbol;
		var synthIndex = SynthDescLib.global[defName].metadata()[defName][id.asSymbol]['index'];

		['/u_cmd', synth.nodeID, synthIndex, '/open', path].postln;

		synth.server.sendMsg('/u_cmd', synth.nodeID, synthIndex, 'load_model', path);
		// .sendMsg('/u_cmd', ~synth.nodeID, 5, '/open', "FabFilter Pro-R 2.vst3", 1, 0, 0)
	}

	checkInputs {
		/* TODO */
		^this.checkValidInputs;
	}

	optimizeGraph {
		// This is called exactly once during SynthDef construction!
		var metadata;
		// For older SC versions, where metadata might be 'nil'
		this.synthDef.metadata ?? { this.synthDef.metadata = () };
		// Add proteus metadata entry if needed:
		metadata = this.synthDef.metadata[\proteus];
		metadata ?? {
			metadata = ();
			this.synthDef.metadata[\proteus] = metadata;
		};
		// Make plugin description and add to metadata:
		this.desc = ();
		this.desc[\index] = this.synthIndex ;

		this.id.notNil.if {
			metadata.put(this.id, this.desc);
		}{
			Error("Each Proteus instance in a Synth must have a unique ID.").throw;
		}
	}
}
