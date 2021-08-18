class AudioProcessor extends AudioWorkletProcessor {
    audioArrayIndex = 0;
    constructor(options) {
        super(options);
        console.debug(`AudioWorkletProcessor sampleRate ${sampleRate}`);
        this.port.onmessage = this.processMessage.bind(this);
    }

    processMessage(event) {
        console.log(event.data);
    }

    process(inputs, outputs, parameters) {
        const data = inputs[0][0];
        if (data) {
            this.port.postMessage({
                action: "detect",
                buffer: data
            });
        }
        return true;
    }
}

registerProcessor("audio-processor", AudioProcessor);