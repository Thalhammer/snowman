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
            // AudioBuffer samples are represented as floating point numbers between -1.0 and 1.0 whilst
            // a signed int16 is between -32768 and 32767
            const int16Buffer = new Int16Array(data.length);
            for (var i = 0, len = data.length; i < len; i++) {
                if (data[i] < 0) int16Buffer[i] = 0x8000 * data[i];
                else int16Buffer[i] = 0x7FFF * data[i];
            }
            this.port.postMessage({
                action: "detect",
                buffer: int16Buffer
            });
        }
        return true;
    }
}

registerProcessor("audio-processor", AudioProcessor);