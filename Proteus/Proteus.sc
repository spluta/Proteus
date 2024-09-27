Proteus : UGen {
	var <>id, <>desc;
	*ar { |in0=0, in1=0, id, bypass=0, mul = 1, add = 0|
		if(in0.rate!='audio'){in0 = K2A.ar(in0)};

		^this.multiNew('audio', id, in0, in1, bypass).madd(mul, add);
	}

	init { arg ... theInputs;
		this.id = theInputs[0];
		theInputs.postln;
		// store the inputs as an array
		inputs = theInputs[1..];
	}

	*loadModel {|synth, id, path, verbose = true|
		//get the index from SynthDescLib
		var defName = synth.defName.asSymbol;
		var synthIndex = SynthDescLib.global[defName];
		
		if (synthIndex.notNil) {
			synthIndex=synthIndex.metadata()[defName][id.asSymbol]['index'];
		}{
			SynthDescLib.read(SynthDef.synthDefDir+/+defName.asString++".scsyndef");
			synthIndex = SynthDescLib.global[defName].metadata()[defName][id.asSymbol]['index'];
		};

		if (synthIndex.isNil){
			"SynthDef has no metadata.\n".error;
		};

		synthIndex.do{|index|
			synth.server.sendMsg('/u_cmd', synth.nodeID, index, 'load_model', path, verbose);
		}
	}

	checkInputs {
		/* TODO */
		^this.checkValidInputs;
	}

	synthIndex_ { arg index;
		if (this.desc.notNil) 
		{ 
			this.desc.index[this.desc.index.indexOf(synthIndex)]=index;
		};
		synthIndex = index;
	}

	optimizeGraph {
		// This is called once per UGen during SynthDef construction!
		var metadata;
		// For older SC versions, where metadata might be 'nil'
		this.synthDef.metadata ?? { this.synthDef.metadata = () };
		
		
		metadata = this.synthDef.metadata[this.synthDef.name];
		if (metadata == nil) {
			// Add proteus metadata entry if needed:
			metadata = ();
			this.synthDef.metadata[this.synthDef.name] = metadata;
			this.desc = ();
			this.desc[\index] = [this.synthIndex];
		}{
			//if the metadata already existed, that means there are multiple UGens with the same id
			
			this.desc = ();
			if (metadata[this.id.asSymbol]==nil){
				//if the id info is not there, it is an additional id
				"add to desc".postln;
				this.desc[\index] = [this.synthIndex];
			}{
				//if the symbol is there, it is probably multichannel expansion
				//so we load all the indexes into an array so we can set them all at once
				this.desc[\index] = (metadata[this.id.asSymbol][\index].add(this.synthIndex));
			};
		};

		this.id.notNil.if {
			metadata.put(this.id, this.desc);
		}{
			Error("Each Proteus instance in a Synth must have a unique ID.").throw;
		};
	}
}
